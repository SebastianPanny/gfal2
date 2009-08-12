/*                                                                -*- mode: c; c-basic-offset: 8;  -*-
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: mds_ifce.c,v $ $Revision: 1.84 $ $Date: 2009/08/12 13:55:48 $ CERN Jean-Philippe Baud
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <lber.h>
#include <ldap.h>
#include "gfal_api.h"
#include "gfal_internals.h"

#define GFAL_BDII_SERVER_FIRST -1

static char *default_search_base = "o=grid";
static const char gfal_remote_type[] = "BDII";

struct bdii_server_info_t {
    char *server;
    int port;
    char *search_base;
    int status;
};

static struct bdii_server_info_t *bdii_servers = NULL;
static int bdii_servers_count = 0;

/*
   Find the number of occurences of char c in string str.
 */
    static int
strchrscan (const char* str, int c)
{
    int n = 0;
    while (*str != 0)
        if (* (str++) == c) ++n;

    return n;
}

/* Convert LDAP error into errno */
static int
ldaperr2errno (int err) {
    int res;

    switch (err) {
        case LDAP_SERVER_DOWN:
            res = EHOSTDOWN; break;
        default:
            res = ECONNREFUSED;
    }

    return res;
}

/*
   Parse the LCG_GFAL_INFOSYS environment variable and fill out the
   bdii_servers_count and bdii_servers vars.
   Usually this function is called only once per program startup.
 */
    static int
bdii_parse_env (char *errbuf, int errbufsz)
{
    char separator[] = ";,";
    char *bdii_env;
    int max_servers_count;
    struct bdii_server_info_t *list;
    int n;
    char *ptr, *colon_pos, *slash_pos;
    char *strtok_state;

    bdii_env = getenv ("LCG_GFAL_BDII_TIMEOUT");
    if (bdii_env != NULL)
        gfal_set_timeout_bdii (atoi (bdii_env));

    bdii_env = getenv ("LCG_GFAL_BDII_DEFAULT_SEARCH_BASE");
    if (bdii_env != NULL)
        default_search_base = strdup (bdii_env);

    bdii_env = getenv ("LCG_GFAL_INFOSYS");
    if (bdii_env == NULL) {
        bdii_servers_count = -1;
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][bdii_parse_env][EINVAL] LCG_GFAL_INFOSYS not set");
        errno = EINVAL;
        return (-1);
    }

    if (strchr (bdii_env, '/'))
        /* some BDII servers are specified with their search base (which can includes ',')
         * in that case, ',' can't be used as a separator
         * note that in other case, ',' can be used as a separator for backward compatibility! */
        separator[1] = 0;

    max_servers_count = strchrscan (bdii_env, separator[1] ? separator[1] : separator[0]) + 1;
    list = (struct bdii_server_info_t*) calloc (max_servers_count, sizeof (struct bdii_server_info_t));
    if (list == NULL) {
        bdii_servers_count = -1;
        return (-1);
    }

    ptr = strtok_r (bdii_env, separator, &strtok_state);
    n = 0;

    while (ptr != NULL) {
        if (!isalnum (*ptr)) { /* ignore this invalid token */
            gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[WARNING] Invalid BDII hostname: %s", ptr);
            ptr = strtok_r (NULL, separator, &strtok_state);
            continue;
        }

        slash_pos = strchr (ptr, '/');
        if (slash_pos != NULL) {
            if (strchr (ptr, ',') < slash_pos || strpbrk (slash_pos + 1, ".:/") > slash_pos) {
                /* ... then ',' has been used as a separation, instead of ';' */
                gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[WARNING] Wrong separator in BDII list (must be ';'): %s", ptr);
                ptr = strtok_r (NULL, separator, &strtok_state);
                continue;
            }

            list[n].search_base = strdup (slash_pos + 1);
            *slash_pos = 0;
        } else { /* no search base is specified, the default one will be used */
            list[n].search_base = default_search_base;
        }

        colon_pos = strchr (ptr, ':');
        if (colon_pos != NULL) {
            list[n].port = atoi (colon_pos + 1);
            *colon_pos = 0;
        } else { /* port not specified, defaulting to 2170 */
            list[n].port = 2170;
        }

        list[n].server = strdup (ptr);
        list[n].status = 1; // servers are considered as good while not detected as bad

        gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_INFO, "[INFO] BDII server: %s:%d/%s",
                list[n].server, list[n].port, list[n].search_base);
        ptr = strtok_r (NULL, separator, &strtok_state);
        ++n;
    }

    if (n == 0) {
        free (list);
        bdii_servers_count = -1;
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][bdii_parse_env][EINVAL] LCG_GFAL_INFOSYS is invalid (wrong separator?)");
        errno = EINVAL;
        return (-1);
    }

    bdii_servers = realloc (list, n * sizeof (struct bdii_server_info_t));
    if (bdii_servers == NULL) {
        /* errno is set by realloc */
        bdii_servers_count = -1;
        return -1;
    }

    bdii_servers_count = n;
    return bdii_servers_count;
}

