#include <davix.hpp>
#include <copy/davixcopy.hpp>
#include <unistd.h>
#include <checksums/checksums.h>
#include <cstdio>
#include <cstring>
#include "gfal_http_plugin.h"


struct PerfCallbackData {
    std::string        source;
    std::string        destination;
    gfalt_monitor_func externalCallback;
    void*              externalData;

    PerfCallbackData(const std::string& src, const std::string& dst,
                     gfalt_monitor_func callback, void* udata):
         source(src), destination(dst),
         externalCallback(callback), externalData(udata)
    {
    }
};


static bool is_streamed_scheme(const char* url)
{
    const char *schemes[] = {"http:", "https:", "dav:", "davs:", "s3:", "s3s:", NULL};
    const char *colon = strchr(url, ':');
    if (!colon)
        return false;
    size_t scheme_len = colon - url + 1;
    for (size_t i = 0; schemes[i] != NULL; ++i) {
        if (strncmp(url, schemes[i], scheme_len) == 0)
            return true;
    }
    return false;
}


static bool is_3rd_scheme(const char* url)
{
    const char *schemes[] = {"http+3rd:", "https+3rd:", "dav+3rd:", "davs+3rd:", NULL};
    const char *colon = strchr(url, ':');
    if (!colon)
        return false;
    size_t scheme_len = colon - url + 1;
    for (size_t i = 0; schemes[i] != NULL; ++i) {
        if (strncmp(url, schemes[i], scheme_len) == 0)
            return true;
    }
    return false;
}


static bool is_http_3rdcopy_enabled(gfal2_context_t context)
{
    GError *err = NULL;
    bool enabled = gfal2_get_opt_boolean(context, "HTTP PLUGIN", "ENABLE_REMOTE_COPY", &err);
    if (err)
        g_error_free(err);
    return enabled;
}


static int gfal_http_exists(plugin_handle plugin_data,
        const char* url, GError** err)
{
    GError *nestedError = NULL;
    int access_stat = gfal_http_access(plugin_data, url, F_OK, &nestedError);

    if (access_stat == 0) {
        return 1;
    }
    else if (nestedError->code == ENOENT) {
        g_error_free(nestedError);
        return 0;
    }
    else {
        gfalt_propagate_prefixed_error(err, nestedError, __func__, "", "");
        return -1;
    }
}


static int gfal_http_copy_overwrite(plugin_handle plugin_data,
        gfalt_params_t params,  const char *dst, GError** err)
{
    GError *nestedError = NULL;

    int exists = gfal_http_exists(plugin_data, dst, &nestedError);

    if (exists > 0) {
        if (!gfalt_get_replace_existing_file(params,NULL)) {
            gfalt_set_error(err, http_plugin_domain, EEXIST, __func__,
                    GFALT_ERROR_DESTINATION, GFALT_ERROR_EXISTS, "The destination file exists and overwrite is not enabled");
            return -1;
        }

        gfal_http_unlinkG(plugin_data, dst, &nestedError);
        if (nestedError) {
            gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_OVERWRITE);
            return -1;
        }

        gfal_log(GFAL_VERBOSE_DEBUG,
                 "File %s deleted with success (overwrite set)", dst);
        plugin_trigger_event(params, http_plugin_domain, GFAL_EVENT_DESTINATION,
                             GFAL_EVENT_OVERWRITE_DESTINATION, "Deleted %s", dst);
    }
    else if (exists == 0) {
        gfal_log(GFAL_VERBOSE_DEBUG, "Destination does not exist");
    }
    else if (exists < 0) {
        gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_OVERWRITE);
        return -1;
    }

    return 0;
}


static char* gfal_http_get_parent(const char* url)
{
    char *parent = g_strdup(url);
    char *slash = strrchr(parent, '/');
    if (slash) {
        *slash = '\0';
    }
    else {
        g_free(parent);
        parent = NULL;
    }
    return parent;
}


