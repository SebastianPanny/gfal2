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


static gboolean gfal_handle_checkG(gfal_handle handle, GError** err){
	if(handle->initiated == 1)
		return TRUE;
	g_set_error(err,0, EINVAL,"[gboolean gfal_handle_checkG] gfal_handle not set correctly");
	return FALSE;
}

/**
 * @brief end point associated with the list
 *  determine the best endpoint associated with the list of url and the params of the actual handle
 *  the returned endpoint need to be free after use
 * */
static char* gfal_get_srm_endpoint(gfal_handle handle, GList* surls, GError** err){
	return strdup("httpg://hlxdpm101.cern.ch/dpm/cern.ch"); // fix it
}

/**
 * initiate a gfal's context with default parameters for use
 * @return a gfal_handle, need to be free after usage or NULL if errors
 */
gfal_handle gfal_initG (GError** err)
{
	gfal_handle handle = calloc(1,sizeof(struct gfal_handle_));// clear allocation of the struct and set defautl options
	if(handle == NULL){
		errno= ENOMEM;
		g_set_error(err,0,ENOMEM, "[gfal_initG] bad allocation, no more memory free");
		return NULL;
	}
	handle->err= NULL;
	handle->srm_proto_type = PROTO_SRMv2;
	handle->initiated = 1;
	handle->srmv2_opt = calloc(1,sizeof(struct _gfal_srmv2_opt));	// define the srmv2 option struct and clear it
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

/**
 * @brief accessor for the default storage type definition
 * */
void gfal_set_default_storageG(gfal_handle handle, enum gfal_srm_proto proto){
	handle->srm_proto_type = proto;
}

/**
 * convert glist to table char**
 */
char** gfal_GList_to_tab(GList* surls){
	int surl_size = g_list_length(surls);
	int i;
	char ** resu = surl_size?((char**)calloc(surl_size+1, sizeof(char*))):NULL;
	for(i=0;i<surl_size; ++i){
		resu[i]= surls->data;
		surls = g_list_next(surls);
	}
	return resu;
}


/**
 * parse a surl to check the validity
 */
int gfal_surl_checker(const char* surl, GError** err){
	regex_t rex;
	int ret = regcomp(&rex, "^srm://.*",REG_ICASE);
	g_return_val_err_if_fail(ret==0,-1,err,"[gfal_surl_checker_] fail to compile regex");
	ret=  regexec(&rex,surl,0,NULL,0);
	return 0;
} 

/**
 *  @brief execute a srmv2 request async
*/
static int gfal_srmv2_getasync(gfal_handle handle, GList* surls, GError** err){
	g_return_val_err_if_fail(surls!=NULL,-1,err,"[gfal_srmv2_getasync] GList passed null");
			
	GError* tmp_err=NULL;
	struct srm_context context;
	int ret=0;
	struct srm_preparetoget_input preparetoget_input;
	struct srm_preparetoget_output preparetoget_output;
	const int size = 2048;
	
	char errbuf[size] ; memset(errbuf,0,size*sizeof(char));
	char *endpoint = gfal_get_srm_endpoint(handle, surls, &tmp_err);		// get endpoint
	const gfal_srmv2_opt* opts = handle->srmv2_opt;
	int surl_size = g_list_length(surls);
	char**  surls_tab = gfal_GList_to_tab(surls);
	
	
	// set the structures datafields	
	preparetoget_input.desiredpintime = opts->opt_srmv2_desiredpintime;		
	preparetoget_input.nbfiles = surl_size;
	preparetoget_input.protocols = opts->opt_srmv2_protocols;
	preparetoget_input.spacetokendesc = opts->opt_srmv2_spacetokendesc;
	preparetoget_input.surls = surls_tab;	
	srm_context_init(&context, endpoint, errbuf, size, gfal_get_verbose());	
	
	
	ret = srm_prepare_to_get_async(&context,&preparetoget_input,&preparetoget_output);
	if(ret != 0){
		g_set_error(err,0,errno,"[gfal_srmv2_getasync] call to srm_ifce error: %s",errbuf);
	} else{
		handle->last_request_state = calloc(1, sizeof(struct _gfal_request_state));
    	handle->last_request_state->srmv2_token = preparetoget_output.token;
    	handle->last_request_state->srmv2_pinstatuses = preparetoget_output.filestatuses;		
	}

	free(endpoint);
	free(surls_tab);
	return ret;	
}



/**
 * @brief launch a surls-> turls translation in asynchronous mode
 * @warning need a initiaed gfal_handle
 * @param handle : the gfal_handle initiated ( \ref gfal_init )
 * @param surls : GList of string of the differents surls to convert
 * @param err : GError for error report
 * @return return 0 if success else -1, check GError for more information
 */
int gfal_get_asyncG(gfal_handle handle, GList* surls, GError** err){
	g_return_val_err_if_fail(handle!=NULL,-1,err,"[gfal_get_asyncG] handle passed is null");
	g_return_val_err_if_fail(surls!=NULL,-1,err,"[gfal_get_asyncG] GList passed is null");
		
	GError* tmp_err=NULL;
	int ret=0;
	if( !gfal_handle_checkG(handle, &tmp_err) ){	// check handle validityo
		g_propagate_prefixed_error(err,tmp_err,"[gfal_get_asyncG]");
		return -1;
	}
	if (handle->srm_proto_type == PROTO_SRMv2){
		ret= gfal_srmv2_getasync(handle,surls,&tmp_err);
		if(ret<0)
			g_propagate_prefixed_error(err, tmp_err, "[gfal_get_asyncG]");
	} else if(handle->srm_proto_type == PROTO_SRM){
			
	} else{
		ret=-1;
		g_set_error(err,0,EPROTONOSUPPORT, "[gfal_get_asyncG] Type de protocole spécifié non supporté ( Supportés : SRM & SRMv2 ) ");
	}
	return ret;

	/*int ret;

    if (check_gfal_handle (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
    	struct srm_preparetoget_input preparetoget_input;
    	struct srm_preparetoget_output preparetoget_output;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_get_verbose());

        if (req->srmv2_pinstatuses)
        {
            free (req->srmv2_pinstatuses);
            req->srmv2_pinstatuses = NULL;
        }
        if (req->srmv2_token)
        {
            free (req->srmv2_token);
            req->srmv2_token = NULL;
        }

        preparetoget_input.desiredpintime = req->srmv2_desiredpintime;
        preparetoget_input.nbfiles = req->nbfiles;
        preparetoget_input.protocols = req->protocols;
        preparetoget_input.spacetokendesc = req->srmv2_spacetokendesc;
        preparetoget_input.surls = req->surls;

        ret = srm_prepare_to_get_async(&context,&preparetoget_input,&preparetoget_output);

    	req->srmv2_token = preparetoget_output.token;
    	req->srmv2_pinstatuses = preparetoget_output.filestatuses;


        //TODO ret = srmv2_gete (req->nbfiles, (const char **) req->surls, req->endpoint,
         //   req->srmv2_desiredpintime, req->srmv2_spacetokendesc, req->protocols,
         //   &(req->srmv2_token), &(req->srmv2_pinstatuses), errbuf, errbufsz, req->timeout);

    } else if (req->setype == TYPE_SRM) {
        if (req->srm_statuses) {
            free (req->srm_statuses);
            req->srm_statuses = NULL;
        }
//TODO REMOVE        ret = srm_getxe (req->nbfiles, (const char **) req->surls, req->endpoint, req->protocols,
 //               &(req->srm_reqid), &(req->srm_statuses), errbuf, errbufsz, req->timeout);
    } else { // req->setype == TYPE_SE
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_get][EPROTONOSUPPORT] SFNs aren't supported");
        errno = EPROTONOSUPPORT;
        return (-1);;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, PIN_STATUS));*/
	
}

/**
 * @brief free a gfal's handle, safe if null
 * 
 */
void gfal_handle_freeG (gfal_handle handle){
	if(handle == NULL)
		return;
	g_clear_error(&(handle->err));
	free(handle->srmv2_opt);
	free(handle->last_request_state);
	free(handle);
	handle = NULL;
}