/*
   Try to find a next bdii server.
   Return 1 if there are still some servers to try or 0 if tried all servers.
 */
    static int
bdii_server_get_next (int *bdii_index, const char **bdii_server_ptr, int *bdii_port_ptr, const char **bdii_search_base_ptr)
{
    do {
        ++(*bdii_index);
    } while (*bdii_index < bdii_servers_count
            && !bdii_servers[*bdii_index].status);

    if (*bdii_index >= bdii_servers_count)
        return (0);

    if (bdii_server_ptr)
        *bdii_server_ptr = bdii_servers[*bdii_index].server;
    if (bdii_port_ptr)
        *bdii_port_ptr = bdii_servers[*bdii_index].port;
    if (bdii_search_base_ptr)
        *bdii_search_base_ptr = bdii_servers[*bdii_index].search_base;
    return (1);
}

/*
   Mark the current bdii server as a good one.
 */
    static void
bdii_server_is_bad (int *bdii_index)
{
    bdii_servers[*bdii_index].status = 0;
}

/*
   make a query to the BDII.
   Store the LDAP* into the ld_ptr, LDAPMessage* into reply_ptr,
   and BDII server used into bdii_server_ptr and bdii_port_ptr.
 */
    static int
bdii_query_send (LDAP **ld_ptr, char* filter, char* attrs[],
        LDAPMessage **reply_ptr, const char **bdii_server_ptr, int *bdii_port_ptr,
        int *bdii_index, char *errbuf, int errbufsz)
{
    const char *bdii_server;
    const char *bdii_search_base;
    int bdii_port;
    LDAP *ld = NULL;
    struct timeval timeout;
    int err = 0, rc = 0;
    int wasfirst = *bdii_index == GFAL_BDII_SERVER_FIRST;

    if (gfal_is_nobdii ()) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][bdii_query_send][EINVAL] BDII calls are needed, but disabled!");
        errno = EINVAL;
        return (-1);
    }

    /* Parse the environment, if required. */
    if (bdii_servers_count == 0) {
        if (bdii_parse_env (errbuf, errbufsz) < 0)
            return (-1);
    }
    if (bdii_servers_count < 0) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFA, NULLL][bdii_query_send][EINVAL] Invalid BDII parameters");
        errno = EINVAL;
        return (-1);
    }

    if (wasfirst)
        gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_INFO, "[INFO] BDII filter: %s", filter);

    if (!bdii_server_get_next (bdii_index, &bdii_server, &bdii_port, &bdii_search_base)) {
        /* it should never come here, previous request should have failed already */
        errno = EINVAL;
        return (-1);
    }

    do {
        gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_INFO, "[INFO] Trying to use BDII: %s:%d/%s (timeout %d)",
                bdii_server, bdii_port, bdii_search_base, gfal_get_timeout_bdii ());

        ld = ldap_init (bdii_server, bdii_port);
        if (ld == NULL) {
            bdii_server_is_bad (bdii_index);
            gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[%s][ldap_init][] %s:%d > %s",
                    gfal_remote_type, bdii_server, bdii_port, strerror (errno));
            continue;
        }

        if (gfal_get_timeout_connect () > 0) {
            timeout.tv_sec = gfal_get_timeout_connect ();
            timeout.tv_usec = 0;
            ldap_set_option (ld, LDAP_OPT_NETWORK_TIMEOUT, &timeout);
        }

        if ((err = ldap_simple_bind_s (ld, "", "")) != LDAP_SUCCESS) {
            ldap_unbind (ld);
            bdii_server_is_bad (bdii_index);
            gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[%s][ldap_simple_bind_s][] %s:%d > %s",
                    gfal_remote_type, bdii_server, bdii_port, ldap_err2string (err));
            continue;
        }

        timeout.tv_sec = gfal_get_timeout_bdii ();
        timeout.tv_usec = 0;
        rc = ldap_search_st (ld, bdii_search_base, LDAP_SCOPE_SUBTREE, filter, attrs, 0,
                gfal_get_timeout_bdii () > 0 ? &timeout : NULL, reply_ptr);
        if (rc != LDAP_SUCCESS) {
            ldap_unbind (ld);
            if (rc == LDAP_TIMELIMIT_EXCEEDED || rc == LDAP_TIMEOUT) {
                gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[%s][ldap_search_st][] %s:%d > Connection Timeout",
                        gfal_remote_type, bdii_server, bdii_port);
            } else {
                gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[%s][ldap_search_st][] %s:%d > %s",
                        gfal_remote_type, bdii_server, bdii_port,
                        ldap_err2string (rc));
            }
            bdii_server_is_bad (bdii_index);
            continue;
        }

        *ld_ptr = ld;
        *bdii_server_ptr = bdii_server;
        *bdii_port_ptr = bdii_port;
        return (0);
    } while (bdii_server_get_next (bdii_index, &bdii_server, &bdii_port, &bdii_search_base));

    if (wasfirst) {
        /* if !wasfirst then at least 1 bdii was accessible but doesn't contain wanted information */
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][bdii_query_send][EINVAL] No accessible BDII",
                gfal_remote_type, bdii_server, bdii_port);
        errno = EINVAL;
    }
    return (-1);
}

    static int