static int gfal_http_copy_make_parent(plugin_handle plugin_data,
        gfalt_params_t params, gfal2_context_t context,
        const char* dst, GError** err)
{
    GError *nestedError = NULL;

    if (!gfalt_get_create_parent_dir(params, NULL))
        return 0;

    char *parent = gfal_http_get_parent(dst);
    if (!parent) {
        gfalt_set_error(err, http_plugin_domain, EINVAL, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_PARENT,
                       "Could not get the parent directory of %s", dst);
        return -1;
    }

    int exists = gfal_http_exists(plugin_data, parent, &nestedError);
    // Error
    if (exists < 0) {
        gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_PARENT);
        return -1;
    }
    // Does exist
    else if (exists == 1) {
        return 0;
    }
    // Does not exist
    else {
        gfal2_mkdir_rec(context, parent, 0755, &nestedError);
        if (nestedError) {
            gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_PARENT);
            return -1;
        }
        gfal_log(GFAL_VERBOSE_TRACE,
                 "[%s] Created parent directory %s", __func__, parent);
        return 0;
    }
}

// dst may be NULL. In that case, the user-defined checksum
// is compared with the source checksum.
// If dst != NULL, then user-defined is ignored
static int gfal_http_copy_checksum(gfal2_context_t context,
        plugin_handle plugin_data,
        gfalt_params_t params,
        const char *src, const char *dst,
        GError** err)
{
    if (!gfalt_get_checksum_check(params, NULL))
        return 0;

    char checksum_type[1024];
    char checksum_value[1024];
    gfalt_get_user_defined_checksum(params,
                                    checksum_type, sizeof(checksum_type),
                                    checksum_value, sizeof(checksum_value),
                                    NULL);
    if (!checksum_type[0])
        g_strlcpy(checksum_type, "MD5", sizeof(checksum_type));

    GError *nestedError = NULL;
    char src_checksum[1024];
    // src may not be http!
    gfal2_checksum(context, src, checksum_type,
                   0, 0, src_checksum, sizeof(src_checksum),
                   &nestedError);

    if (nestedError) {
        gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_SOURCE, GFALT_ERROR_CHECKSUM);
        return -1;
    }

    if (!dst) {
        if (checksum_value[0] && gfal_compare_checksums(src_checksum, checksum_value, sizeof(checksum_value)) != 0) {
            gfalt_set_error(err, http_plugin_domain, EIO, __func__, GFALT_ERROR_SOURCE, GFALT_ERROR_CHECKSUM,
                            "Source and user-defined %s do not match (%s != %s)",
                            checksum_type, src_checksum, checksum_value);
            return -1;
        }
        else if (checksum_value[0]) {
            gfal_log(GFAL_VERBOSE_TRACE,
                     "[%s] Source and user-defined %s match: %s",
                     __func__, checksum_type, checksum_value);
        }
    }
    else {
        char dst_checksum[1024];
        gfal_http_checksum(plugin_data, dst, checksum_type,
                           dst_checksum, sizeof(dst_checksum),
                           0, 0, &nestedError);
        if (nestedError) {
            gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_CHECKSUM);
            return -1;
        }

        if (gfal_compare_checksums(src_checksum, dst_checksum, sizeof(dst_checksum)) != 0) {
            gfalt_set_error(err, http_plugin_domain, EIO, __func__, GFALT_ERROR_TRANSFER, GFALT_ERROR_CHECKSUM,
                            "Source and destination %s do not match (%s != %s)",
                            checksum_type, src_checksum, dst_checksum);
            return -1;
        }

        gfal_log(GFAL_VERBOSE_TRACE,
                 "[%s] Source and destination %s match: %s",
                 __func__, checksum_type, src_checksum);
    }
    return 0;
}


