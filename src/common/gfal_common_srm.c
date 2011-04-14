/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file gfal_common.c
 * @brief the header file with the main srm funcs of the common API
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/04/2011
 * */
 
#include "gfal_common.h"
#include "gfal_common_srm.h"

/**
 * initiate a gfal's context with default parameters for use
 * @return a gfal_handle, need to be free after usage or NULL if errors
 */
gfal_handle gfal_initG (GError** err)
{
	gfal_handle handle = malloc(sizeof(struct gfal_handle_));
	if(handle == NULL){
		errno= ENOMEM;
		g_set_error(err,0,ENOMEM, "[gfal_initG] bad allocation, no more memory free");
		return NULL;
	}
	memset((void*)handle,0,sizeof(struct gfal_handle_));
	handle->err= NULL;
	handle->srm_proto_type = PROTO_SRMv2;
	/*
    int i = 0;
    char **se_endpoints;
    char **se_types;
    char *srmv1_endpoint = NULL;
    char *srmv2_endpoint = NULL;
    int isclassicse = 0;
    int endpoint_offset=0;

    if (req == NULL || req->nbfiles < 0 || (req->endpoint == NULL && req->surls == NULL)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] Invalid request: Endpoint or SURLs must be specified");
        errno = EINVAL;
        return (-1);
    }
    if (req->oflag != 0 && req->filesizes == NULL) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] Invalid request: File sizes must be specified for put requests");
        errno = EINVAL;
        return (-1);
    }
    if (req->srmv2_lslevels > 1) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] Invalid request: srmv2_lslevels must be 0 or 1");
        errno = EINVAL;
        return (-1);
    }

    if ((*gfal = (gfal_handle) malloc (sizeof (struct gfal_handle_))) == NULL) {
        errno = ENOMEM;
        return (-1);
    }

    memset (*gfal, 0, sizeof (struct gfal_handle_));
    memcpy (*gfal, req, sizeof (struct gfal_request_));
    /// Use default SRM timeout if not specified in request 
    if (!(*gfal)->timeout)
        (*gfal)->timeout = gfal_get_timeout_srm ();

    if ((*gfal)->no_bdii_check) {
        if ((*gfal)->surls != NULL && ((*gfal)->setype != TYPE_NONE ||
                    ((*gfal)->setype = (*gfal)->defaultsetype) != TYPE_NONE)) {
            if ((*gfal)->setype == TYPE_SE) {
                gfal_handle_free (*gfal);
                *gfal = NULL;
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                        "[GFAL][gfal_init][EINVAL] Invalid request: Disabling BDII checks is not compatible with Classic SEs");
                errno = EINVAL;
                return (-1);
            }
            else if ((*gfal)->setype != TYPE_SE && (*gfal)->endpoint == NULL && ((*gfal)->free_endpoint = 1) &&
                    ((*gfal)->endpoint = endpointfromsurl ((*gfal)->surls[0], errbuf, errbufsz, 1)) == NULL) {
                gfal_handle_free (*gfal);
                *gfal = NULL;
                return (-1);
            }
            else {
                // Check if the endpoint is full or not 
                if(strncmp ((*gfal)->endpoint, ENDPOINT_DEFAULT_PREFIX, ENDPOINT_DEFAULT_PREFIX_LEN) == 0)
                    endpoint_offset = ENDPOINT_DEFAULT_PREFIX_LEN;
                else
                    endpoint_offset = 0;
                const char *s = strchr ((*gfal)->endpoint + endpoint_offset, '/');
                const char *p = strchr ((*gfal)->endpoint + endpoint_offset, ':');

                if (((*gfal)->setype == TYPE_SRMv2 && s == NULL) || p == NULL || (s != NULL && s < p)) {
                    gfal_handle_free (*gfal);
                    *gfal = NULL;
                    gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                            "[GFAL][gfal_init][EINVAL] Invalid request: When BDII checks are disabled, you must provide full endpoint");
                    errno = EINVAL;
                    return (-1);
                }

                return (0);
            }

        } else {
            gfal_handle_free (*gfal);
            *gfal = NULL;
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                    "[GFAL][gfal_init][EINVAL] Invalid request: When BDII checks are disabled, you must provide SURLs and endpoint types");
            errno = EINVAL;
            return (-1);
        }
    }

    if ((*gfal)->endpoint == NULL) {
        // (*gfal)->surls != NULL 
        if ((*gfal)->surls[0] != NULL) {
            if (((*gfal)->endpoint = endpointfromsurl ((*gfal)->surls[0], errbuf, errbufsz, 0)) == NULL)
                return (-1);
            (*gfal)->free_endpoint = 1;
        } else {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                    "[GFAL][gfal_init][EINVAL] Invalid request: You have to specify either an endpoint or at least one SURL");
            gfal_handle_free (*gfal);
            *gfal = NULL;
            errno = EINVAL;
            return (-1);
        }
    }
    if ((strchr ((*gfal)->endpoint, '.') == NULL)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] No domain name specified for storage element endpoint");
        gfal_handle_free (*gfal);
        *gfal = NULL;
        errno = EINVAL;
        return (-1);
    }
    if (setypesandendpoints ((*gfal)->endpoint, &se_types, &se_endpoints, errbuf, errbufsz) < 0) {
        gfal_handle_free (*gfal);
        *gfal = NULL;
        return (-1);
    }

    while (se_types[i]) {
        if (srmv1_endpoint == NULL && strcmp (se_types[i], "srm_v1") == 0)
            srmv1_endpoint = se_endpoints[i];
        else if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
            srmv2_endpoint = se_endpoints[i];
        else {
            free (se_endpoints[i]);
            if ((strcmp (se_types[i], "disk")) == 0)
                isclassicse = 1;
        }

        free (se_types[i]);
        ++i;
    }

    free (se_types);
    free (se_endpoints);

    if ((*gfal)->surls != NULL && strncmp ((*gfal)->surls[0], "sfn:", 4) == 0 &&
            (isclassicse || srmv1_endpoint || srmv2_endpoint)) {
        // if surls start with sfn:, we force the SE type to classic SE //
        (*gfal)->setype = TYPE_SE;
        if (srmv1_endpoint) free (srmv1_endpoint);
        if (srmv2_endpoint) free (srmv2_endpoint);
        return (0);
    }

    // srmv2 is the default if nothing specified by user! //
    if ((*gfal)->setype == TYPE_NONE) {
        if (srmv2_endpoint &&
                ((*gfal)->defaultsetype == TYPE_NONE || (*gfal)->defaultsetype == TYPE_SRMv2 ||
                 ((*gfal)->defaultsetype == TYPE_SRM && !srmv1_endpoint))) {
            (*gfal)->setype = TYPE_SRMv2;
        } else if (!(*gfal)->srmv2_spacetokendesc && !(*gfal)->srmv2_desiredpintime &&
                !(*gfal)->srmv2_lslevels && !(*gfal)->srmv2_lsoffset &&	!(*gfal)->srmv2_lscount) {
            if (srmv1_endpoint && (*gfal)->defaultsetype != TYPE_SE)
                (*gfal)->setype = TYPE_SRM;
            else if (srmv2_endpoint || srmv1_endpoint || isclassicse)
                (*gfal)->setype = TYPE_SE;
        }
    }
    else if ((*gfal)->setype == TYPE_SRMv2 && !srmv2_endpoint) {
        (*gfal)->setype = TYPE_NONE;
    } else if ((*gfal)->srmv2_spacetokendesc || (*gfal)->srmv2_desiredpintime ||
            (*gfal)->srmv2_lslevels || (*gfal)->srmv2_lsoffset || (*gfal)->srmv2_lscount) {
        (*gfal)->setype = TYPE_NONE;
    } else {
        if ((*gfal)->setype == TYPE_SRM && !srmv1_endpoint)
            (*gfal)->setype = TYPE_NONE;
        else if ((*gfal)->setype == TYPE_SE && !srmv2_endpoint && !srmv1_endpoint && !isclassicse)
            (*gfal)->setype = TYPE_NONE;
    }

    if ((*gfal)->setype == TYPE_SRMv2) {
        if ((*gfal)->free_endpoint) free ((*gfal)->endpoint);
        (*gfal)->endpoint = srmv2_endpoint;
        (*gfal)->free_endpoint = 1;
        if (srmv1_endpoint) free (srmv1_endpoint);
    } else if ((*gfal)->setype == TYPE_SRM) {
        if ((*gfal)->free_endpoint) free ((*gfal)->endpoint);
        (*gfal)->endpoint = srmv1_endpoint;
        (*gfal)->free_endpoint = 1;
        if (srmv2_endpoint) free (srmv2_endpoint);
    } else if ((*gfal)->setype == TYPE_NONE) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] Invalid request: Desired SE type doesn't match request parameters or SE");
        gfal_handle_free (*gfal);
        *gfal = NULL;
        if (srmv1_endpoint) free (srmv1_endpoint);
        if (srmv2_endpoint) free (srmv2_endpoint);
        errno = EINVAL;
        return (-1);
    }

    if ((*gfal)->generatesurls) {
        if ((*gfal)->surls == NULL) {
            if (generate_surls (*gfal, errbuf, errbufsz) < 0)
                return (-1);
        } else {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                    "[GFAL][gfal_init][EINVAL] Invalid request: No SURLs must be specified with 'generatesurls' activated");
            gfal_handle_free (*gfal);
            *gfal = NULL;
            errno = EINVAL;
            return (-1);
        }
    }
	*/
}

void gfal_set_default_storage(gfal_handle handle, enum gfal_srm_proto proto){
	handle->srm_proto_type = proto;
}

/**
 * @brief free a gfal's context and set to NULL
 * 
 */
void gfal_handle_freeG (gfal_handle handle){
	g_clear_error(&(handle->err));
	free(handle);
	handle = NULL;
}