bdii_query_free (LDAP** ld_ptr, LDAPMessage **reply_ptr)
{
    ldap_msgfree (*reply_ptr);
    ldap_unbind (*ld_ptr);
    return 0;
}

/* get BDII hostname and port number */
/* backwards compatibility version */
get_bdii (char *bdii_server, int buflen, int *bdii_port, char *errbuf, int errbufsz)
{
    const char *bdii_server_r;
    int bdii_port_r;
    int bdii_index = GFAL_BDII_SERVER_FIRST;

    /* Parse the environment, if required. */
    if (bdii_servers_count == 0) {
        if (bdii_parse_env (errbuf, errbufsz) < 0)
            return (-1);
    }
    if (bdii_servers_count < 0) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_bdii][EINVAL] Invalid BDII parameters");
        errno = EINVAL;
        return (-1);
    }
    if (!bdii_server_get_next (&bdii_index, &bdii_server_r, &bdii_port_r, NULL)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_bdii][EINVAL] no working BDII");
        errno = EINVAL;
        return (-1);
    }
    if (strlen (bdii_server_r) >= buflen) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_bdii][EINVAL] bdii_server buffer length is too short");
        errno = EINVAL;
        return -1;
    }

    strcpy (bdii_server, bdii_server_r);
    *bdii_port = bdii_port_r;

    return (0);
}

/* generates part of bdii filter with accesscontrolbaserule */
static char *
generate_acbr (const char *glueobject, char *errbuf, int errbufsz) {
    char tmp[64 + GFAL_FQAN_MAXLEN];
    char *filter = NULL, *vo = NULL, **fqan = NULL;
    int nb_fqan, filterlen, i;

    if ((vo = gfal_get_vo (errbuf, errbufsz)) == NULL)
        return (NULL);

    nb_fqan = gfal_get_fqan (&fqan, errbuf, errbufsz);
    filterlen = nb_fqan * (sizeof (tmp)) + 3 * (64 + GFAL_VO_MAXLEN);

    if ((filter = (char *) calloc (filterlen, sizeof (char))) == NULL)
        return (NULL);

    snprintf (filter, filterlen,
            "(| (%sAccessControlBaseRule=VO:%s) (%sAccessControlBaseRule=%s) (%sAccessControlRule=%s)",
            glueobject, vo, glueobject, vo, glueobject, vo);

    for (i = 0; i < nb_fqan; ++i) {
        snprintf (tmp, sizeof (tmp), " (%sAccessControlBaseRule=VOMS:%s)", glueobject, fqan[i]);
        strcat (filter, tmp);
    }

    strcat (filter, ")");
    return (filter);
}

/* get from the BDII the CE Accesspoint for a given SE */

get_ce_ap (const char *host, char **ce_ap, char *errbuf, int errbufsz)
{
    static char ce_ap_atnm[] = "GlueCESEBindCEAccesspoint";
    static char *template = "(GlueCESEBindSEUniqueID=%s)";
    static char *attrs[] = {ce_ap_atnm, NULL};
    int bdii_port;
    const char *bdii_server;
    LDAPMessage *entry;
    char filter[128];
    LDAP *ld;
    int rc = 0;
    LDAPMessage *reply;
    char **value;
    int sav_errno = 0;
    int bdii_index = GFAL_BDII_SERVER_FIRST;

    *ce_ap = NULL;

    if (strlen (template) + strlen (host) - 2 >= sizeof (filter)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_ce_ap][ENAMETOOLONG] %s:%d: Hostname too long",
                bdii_server, bdii_port);
        errno = ENAMETOOLONG;
        return (-1);
    }
    sprintf (filter, template, host);

    while (!*ce_ap && !sav_errno) {
        rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, &bdii_index, errbuf, errbufsz);
        if (rc < 0) {
            sav_errno = errno;
            break;
        }

        if ((entry = ldap_first_entry (ld, reply)) && (value = ldap_get_values (ld, entry, ce_ap_atnm))) {
            if ((*ce_ap = strdup (value[0])) == NULL)
                sav_errno = errno ? errno : ENOMEM;
            ldap_value_free (value);
        } else {
            gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN,
                    "[%s][][] %s:%d: No GlueCESEBindCEAccesspoint found for host %s", gfal_remote_type, bdii_server, bdii_port, host);
        }

        bdii_query_free (&ld, &reply);
    }

    if (!*ce_ap) {
        if (!sav_errno) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                    "[%s][][] No GlueCESEBindCEAccesspoint found for host %s", gfal_remote_type, host);
            sav_errno = EINVAL;
        }
        errno = sav_errno;
        return (-1);
    }

    return (0);
}