static void gfal_http_3rdcopy_perfcallback(const Davix::PerformanceData& perfData, void* data)
{
    PerfCallbackData* pdata = static_cast<PerfCallbackData*>(data);
    if (pdata && pdata->externalCallback)
    {
        gfalt_hook_transfer_plugin_t hook;

        hook.average_baudrate = static_cast<size_t>(perfData.avgTransfer());
        hook.bytes_transfered = static_cast<size_t>(perfData.totalTransferred());
        hook.instant_baudrate = static_cast<size_t>(perfData.diffTransfer());
        hook.transfer_time    = perfData.absElapsed();

        gfalt_transfer_status_t state = gfalt_transfer_status_create(&hook);
        pdata->externalCallback(state,
                pdata->source.c_str(), pdata->destination.c_str(),
                pdata->externalData);
        gfalt_transfer_status_delete(state);
    }
}

/// Clean dst, update err if failed during cleanup with something else than ENOENT,
/// returns always -1 for convenience
static int gfal_http_copy_cleanup(plugin_handle plugin_data, const char* dst, GError** err)
{
    GError *unlink_err = NULL;
    if ((*err)->code != EEXIST) {
        if (gfal_http_unlinkG(plugin_data, dst, &unlink_err) != 0) {
            if (unlink_err->code != ENOENT) {
                gfal_log(GFAL_VERBOSE_VERBOSE,
                         "When trying to clean the destination: %s", unlink_err->message);
            }
            g_error_free(unlink_err);
        }
        else {
            gfal_log(GFAL_VERBOSE_DEBUG, "Destination file removed");
        }
    }
    else {
        gfal_log(GFAL_VERBOSE_DEBUG, "The transfer failed because the file exists. Do not clean!");
    }
    return -1;
}

// Only http or https
// See LCGUTIL-473
static std::string get_canonical_uri(const std::string& original)
{
    std::string scheme;
    char last_scheme;

    size_t plus_pos = original.find('+');
    size_t colon_pos = original.find(':');

    if (plus_pos < colon_pos)
        last_scheme = original[plus_pos - 1];
    else
        last_scheme = original[colon_pos - 1];

    if (last_scheme == 's')
        scheme = "https";
    else
        scheme = "http";

    std::string canonical = (scheme + original.substr(colon_pos));
    return canonical;
}


static void gfal_http_third_party_copy(GfalHttpPluginData* davix,
        const char* src, const char* dst,
        gfalt_params_t params,
        GError** err)
{
    gfal_log(GFAL_VERBOSE_VERBOSE, "Performing a HTTP third party copy");

    PerfCallbackData perfCallbackData(
            src, dst,
            gfalt_get_monitor_callback(params, NULL),
            gfalt_get_user_data(params, NULL)
    );

    std::string canonical_dst = get_canonical_uri(dst);
    gfal_log(GFAL_VERBOSE_VERBOSE, "Normalize destination to %s", canonical_dst.c_str());

    Davix::Uri src_uri(src);
    Davix::Uri dst_uri(canonical_dst);

    Davix::RequestParams req_params;
    davix->get_params(&req_params, src_uri);
    Davix::DavixCopy copy(davix->context, &req_params);

    copy.setPerformanceCallback(gfal_http_3rdcopy_perfcallback, &perfCallbackData);

    Davix::DavixError* davError = NULL;
    copy.copy(src_uri, dst_uri,
              gfalt_get_nbstreams(params, NULL),
              &davError);

    if (davError != NULL) {
        davix2gliberr(davError, err);
        Davix::DavixError::clearError(&davError);
    }
}


struct HttpStreamProvider {
    const char *source, *destination;

    gfal2_context_t context;
    int source_fd;
    time_t start, last_update;
    dav_ssize_t read_instant;
    gfalt_hook_transfer_plugin_t perf;

    gfalt_monitor_func perf_callback;
    void* perf_callback_data;

    HttpStreamProvider(const char* source, const char* destination,
            gfal2_context_t context, int source_fd, gfalt_params_t params):
        source(source), destination(destination),
        context(context), source_fd(source_fd), start(time(NULL)),
        last_update(start), read_instant(0)
    {
        memset(&perf, 0, sizeof(perf));

        perf_callback = gfalt_get_monitor_callback(params, NULL);
        perf_callback_data = gfalt_get_user_data(params, NULL);
    }
};


