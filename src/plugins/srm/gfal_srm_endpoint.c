/*
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/*
 * @file gfal_srm_endpoint.c
 * @brief file for internal use in the srm part, endpoint determination
 * @author Devresse Adrien
 * @version 2.0
 * @date 22/06/2011
 * */



#include <time.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <stdlib.h>
#include <errno.h>
#include <mds/gfal_mds.h>

#include "gfal_srm_endpoint.h"

static enum gfal_srm_proto gfal_proto_list_prefG[]= { PROTO_SRMv2, PROTO_SRM, PROTO_ERROR_UNKNOW };


// construct a default service endpoint format, Guessing that the service endpoint follows the default DPM/dCache convention
static int gfal_srm_guess_service_endpoint(gfal_srmv2_opt* opts, const char* surl, char* buff_endpoint, size_t s_buff, enum gfal_srm_proto* srm_type, GError** err ){
    guint msize =0;
    GError* tmp_err=NULL;
    int ret =0;

    msize = g_strlcpy(buff_endpoint, GFAL_ENDPOINT_DEFAULT_PREFIX, s_buff);
    char* p,* org_p;
    p = org_p = ((char*)surl) + strlen(GFAL_PREFIX_SRM);
    const int surl_len = strlen(surl);
    while(p < surl + surl_len && *p != '/' && *p != '\0')
        p++;
    if( org_p +1 > p || msize >= s_buff || p - org_p + msize + strlen(GFAL_DEFAULT_SERVICE_ENDPOINT_SUFFIX) > s_buff){
        gfal2_set_error(&tmp_err,gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                "Impossible to setup default service endpoint from %s : bad URI format", surl);
        ret = -1;
    }else{
         strncat(buff_endpoint, org_p, p-org_p);
         g_strlcat(buff_endpoint, GFAL_DEFAULT_SERVICE_ENDPOINT_SUFFIX, s_buff);
         *srm_type= opts->srm_proto_type;
         ret =0;
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


/*
 *  return TRUE if a full endpoint is contained in surl  else FALSE
 *
*/
static gboolean gfal_check_fullendpoint_in_surlG(gfal_srmv2_opt* opts,const char * surl, GError ** err){
    const int ret=  regexec(&(opts->rex_full),surl,0,NULL,0);
	return (ret==0)?TRUE:FALSE;
}


/*
 *  @brief create a full endpoint from a "full-surl"
 * */
static int gfal_get_fullendpointG(const char* surl, char* buff_endpoint, size_t s_buff, GError** err){
	char* p = strstr(surl,"?SFN=");
	const int len_prefix = strlen(GFAL_PREFIX_SRM);						// get the srm prefix length
	const int len_endpoint_prefix = strlen(GFAL_ENDPOINT_DEFAULT_PREFIX); // get the endpoint protocol prefix len
	g_return_val_err_if_fail(p && len_prefix && (p>(surl+len_prefix)) && len_endpoint_prefix, -1,err,"[gfal_get_fullendpoint] full surl must contain ?SFN= and a valid prefix, fatal error");	// assertion on params

	size_t need_size = p- surl-len_prefix +len_endpoint_prefix;
	if(s_buff > need_size){
		memcpy(buff_endpoint, GFAL_ENDPOINT_DEFAULT_PREFIX, len_endpoint_prefix);	// copy prefix
		*((char*)mempcpy(buff_endpoint + len_endpoint_prefix, surl+len_prefix, p- surl-len_prefix))= '\0';		// copy endpoint
		return 0;
	}
    gfal2_set_error(err, gfal2_get_plugin_srm_quark(), ENOBUFS, __func__, "buffer too small");
	return -1;
}



 /*
  * map a bdii se protocol type to a gfal protocol type
  */
static enum gfal_srm_proto gfal_convert_proto_from_bdii(const char* se_type_bdii){
	enum gfal_srm_proto resu;
	if( strcmp(se_type_bdii,"srm_v1") == 0){
		resu = PROTO_SRM;
	}else if( strcmp(se_type_bdii,"srm_v2") == 0){
		resu = PROTO_SRMv2;
	}else{
		resu = PROTO_ERROR_UNKNOW;
	}
	return resu;
}


/*
 * select the best protocol choice and the best endpoint choice  from a list of protocol and endpoints obtained by the bdii
 *
 */
static int gfal_select_best_protocol_and_endpointG(gfal_srmv2_opt* opts, char** tab_se_type, char** tab_endpoint, char* buff_endpoint, size_t s_buff, enum gfal_srm_proto* srm_type, GError** err){
	g_return_val_err_if_fail(opts && buff_endpoint && s_buff && srm_type && tab_se_type && tab_endpoint, -1, err, "[gfal_select_best_protocol_and_endpoint] Invalid value");
	char** pse =tab_se_type;
	enum gfal_srm_proto* p_pref = &(opts->srm_proto_type);
	while( *p_pref != PROTO_ERROR_UNKNOW){
		while(*pse != NULL &&  *tab_endpoint != NULL ){
			if( *p_pref == gfal_convert_proto_from_bdii(*pse) ){ // test if the response is the actual preferred response
				g_strlcpy(buff_endpoint, *tab_endpoint, s_buff);
				*srm_type = *p_pref;
				return 0;
			}
			tab_endpoint++;
			pse++;
		}
		if(p_pref == &(opts->srm_proto_type)) // switch desired proto to the list if the default choice is not in the list
			p_pref=gfal_proto_list_prefG;
		else
			p_pref++;
	}
    gfal2_set_error(err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
            "cannot obtain a valid protocol from the bdii response, fatal error");
	return -2;

}


/*
 * @brief get the hostname from a surl
 *  @return return 0 if success else -1 and err is set
 */
static int  gfal_get_hostname_from_surlG(const char * surl, char* buff_hostname, size_t s_buff, GError** err){
	 const int srm_prefix_len = strlen(GFAL_PREFIX_SRM);
	 const int surl_len = strnlen(surl,GFAL_URL_MAX_LEN);
	 g_return_val_err_if_fail(surl &&  (srm_prefix_len < surl_len)  && (surl_len < GFAL_URL_MAX_LEN),-1, err, "[gfal_get_hostname_from_surl] invalid value in params");

	 char* p;
	 if((p = strchr(surl+srm_prefix_len,'/')) ==NULL){
        gfal2_set_error(err, gfal2_get_plugin_srm_quark(), EINVAL, __func__, "url invalid");
		return -1;
	 }

	 if(s_buff > (p-surl-srm_prefix_len)){
		*((char*) mempcpy(buff_hostname, surl+srm_prefix_len, p-surl-srm_prefix_len)) = '\0';
		if(*buff_hostname =='\0'){
            gfal2_set_error(err, gfal2_get_plugin_srm_quark(), EINVAL, __func__, "url invalid");
			return -1;
		}
		return 0;
	}
	gfal2_set_error(err, gfal2_get_plugin_srm_quark(), ENOBUFS, __func__, "buffer size too small");
	return -1;
 }


/*
 * get endpoint from the bdii system only
 * 0 == success
 * < 0 error
 * > 0 : bddi disabled
 *
 * */
static int gfal_get_endpoint_and_setype_from_bdiiG(gfal_srmv2_opt* opts, const char* surl,
        char* buff_endpoint, size_t s_buff, enum gfal_srm_proto* srm_type, GError** err)
{
    g_return_val_err_if_fail(opts && buff_endpoint && srm_type && surl && s_buff, -1, err,
            "[gfal_get_endpoint_and_setype_from_bdiiG] invalid parameters");

    char** tab_endpoint = NULL;
    char** tab_se_type = NULL;
    char hostname[GFAL_URL_MAX_LEN];
    int ret = -1;
    GError* tmp_err = NULL;
    if ((ret = gfal_get_hostname_from_surlG(surl, hostname, GFAL_URL_MAX_LEN, &tmp_err)) == 0) { // get the hostname

        if ((ret = gfal_mds_get_se_types_and_endpoints(opts->handle, hostname, &tab_se_type, &tab_endpoint, &tmp_err)) == 0) { // questioning the bdii
            ret = gfal_select_best_protocol_and_endpointG(opts, tab_se_type, tab_endpoint, buff_endpoint,
                    GFAL_URL_MAX_LEN, srm_type, &tmp_err); // map the response if correct
            g_strfreev(tab_endpoint);
            g_strfreev(tab_se_type);
        }
    }
    G_RETURN_ERR(ret, tmp_err, err);
}

/*
 * extract endpoint and srm_type from a surl
 *  determine the best endpoint associated with the surl and the param of the actual handle (no bdii check or not)
 *  see the diagram in doc/diagrams/surls_get_endpoint_activity_diagram.svg for more informations
 *  @return return 0 with endpoint and types set if success else -1 and set Error
 * */
int gfal_srm_determine_endpoint(gfal_srmv2_opt* opts, const char* surl,
        char* buff_endpoint, size_t s_buff, enum gfal_srm_proto* srm_type,
        GError** err)
{
    g_return_val_err_if_fail(
            opts && buff_endpoint && srm_type && surl && s_buff, -1, err,
            "[gfal_srm_determine_endpoint] invalid value in params"); // check params

    GError* tmp_err = NULL;
    int ret = -1;
    gboolean isFullEndpoint = gfal_check_fullendpoint_in_surlG(opts, surl, &tmp_err); // check if a full endpoint exist
    if (!tmp_err) {
        if (isFullEndpoint == TRUE) { // if full endpoint contained in url, get it and set type to default type
            if (gfal_get_fullendpointG(surl, buff_endpoint, s_buff, &tmp_err)
                    == 0) {
                *srm_type = opts->srm_proto_type;
                ret = 0;
                gfal_log(GFAL_VERBOSE_DEBUG,
                        "Service endpoint resolution, resolved from FULL SURL %s -> %s",
                        surl, buff_endpoint);
            }
        }
        else {
            if (gfal_get_nobdiiG(opts->handle) == TRUE ||
                ((ret = gfal_get_endpoint_and_setype_from_bdiiG(opts, surl, buff_endpoint, s_buff, srm_type, &tmp_err)) != 0)) {
                if (tmp_err) {
                    gfal_log(GFAL_VERBOSE_VERBOSE,
                            "WARNING : Error while bdii SRM service resolution : %s, fallback on the default service path."
                                    "This can lead to wrong service path, you should use FULL SURL format or register your endpoint into the BDII",
                            tmp_err->message);
                    g_clear_error(&tmp_err);
                }
                else {
                    gfal_log(GFAL_VERBOSE_VERBOSE,
                            "WARNING : BDII usage disabled, fallback on the default service path."
                                    "This can lead to wrong service path, you should use FULL SURL format or register your endpoint into the BDII");

                }
                ret = gfal_srm_guess_service_endpoint(opts, surl, buff_endpoint, s_buff, srm_type, &tmp_err);
                if (ret == 0) {
                    gfal_log(GFAL_VERBOSE_DEBUG,
                            "Service endpoint resolution, set to default path %s -> %s",
                            surl, buff_endpoint);
                }
            }
            else {
                gfal_log(GFAL_VERBOSE_DEBUG,
                        "Service endpoint resolution, resolved from BDII %s -> %s",
                        surl, buff_endpoint);

            }
        }

    }
    G_RETURN_ERR(ret, tmp_err, err);
}