/* get from the BDII the RLS endpoints */

get_rls_endpoints (char **lrc_endpoint, char **rmc_endpoint, char *errbuf, int errbufsz)
{
    static char rls_ep[] = "GlueServiceEndpoint";
    static char rls_type[] = "GlueServiceType";
    static char *template = "(&(GlueServiceType=edg-*)(|(GlueServiceAccessControlBaseRule=%s)(GlueServiceAccessControlRule=%s)))";
    char *attr;
    static char *attrs[] = {rls_type, rls_ep, NULL};
    int bdii_port;
    const char *bdii_server = NULL;
    BerElement *ber = NULL;
    LDAPMessage *entry = NULL;
    char filter[100 + 2 * GFAL_VO_MAXLEN];
    LDAP *ld = NULL;
    int rc = 0;
    LDAPMessage *reply = NULL;
    char *service_type = NULL, *service_url = NULL, *vo = NULL, **value = NULL;
    int bdii_index = GFAL_BDII_SERVER_FIRST;

    if ((vo = gfal_get_vo (errbuf, errbufsz)) == NULL) {
        errno = EINVAL;
        return (-1);
    }
    if (strlen (vo) > GFAL_VO_MAXLEN) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_rls_endpoints][ENAMETOOLONG] VO name too long");
        errno = ENAMETOOLONG;
        return (-1);
    }
    sprintf (filter, template, vo, vo);

    rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, &bdii_index, errbuf, errbufsz);
    if (rc < 0) return rc;
    GFAL_DEBUG ("DEBUG: get_rls_endpoints used server %s:%d\n", bdii_server, bdii_port);

    for (entry = ldap_first_entry (ld, reply);
            entry != NULL;
            entry = ldap_next_entry (ld, entry)) {
        service_type = NULL;
        service_url = NULL;
        for (attr = ldap_first_attribute (ld, entry, &ber);
                attr != NULL;
                attr = ldap_next_attribute (ld, entry, ber)) {
            value = ldap_get_values (ld, entry, attr);
            if (value != NULL) {
                if (strcmp (attr, "GlueServiceType") == 0) {
                    if ((service_type = strdup (value[0])) == NULL)
                        rc = -1;
                } else {	/* GlueServiceEndpoint */
                    if ((service_url = strdup (value[0])) == NULL)
                        rc = -1;
                }
                ldap_value_free (value);
            }
            else
                rc = -1;
        }
        if (rc == 0) {
            if (strcmp (service_type, "edg-replica-metadata-catalog") == 0) {
                if ( (*rmc_endpoint = strdup (service_url)) == NULL)
                    rc = -1;
            } else if (strcmp (service_type, "edg-local-replica-catalog") == 0) {
                if ( (*lrc_endpoint = strdup (service_url)) == NULL)
                    rc = -1;
            }
        }
        free (service_type);
        free (service_url);
    }

    ber_free (ber, 0);
    bdii_query_free (&ld, &reply);

    if (*lrc_endpoint == NULL) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][][] %s:%d: LRC endpoint not found",
                gfal_remote_type, bdii_server, bdii_port);
        errno = EINVAL;
        rc = -1;
    }
    if (*rmc_endpoint == NULL) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][][] %s:%d: RMC endpoint not found",
                gfal_remote_type, bdii_server, bdii_port);
        errno = EINVAL;
        rc = -1;
    }
    return (rc);
}

/* get from the BDII the endpoint for the LFC */

get_lfc_endpoint (char **lfc_endpoint, char *errbuf, int errbufsz)
{
    static char ep[] = "GlueServiceEndpoint";
    static char *template = "(&(GlueServiceType=lcg-file-catalog)%s)";
    static char *attrs[] = {ep, NULL};
    LDAPMessage *entry;
    char *filter, *filter_tmp;
    LDAP *ld;
    int rc = 0;
    LDAPMessage *reply;
    char **value;
    const char *bdii_server;
    int bdii_port;
    int sav_errno = 0;
    int bdii_index = GFAL_BDII_SERVER_FIRST;

    *lfc_endpoint = NULL;

    if ((filter_tmp = generate_acbr ("GlueService", errbuf, errbufsz)) == NULL)
        return (-1);

    rc = asprintf (&filter, template, filter_tmp);
    free (filter_tmp);
    if (rc < 0) return (-1);

    while (!*lfc_endpoint && !sav_errno) {
        rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, &bdii_index, errbuf, errbufsz);
        if (rc < 0) {
            sav_errno = errno;
            break;
        }

        if ((entry = ldap_first_entry (ld, reply)) && (value = ldap_get_values (ld, entry, ep))) {
            if ((*lfc_endpoint = strdup (*value)) == NULL)
                sav_errno = errno ? errno : ENOMEM;
            ldap_value_free (value);
        } else {
            gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN,
                    "[%s][][] %s:%d: No LFC Endpoint found", gfal_remote_type, bdii_server, bdii_port);
        }

        bdii_query_free (&ld, &reply);
    }

    free (filter);

    if (!*lfc_endpoint) {
        if (!sav_errno) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                    "[%s][][] No LFC Endpoint found", gfal_remote_type);
            sav_errno = EINVAL;
        }
        errno = sav_errno;
        return (-1);
    }

    return (0);
}