static dav_ssize_t gfal_http_streamed_provider(void *userdata,
        char *buffer, dav_size_t buflen)
{
    GError* error = NULL;
    HttpStreamProvider* data = static_cast<HttpStreamProvider*>(userdata);
    dav_ssize_t ret = 0;

    time_t now = time(NULL);

    if (buflen == 0) {
        data->perf.bytes_transfered = data->read_instant = 0;
        data->perf.average_baudrate = 0;
        data->perf.instant_baudrate = 0;
        data->start = data->last_update = now;

        if (gfal2_lseek(data->context, data->source_fd, 0, SEEK_SET, &error) < 0)
            ret = -1;
    }
    else {
        ret = gfal2_read(data->context, data->source_fd, buffer, buflen, &error);
        if (ret > 0)
            data->read_instant += ret;

        if (now - data->last_update >= 5) {
            data->perf.bytes_transfered += data->read_instant;
            data->perf.transfer_time = now - data->start;
            data->perf.average_baudrate = data->perf.bytes_transfered / data->perf.transfer_time;
            data->perf.instant_baudrate = data->read_instant / now - data->last_update;

            data->last_update = now;
            data->read_instant = 0;

            if (data->perf_callback) {
                gfalt_transfer_status_t state = gfalt_transfer_status_create(&data->perf);
                data->perf_callback(state, data->source, data->destination, data->perf_callback_data);
                gfalt_transfer_status_delete(state);
            }
        }
    }

    if (error)
        g_error_free(error);

    return ret;
}


static void gfal_http_streamed_copy(gfal2_context_t context,
        GfalHttpPluginData* davix,
        const char* src, const char* dst,
        gfalt_params_t params,
        GError** err)
{
    gfal_log(GFAL_VERBOSE_VERBOSE, "Performing a HTTP streamed copy");
    GError *nested_err = NULL;

    struct stat src_stat;
    if (gfal2_stat(context, src, &src_stat, &nested_err) != 0) {
        gfal2_propagate_prefixed_error(err, nested_err, __func__);
        return;
    }

    int source_fd = gfal2_open(context, src, O_RDONLY, &nested_err);
    if (source_fd < 0) {
        gfal2_propagate_prefixed_error(err, nested_err, __func__);
        return;
    }

    Davix::Uri dst_uri(dst);

    Davix::DavixError* dav_error = NULL;
    Davix::PutRequest request(davix->context, dst_uri, &dav_error);
    if (dav_error != NULL) {
        davix2gliberr(dav_error, err);
        Davix::DavixError::clearError(&dav_error);
        return;
    }

    Davix::RequestParams req_params;
    davix->get_params(&req_params, dst_uri);
    if (dst_uri.getProtocol() == "s3" || dst_uri.getProtocol() == "s3s")
        req_params.setProtocol(Davix::RequestProtocol::AwsS3);
    request.setParameters(req_params);

    HttpStreamProvider provider(src, dst, context, source_fd, params);

    request.setRequestBody(gfal_http_streamed_provider, src_stat.st_size, &provider);
    request.executeRequest(&dav_error);

    gfal2_close(context, source_fd, &nested_err);
    // Throw away this error
    if (nested_err)
        g_error_free(nested_err);

    if (dav_error != NULL) {
        davix2gliberr(dav_error, err);
        Davix::DavixError::clearError(&dav_error);
        return;
    }

    // Double check the HTTP code
    if (request.getRequestCode() >= 400) {
        http2gliberr(err, request.getRequestCode(), __func__, "Failed to PUT the file");
    }

    gfal_log(GFAL_VERBOSE_VERBOSE, "HTTP code %d", request.getRequestCode());
}