/* Get from the BDII the SAPath */

    static int
get_sa_path (const char *host, const char *salocalid, char **sa_path, char **sa_root, char *errbuf, int errbufsz)
{
    static char sa_path_atnm[] = "GlueSAPath";
    static char sa_root_atnm[] = "GlueSARoot";
    static char *template =
        "(&(GlueSALocalID=%s)(GlueChunkKey=GlueSEUniqueID=%s))";
    static char *attrs[] = {sa_root_atnm, sa_path_atnm, NULL};
    int bdii_port;
    const char *bdii_server;
    LDAPMessage *entry;
    char *filter;
    LDAP *ld;
    int rc = 0;
    LDAPMessage *reply;
    char **value;
    char *vo;
    int sav_errno = 0;
    int bdii_index = GFAL_BDII_SERVER_FIRST;

    if (!host || !sa_path) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_sa_path][EINVAL] Invalid arguments");
        errno = EINVAL;
        return (-1);
    }

    if (strlen (host) > GFAL_HOSTNAME_MAXLEN) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_sa_path][ENAMETOOLONG] %s: Hostname too long", host);
        errno = ENAMETOOLONG;
        return (-1);
    }

    if ((vo = gfal_get_vo (errbuf, errbufsz)) == NULL)
        return (-1);

    if (salocalid == NULL) {
        salocalid = vo;
    }

    rc = asprintf (&filter, template, salocalid, host);
    if (rc < 0) return (-1);

    *sa_path = *sa_root = NULL;

    while (!*sa_path && !*sa_root && !sav_errno) {
        rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, &bdii_index, errbuf, errbufsz);
        if (rc < 0) {
            sav_errno = errno;
            break;
        }

        if ((entry = ldap_first_entry (ld, reply))) {
            if ((value = ldap_get_values (ld, entry, sa_path_atnm)) != NULL) {
                /* We deal with pre-LCG 2.7.0 where SA Path was incorrect and had vo: prefix */
                if ((strncmp (value[0], vo, strlen (vo)) == 0) && (* (value[0] + strlen (vo)) == ':')) {
                    if ( (*sa_path = strdup (value[0] + strlen (vo) + 1)) == NULL)
                        sav_errno = errno ? errno : ENOMEM;
                } else if ( (*sa_path = strdup (value[0])) == NULL)
                    sav_errno = errno ? errno : ENOMEM;
                ldap_value_free (value);
            }
            else if ((value = ldap_get_values (ld, entry, sa_root_atnm)) != NULL) {
                if ((*sa_root = strdup (value[0] + strlen (vo) + 1)) == NULL)
                    sav_errno = errno ? errno : ENOMEM;
                ldap_value_free (value);
            } else {
                gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN,
                        "[%s][][] %s:%d: Both SAPath and SARoot are not set about VO %s and SE : %s",
                        gfal_remote_type, bdii_server, bdii_port, vo, host);
            }
        } else {
            gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN,
                    "[%s][][] %s:%d: No GlueSA information found about VO %s and SE %s",
                    gfal_remote_type, bdii_server, bdii_port, vo, host);
        }

        bdii_query_free (&ld, &reply);
    }

    free (filter);

    if (!*sa_path && !*sa_root) {
        if (!sav_errno) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                    "[%s][][] No GlueSA information found about VO %s and SE %s",
                    gfal_remote_type, vo, host);
            sav_errno = EINVAL;
        }
        errno = sav_errno;
        return (-1);
    }

    return (0);
}

    static int
get_voinfo (const char *host, const char *spacetokendesc, char **sa_path, char **salocalid, char *errbuf, int errbufsz)
{
    static char sa_path_atnm[] = "GlueVOInfoPath";
    static char sa_key_atnm[] = "GlueChunkKey";
    static char *template =
        "(&%s(GlueVOInfoTag=%s)(GlueChunkKey=GlueSEUniqueID=%s))";
    static char *template2 =
        "(&%s(!(GlueVOInfoTag=*))(GlueChunkKey=GlueSEUniqueID=%s))";
    static char *attrs[] = {sa_key_atnm, sa_path_atnm, NULL};
    int i;
    int bdii_port;
    const char *bdii_server;
    LDAPMessage *entry;
    char *filter, *filter_tmp;
    LDAP *ld;
    int rc = 0;
    LDAPMessage *reply;
    char **value;
    int sav_errno = 0;
    int bdii_index = GFAL_BDII_SERVER_FIRST;

    if (!host || !sa_path || !salocalid) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_voinfo][EINVAL] Invalid arguments");
        errno = EINVAL;
        return (-1);
    }

    if (strlen (host) > GFAL_HOSTNAME_MAXLEN) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_voinfo][ENAMETOOLONG] %s: Hostname too long", host);
        errno = ENAMETOOLONG;
        return (-1);
    }

    if ((filter_tmp = generate_acbr ("GlueVOInfo", errbuf, errbufsz)) == NULL)
        return (-1);

    if (spacetokendesc)
        rc = asprintf (&filter, template, filter_tmp, spacetokendesc, host);
    else
        rc = asprintf (&filter, template2, filter_tmp, host);

    free (filter_tmp);
    if (rc < 0) return (-1);
    *sa_path = *salocalid = NULL;

    while (!*sa_path && !*salocalid && !sav_errno) {
        rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, &bdii_index, errbuf, errbufsz);
        if (rc < 0) {
            sav_errno = errno;
            break;
        }

        if ((entry = ldap_first_entry (ld, reply))) {
            if ((value = ldap_get_values (ld, entry, sa_path_atnm)) != NULL) {
                if ( (*sa_path = strdup (value[0])) == NULL)
                    sav_errno = errno ? errno : ENOMEM;
                ldap_value_free (value);
            }
            else if ((value = ldap_get_values (ld, entry, sa_key_atnm)) != NULL) {
                for (i = 0; value[i] && !*salocalid && !rc; ++i) {
                    if (strncmp (value[i], "GlueSALocalID=", 14) == 0)
                        if ( (*salocalid = strdup (value[i] + 14)) == NULL)
                            sav_errno = errno ? errno : ENOMEM;
                }
                ldap_value_free (value);
            } else {
                if (spacetokendesc)
                    gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_WARN, "[%s][][] %s:%d: [WARN] GlueVOInfo for tag '%s' and SE '%s' wrongly published",
                            gfal_remote_type, bdii_server, bdii_port, spacetokendesc, host);
                else
                    gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_WARN, "[%s][][] %s:%d: [WARN] GlueVOInfo for SE '%s' (with no tag) wrongly published",
                            gfal_remote_type, bdii_server, bdii_port, host);
            }

        } else {
            if (spacetokendesc)
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_INFO, "[%s][][] %s:%d: [INFO] no GlueVOInfo information found about tag '%s' and SE '%s'",
                        gfal_remote_type, bdii_server, bdii_port, spacetokendesc, host);
            else
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_INFO, "[%s][][] %s:%d: [INFO] no GlueVOInfo information found about SE '%s' (with no tag)",
                        gfal_remote_type, bdii_server, bdii_port, host);
        }

        bdii_query_free (&ld, &reply);
    }

    free (filter);

    if (!*sa_path && !*salocalid) {
        if (!sav_errno) {
            if (spacetokendesc)
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_INFO,
                        "[%s][][] [INFO] no GlueVOInfo information found about tag '%s' and SE '%s'",
                        gfal_remote_type, spacetokendesc, host);
            else
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_INFO,
                        "[%s][][] [INFO] no GlueVOInfo information found about SE '%s' (with no tag)",
                        gfal_remote_type, host);
            sav_errno = EINVAL;
        }
        errno = sav_errno;
        return (-1);
    }

    return (0);
}

get_storage_path (const char *host, const char *spacetokendesc, char **sa_path, char **sa_root, char *errbuf, int errbufsz)
{
    char *salocalid = NULL;
    int rc = 0;

    if (!host || !sa_path || !sa_root) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_storage_path][] Invalid arguments");
        errno = EINVAL;
        return (-1);
    }

    *sa_path = *sa_root = NULL;

    rc = get_voinfo (host, spacetokendesc, sa_path, &salocalid, errbuf, errbufsz);

    if (!*sa_path)
        rc = get_sa_path (host, salocalid, sa_path, sa_root, errbuf, errbufsz);

    return (rc);
}


/* get from the BDII the SE type (disk, srm_v1) */