void strip_3rd_from_url(const char* url_full, char* url, size_t url_size)
{
    const char* colon = strchr(url_full, ':');
    const char* plus = strchr(url_full, '+');
    if (!plus || !colon || plus > colon) {
        g_strlcpy(url, url_full, url_size);
    }
    else {
        size_t len = plus - url_full + 1;
        if (len >= url_size)
            len = url_size;
        g_strlcpy(url, url_full, len);
        g_strlcat(url, colon, url_size);
    }
}


int gfal_http_copy(plugin_handle plugin_data, gfal2_context_t context,
        gfalt_params_t params, const char* src_full, const char* dst_full, GError** err)
{
    GError* nested_error = NULL;
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_ENTER,
                         "%s => %s", src_full, dst_full);

    // Determine if urls are 3rd party, and strip the +3rd if they are
    char src[GFAL_URL_MAX_LEN], dst[GFAL_URL_MAX_LEN];
    bool src_is_3rd = is_3rd_scheme(src_full);
    bool dst_is_3rd = is_3rd_scheme(dst_full);;
    strip_3rd_from_url(src_full, src, sizeof(src));
    strip_3rd_from_url(dst_full, dst, sizeof(dst));

    gfal_log(GFAL_VERBOSE_VERBOSE, "Using source: %s", src);
    gfal_log(GFAL_VERBOSE_VERBOSE, "Using destination: %s", dst);

    // When this flag is not set, the plugin should handle overwriting,
    // parent directory creation,...
    if (!gfalt_get_strict_copy_mode(params, NULL)) {
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER,
                             "");
        if (gfal_http_copy_checksum(context, plugin_data, params, src, NULL, &nested_error) != 0) {
            gfal2_propagate_prefixed_error(err, nested_error, __func__);
            return -1;
        }
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT,
                             "");

        if (gfal_http_copy_overwrite(plugin_data, params, dst, &nested_error) != 0 ||
            gfal_http_copy_make_parent(plugin_data, params, context, dst, &nested_error) != 0) {
            gfal2_propagate_prefixed_error(err, nested_error, __func__);
            return -1;
        }
    }

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_EXIT,
                         "%s => %s", src_full, dst_full);

    // The real, actual, copy
    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_ENTER,
                         "%s => %s", src_full, dst_full);

    if (!src_is_3rd && !dst_is_3rd) {
        gfal_http_streamed_copy(context, davix, src, dst, params, &nested_error);
    }
    else if (src_is_3rd && dst_is_3rd) {
        if (is_http_3rdcopy_enabled(context)) {
            gfal_http_third_party_copy(davix, src, dst, params, &nested_error);
        }
        else {
            gfal2_set_error(err, http_plugin_domain, ENOENT, __func__,
                    "3rd party copy requested, but disabled in the configuration");
        }
    }
    else {
        gfal2_set_error(err, http_plugin_domain, ENOENT, __func__,
                    "Invalid combination of 3rd party and non 3rd party urls");
        return -1;
    }

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_EXIT,
                         "%s => %s", src, dst);

    if (nested_error != NULL) {
        gfalt_propagate_prefixed_error(err, nested_error, __func__, GFALT_ERROR_TRANSFER, "");
        return gfal_http_copy_cleanup(plugin_data, dst, err);
    }

    // Checksum check
    if (!gfalt_get_strict_copy_mode(params, NULL)) {
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER,
                             "");
        if (gfal_http_copy_checksum(context, plugin_data, params, src, dst, err) != 0)
            return gfal_http_copy_cleanup(plugin_data, dst, err);
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_EXIT,
                             "");
    }

    return 0;
}


int gfal_http_copy_check(plugin_handle plugin_data, gfal2_context_t context, const char* src,
        const char* dst, gfal_url2_check check)
{
    if (check != GFAL_FILE_COPY)
        return 0;
    // This plugin handles everything that writes into an http endpoint
    return (is_streamed_scheme(dst) && !is_3rd_scheme(src)) || (is_3rd_scheme(src) && is_3rd_scheme(dst));
}