get_se_types_and_endpoints (const char *host, char ***se_types, char ***se_endpoints, char *errbuf, int errbufsz)
{
    static char se_type_atpt[] = "GlueSEPort";
    static char se_type_atst[] = "GlueSEStatus";
    static char se_type_atve[] = "GlueServiceVersion";
    static char se_type_atty[] = "GlueServiceType";
    static char se_type_atep[] = "GlueServiceEndpoint";
    static char *template = "(|(GlueSEUniqueID=%s)(&(GlueServiceType=srm*)(GlueServiceEndpoint=*://%s:%s*)))";
    static char *attrs[] = {se_type_atpt, se_type_atst, se_type_atve, se_type_atty, se_type_atep, NULL};
    char host_tmp[GFAL_HOSTNAME_MAXLEN];
    int len_tmp;
    char *port;
    int bdii_port;
    const char *bdii_server;
    LDAPMessage *entry;
    char filter[2 * GFAL_HOSTNAME_MAXLEN + 110];
    int i, nbentries = 0, n = 0, rc = 0;
    LDAP *ld;
    LDAPMessage *reply;
    char **sep = NULL, **stp = NULL, **st = NULL, **sv = NULL, **ep = NULL, **value = NULL;
    int sav_errno = 0;
    int bdii_index = GFAL_BDII_SERVER_FIRST;

    *se_types = NULL;
    *se_endpoints = NULL;
    len_tmp = strlen (host);

    if (len_tmp >= GFAL_HOSTNAME_MAXLEN) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_se_types_and_endpoints][ENAMETOOLONG] %s: Hostname too long", host);
        errno = ENAMETOOLONG;
        return (-1);
    }

    strncpy (host_tmp, host, GFAL_HOSTNAME_MAXLEN);
    if ((port = strchr (host_tmp, ':')) != NULL)
        *port = 0;

    sprintf (filter, template, host_tmp, host_tmp, port == NULL ? "" : port + 1);

    while (n < 1 && !sav_errno) {
        rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, &bdii_index, errbuf, errbufsz);
        if (rc < 0) {
            sav_errno = errno;
            break;
        }

        if ((nbentries = ldap_count_entries (ld, reply)) < 1) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_WARN,
                    "[%s][][] %s: No entries for host: %s",
                    gfal_remote_type, bdii_server, host);
            bdii_query_free (&ld, &reply);
            continue;
        }

        ++nbentries;
        if ( (st = calloc (nbentries, sizeof (char *))) == NULL ||
                (sv = calloc (nbentries, sizeof (char *))) == NULL ||
                (ep = calloc (nbentries, sizeof (char *))) == NULL ||
                (stp = calloc (nbentries, sizeof (char *))) == NULL ||
                (sep = calloc (nbentries, sizeof (char *))) == NULL) {
            sav_errno = errno ? errno : ENOMEM;
            bdii_query_free (&ld, &reply);
            rc = -1;
            break;
        }

        for (entry = ldap_first_entry (ld, reply);
                entry != NULL && rc == 0;
                entry = ldap_next_entry (ld, entry)) {

            if ((value = ldap_get_values (ld, entry, se_type_atep)) != NULL) {
                // GlueService entry

                ep[n] = strdup (value[0]);
                ldap_value_free (value);

                if ((value = ldap_get_values (ld, entry, se_type_atty)) == NULL) {
                    ldap_value_free (value);
                    free (ep[n]);
                    continue;
                }
                st[n] = strdup (value[0]);
                ldap_value_free (value);

                if ((value = ldap_get_values (ld, entry, se_type_atve)) == NULL) {
                    ldap_value_free (value);
                    free (ep[n]);
                    free (st[n]);
                    continue;
                }
                sv[n] = strdup (value[0]);
                ldap_value_free (value);
                ++n;

            } else {
                // GlueSE entry
                // NB: there is only one GlueSE entry per SE!
                // (even if there are several interfaces, like srm_v1 and srm_v2)

                /* Due to some unofficial values, it has been disabled !!
                 *
                 if ((value = ldap_get_values (ld, entry, se_type_atst)) != NULL &&
                 strcasecmp (value[0], "production") != 0) {
                 gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                 "[%s][][] %s: is not in 'production' status in BDII ('%s')", gfal_remote_type, host, value[0]);
                 ldap_value_free (value);
                 sav_errno = EINVAL;
                 rc = -1;
                 break;
                 }
                 else ;
                 */

                if (port == NULL) {
                    // If port is not yet defined in host_tmp, and is available
                    // it is copied to host_tmp buffer
                    // ... But it will only be used if there is no GlueService entry
                    ldap_value_free (value);
                    value = ldap_get_values (ld, entry, se_type_atpt);
                    if (value == NULL) {
                        continue;
                    } else if (len_tmp + strlen (value[0]) + 1 < GFAL_HOSTNAME_MAXLEN) {
                        port = host_tmp + len_tmp;
                        strcpy (port + 1, value[0]);
                    } else {
                        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_se_types_and_endpoints][ENAMETOOLONG] %s: Hostname too long", host);
                        ldap_value_free (value);
                        sav_errno = ENAMETOOLONG;
                        break;
                    }
                }

                ldap_value_free (value);
            }
        }
        bdii_query_free (&ld, &reply);
    }

    if (rc < 0) {
        for (i = 0; i < n; ++i) {
            if (st && st[i]) free (st[i]);
            if (sv && sv[i]) free (sv[i]);
            if (ep && ep[i]) free (ep[i]);
        }
        if (st) free (st);
        if (sv) free (sv);
        if (ep) free (ep);
        if (stp) free (stp);
        if (sep) free (sep);
        errno = sav_errno ? sav_errno : EINVAL;
    } else {
        if (n > 0) {
            // If there are GlueServices entries...
            for (i = 0; i < n; ++i) {
                if ((strcmp (st[i], "srm_v1") == 0 || strcmp (st[i], "srm_v2") == 0) && ep[i]) {
                    stp[i] = strdup (st[i]);
                    sep[i] = strdup (ep[i]);
                } else if ((strcasecmp (st[i], "SRM") == 0) && (strncmp (sv[i], "1.", 2)) == 0 && ep[i]) {
                    stp[i] = strdup ("srm_v1");
                    sep[i] = strdup (ep[i]);
                } else if ((strcasecmp (st[i], "SRM") == 0) && (strncmp (sv[i], "2.2", 3)) == 0 && ep[i]) {
                    stp[i] = strdup ("srm_v2");
                    sep[i] = strdup (ep[i]);
                }
                free (st[i]);
                free (sv[i]);
                free (ep[i]);
            }
            free (st);
            free (sv);
            free (ep);
        } else if (n == 0 && port != NULL) {
            // There were no GlueService entry...
            // ... so endpoint is hostname:port, and type is disk
            *port = ':';
            stp[0] = strdup ("disk");
            sep[0] = strdup (host_tmp);
        } else {
            free (stp);
            free (sep);
            rc = -1;
            errno = sav_errno ? sav_errno : EINVAL;
        }

        if (rc == 0) {
            *se_types = stp;
            *se_endpoints = sep;
        }
    }

    return (rc);
}

/* get from the BDII the list of supported protocols with their associated
 * port number
 */

get_seap_info (const char *host, char ***access_protocol, int **port, char *errbuf, int errbufsz)
{
    static char proto_port[] = "GlueSEAccessProtocolPort";
    static char proto_type[] = "GlueSEAccessProtocolType";
    static char *template = "(&(GlueSEAccessProtocolType=*)(GlueChunkKey=GlueSEUniqueID=%s))";
    char **ap = NULL;
    char *attr = NULL;
    static char *attrs[] = {proto_type, proto_port, NULL};
    int bdii_port = 0;
    const char *bdii_server;
    BerElement *ber = NULL;
    LDAPMessage *entry = NULL;
    char filter[GFAL_HOSTNAME_MAXLEN + 70];
    int i = 0, n = 0, rc = 0;
    LDAP *ld = NULL;
    int nbentries = 0;
    int *pn = NULL;
    LDAPMessage *reply = NULL;
    char **value = NULL;
    int sav_errno = 0;
    int bdii_index = GFAL_BDII_SERVER_FIRST;

    if (strlen (template) + strlen (host) - 2 >= sizeof (filter)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_seap_info][ENAMETOOLONG] %s: Hostname too long", host);
        errno = ENAMETOOLONG;
        return (-1);
    }
    sprintf (filter, template, host);

    while (n < 1 && !sav_errno) {
        rc = bdii_query_send (&ld, filter, attrs, &reply, &bdii_server, &bdii_port, &bdii_index, errbuf, errbufsz);
        if (rc < 0) {
            sav_errno = errno;
            break;
        }

        nbentries = ldap_count_entries (ld, reply);
        nbentries++;
        if ( (ap = calloc (nbentries, sizeof (char *))) == NULL ||
                (pn = calloc (nbentries, sizeof (int))) == NULL) {
            sav_errno = errno ? errno : ENOMEM;
            bdii_query_free (&ld, &reply);
            rc = -1;
            break;
        }

        for (entry = ldap_first_entry (ld, reply);
                entry != NULL;
                entry = ldap_next_entry (ld, entry)) {
            for (attr = ldap_first_attribute (ld, entry, &ber);
                    attr != NULL;
                    attr = ldap_next_attribute (ld, entry, ber)) {
                value = ldap_get_values (ld, entry, attr);
                if (value == NULL) {
                    continue;
                }
                if (strcmp (attr, "GlueSEAccessProtocolType") == 0) {
                    if ( (ap[n] = strdup (value[0])) == NULL) {
                        sav_errno = errno ? errno : ENOMEM;
                        ldap_unbind (ld);
                        errno = sav_errno;
                        return (-1);
                    }
                } else
                    pn[n] = atoi (value[0]);

                ++n;
                ldap_value_free (value);
            }
        }
        ber_free (ber, 0);
        bdii_query_free (&ld, &reply);
    }

    if (rc < 0) {
        for (i = 0; i < n; ++i)
            free (ap[n]);
        free (ap);
        free (pn);
        errno = sav_errno ? sav_errno : EINVAL;
    } else {
        *access_protocol = ap;
        *port = pn;
    }
    return (rc);
}
