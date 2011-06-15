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
 
 #define _GNU_SOURCE 

#include <regex.h>
#include <time.h> 
 
#include "gfal_common_srm.h"
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_catalog.h"
#include "gfal_common_srm_access.h"
#include "gfal_common_srm_internal_layer.h"

/**
 * list of the protols in the order of preference
 */
static enum gfal_srm_proto gfal_proto_list_pref[]= { PROTO_SRMv2, PROTO_SRM, PROTO_ERROR_UNKNOW };


/**
 * parse a surl to check the validity
 */
int gfal_surl_checker(const char* surl, GError** err){
	g_return_val_err_if_fail(surl != NULL,-1,err,"[gfal_surl_checker_] check URL failed : surl is empty");
	regex_t rex;
	int ret = regcomp(&rex, "^srm://([:alnum:]|-|/|\.|_)+$",REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret==0,-1,err,"[gfal_surl_checker_] fail to compile regex, report this bug");
	ret=  regexec(&rex,surl,0,NULL,0);
	if(ret) 
		g_set_error(err,0,EINVAL,"[gfal_surl_checker] Incorrect surl, impossible to parse surl %s :", surl);
	regfree(&rex);
	return ret;
} 

/**
 * url checker for the srm module, surl part
 * 
 * */
static gboolean gfal_srm_check_url(catalog_handle handle, const char* url, catalog_mode mode, GError** err){
	switch(mode){
		case GFAL_CATALOG_GETTURL:
			return (gfal_surl_checker(url,  err)==0)?TRUE:FALSE;
		default:
			return FALSE;		
	}
}
/**
 * destroyer function, call when the module is unload
 * */
static void gfal_srm_destroyG(catalog_handle handle){
	
}

/**
 * Init function, called before all
 * */
gfal_catalog_interface gfal_srm_initG(gfal_handle handle, GError** err){
	gfal_catalog_interface srm_catalog;
	GError* tmp_err=NULL;
	memset(&srm_catalog,0,sizeof(gfal_catalog_interface));	// clear the catalog	

	srm_catalog.handle = (void*) handle;	
	srm_catalog.check_catalog_url = &gfal_srm_check_url;
	srm_catalog.catalog_delete = &gfal_srm_destroyG;
	srm_catalog.accessG = &gfal_srm_accessG;
	return srm_catalog;
}





/**
 * check the validity of the current handle
 */
static gboolean gfal_handle_checkG(gfal_handle handle, GError** err){
	if(handle->initiated == 1)
		return TRUE;
	g_set_error(err,0, EINVAL,"[gboolean gfal_handle_checkG] gfal_handle not set correctly");
	return FALSE;
}




/**
 *  @brief create a full endpath from a surl with full endpath
 * */
char* gfal_get_fullendpoint(const char* surl, GError** err){
	char* p = strstr(surl,"?SFN=");
	const int len_prefix = strlen(GFAL_PREFIX_SRM);						// get the srm prefix length
	const int len_endpoint_prefix = strlen(GFAL_ENDPOINT_DEFAULT_PREFIX); // get the endpoint protocol prefix len 
	g_return_val_err_if_fail(p && len_prefix && (p>(surl+len_prefix)) && len_endpoint_prefix,NULL,err,"[gfal_get_fullendpoint] full surl must contain ?SFN= and a valid prefix, fatal error");	// assertion on params
	char* resu = calloc(p-surl-len_prefix+len_endpoint_prefix, sizeof(char));	
	strncpy(resu, GFAL_ENDPOINT_DEFAULT_PREFIX, len_endpoint_prefix);	// copy prefix
	strncpy(resu + len_endpoint_prefix, surl+len_prefix, p- surl-len_prefix);		// copy endpoint
	return resu;
}

/**
 * @brief get the hostname from a surl
 *  @return return NULL if error and set err else return the hostname value
 */
 char*  gfal_get_hostname_from_surl(const char * surl, GError** err){
	 const int srm_prefix_len = strlen(GFAL_PREFIX_SRM);
	 const int surl_len = strnlen(surl,2048);
	 g_return_val_err_if_fail(surl &&  (srm_prefix_len < surl_len)  && (surl_len < 2048),NULL, err, "[gfal_get_hostname_from_surl] invalid value in params");
	 char* p = strchr(surl+srm_prefix_len,'/');
	 char* prep = strstr(surl, GFAL_PREFIX_SRM);
	 if(prep != surl){
		 g_set_error(err,0, EINVAL, "[gfal_get_hostname_from_surl not a valid surl");
		 return NULL;
	 }
	 return strndup(surl+srm_prefix_len, p-surl-srm_prefix_len);	 
 }
 
 /**
  * map a bdii se protocol type to a gfal protocol type
  */
static enum gfal_srm_proto gfal_get_proto_from_bdii(const char* se_type_bdii){
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

/**
 * select the best protocol choice and the best endpoint choice  from a list of protocol and endpoints obtained by the bdii
 * 
 */
int gfal_select_best_protocol_and_endpoint(gfal_handle handle, char** endpoint, enum gfal_srm_proto* srm_type, char** tab_se_type, char** tab_endpoint, GError** err){
	g_return_val_err_if_fail(handle && endpoint && srm_type && tab_se_type && tab_endpoint, -1, err, "[gfal_select_best_protocol_and_endpoint] Invalid value");
	int i=0;
	char** pse =tab_se_type;
	char** pendpoint = tab_endpoint;
	enum gfal_srm_proto* p_pref = &(handle->srm_proto_type);	
	while( *p_pref != PROTO_ERROR_UNKNOW){
		while(pse != NULL &&  tab_endpoint != NULL ){
			if( *p_pref == gfal_get_proto_from_bdii(*pse) ){ // test if the response is the actual preferred response
				*endpoint = strndup(*tab_endpoint,2048);
				*srm_type = *p_pref;
				return 0;
			}
			tab_endpoint++;
			pse++;
		}	
		if(p_pref == &(handle->srm_proto_type)) // switch desired proto to the list if the default choice is not in the list
			p_pref=gfal_proto_list_pref;
		else
			p_pref++;
	}
	g_set_error(err,0, EINVAL, "[gfal_select_best_protocol_and_endpoint] cannot obtain a valid protocol from the bdii response, fatal error");
	return -2;
	
}
 
  
/**
 * @brief get endpoint from the bdii system only
 * @return 0 if success with endpoint and srm_type set correctly else -1 and err set
 * 
 * */
int gfal_get_endpoint_and_setype_from_bdii(gfal_handle handle, char** endpoint, enum gfal_srm_proto* srm_type, GList* surls, GError** err){
	g_return_val_err_if_fail(handle && endpoint && srm_type && surls, -1, err, "[gfal_get_endpoint_and_setype_from_bdii] invalid parameters");
	char** tab_endpoint=NULL;
	char** tab_se_type=NULL;
	char * hostname;
	int ret =-1, i;
	GError* tmp_err=NULL;
	if( (hostname = gfal_get_hostname_from_surl(surls->data, &tmp_err)) == NULL){ 		// get the hostname
		g_propagate_prefixed_error(err, tmp_err, "[gfal_get_endpoint_and_setype_from_bdii]");
		return -1;
	}

	if( (ret =gfal_mds_get_se_types_and_endpoints( hostname,  &tab_se_type, &tab_endpoint, &tmp_err)) != 0){ // questioning the bdii
		g_propagate_prefixed_error(err, tmp_err, "[gfal_get_endpoint_and_setype_from_bdii]");
		free(hostname);
		return -2;
	}
	if( gfal_select_best_protocol_and_endpoint(handle, endpoint, srm_type, tab_se_type, tab_endpoint, &tmp_err) != 0){ // map the response if correct
		g_propagate_prefixed_error(err, tmp_err, "[gfal_get_endpoint_and_setype_from_bdii]");
		free(hostname);
		return -3;		
	}
	for(i=0; tab_endpoint[i] != NULL && tab_se_type[i] != NULL;i++){
		free(tab_endpoint[i]);
		free(tab_se_type[i]);
	}
	free(tab_endpoint);
	free(tab_se_type);
	free(hostname);
	
	return 0;
	
}


/**
 * @brief get endpoint and srm_associated type from a list of surls
 *  determine the best endpoint associated with the list of url and the params of the actual handle (no bdii check or not)
 *  see the diagram in doc/diagrams/surls_get_endpoint_activity_diagram.svg for more informations
 *  @return return 0 with endpoint and types set if success else -1 and set Error
 * */
int gfal_auto_get_srm_endpoint(gfal_handle handle, char** endpoint, enum gfal_srm_proto* srm_type, GList* surls, GError** err){
	g_return_val_err_if_fail(handle && endpoint && srm_type && surls,-1, err, "[gfal_auto_get_srm_endpoint] invalid value in params"); // check params
	
	GError* tmp_err=NULL;
	char * tmp_endpoint=NULL;;
	gboolean isFullEndpoint = gfal_check_fullendpoint_in_surl(surls->data, &tmp_err)?FALSE:TRUE;		// check if a full endpoint exist
	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[gfal_auto_get_srm_endpoint]");
		return -2;
	}
	if(handle->no_bdii_check == TRUE && isFullEndpoint == FALSE){ // no bdii checked + no full endpoint provided = error 
		g_set_error(err,0,EINVAL,"[gfal_auto_get_srm_endpoint] no_bdii_check option need a full endpoint in the first surl");
		return -3;
	}
	if( isFullEndpoint == TRUE  ){ // if full endpoint contained in url, get it and set type to default type
		if( (tmp_endpoint = gfal_get_fullendpoint(surls->data,&tmp_err) ) == NULL){
				g_propagate_prefixed_error(err, tmp_err, "[gfal_aut_get_srm_endpoint]");
				return -4;
			}
		*endpoint = tmp_endpoint;
		*srm_type= handle->srm_proto_type;
		return 0;
	}
	if( gfal_get_endpoint_and_setype_from_bdii(handle, endpoint, srm_type, surls, &tmp_err) != 0){
		g_propagate_prefixed_error(err, tmp_err, "[gfal_auto_get_srm_endpoint]");
		return -1;
	}
	return 0;
}

/**
 *  @brief get endpoint and srm_associated type from a a surl
 *  determine the best endpoint associated with the list of url and the params of the actual handle (no bdii check or not)
 *  see the diagram in doc/diagrams/surls_get_endpoint_activity_diagram.svg for more informations
 *  @return return 0 with endpoint and types set if success else -1 and set Error
 * */
int gfal_auto_get_srm_endpoint_for_surl(gfal_handle handle, char** endpoint,
												enum gfal_srm_proto* srm_type, const char* surl, GError** err){
	GList* list = g_list_append(NULL, (char*) surl);
	const int ret = gfal_auto_get_srm_endpoint(handle, endpoint, srm_type, list, err);
	g_list_free(list);
	return ret;
	
}



/**
 * @brief accessor for the default storage type definition
 * */
void gfal_set_default_storageG(gfal_handle handle, enum gfal_srm_proto proto){
	handle->srm_proto_type = proto;
}




/**
 * delete properly a gfal_request_state structure
 * 
 * */
void gfal_delete_request_state(gfal_request_state* request_state){
	 if(request_state){
		 free(request_state->request_endpoint);
		 free(request_state->srmv2_token);
		 free(request_state->srmv2_statuses);
		 gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(request_state->srmv2_pinstatuses, request_state->number);
		 free(request_state);
	 }
 }
 
/**
 * create a new current request state and init it, delete the old one if exist
 * 
 * */
void gfal_new_request_state(gfal_handle handle){
	if(handle->last_request_state){
		gfal_delete_request_state(handle->last_request_state);
	}
	handle->last_request_state = calloc(1, sizeof(struct _gfal_request_state));		
}





/**
 *  @brief execute a srmv2 request async "GET" on the srm_ifce
*/
static int gfal_getasync_srmv2(gfal_handle handle, char* endpoint, GList* surls, GError** err){
	g_return_val_err_if_fail(surls!=NULL,-1,err,"[gfal_srmv2_getasync] GList passed null");
			
	GError* tmp_err=NULL;
	struct srm_context context;
	int ret=0,i=0;
	struct srm_preparetoget_input preparetoget_input;
	struct srm_preparetoget_output preparetoget_output;
	const int size = 2048;
	
	char errbuf[size] ; memset(errbuf,0,size*sizeof(char));
	const gfal_srmv2_opt* opts = handle->srmv2_opt;							// get default opts for srmv2
	int surl_size = g_list_length(surls);										// get the length of glist
	
	char**  surls_tab = gfal_GList_to_tab(surls);								// convert glist
	
	
	// set the structures datafields	
	preparetoget_input.desiredpintime = opts->opt_srmv2_desiredpintime;		
	preparetoget_input.nbfiles = surl_size;
	preparetoget_input.protocols = opts->opt_srmv2_protocols;
	preparetoget_input.spacetokendesc = opts->opt_srmv2_spacetokendesc;
	preparetoget_input.surls = surls_tab;	
	gfal_srm_external_call.srm_context_init(&context, endpoint, errbuf, size, gfal_get_verbose());	
	
	
	ret = srm_prepare_to_get_async(&context,&preparetoget_input,&preparetoget_output);
	if(ret < 0){
		g_set_error(err,0,errno,"[gfal_srmv2_getasync] call to srm_ifce error: %s",errbuf);
	} else{
		gfal_new_request_state(handle);								// copy the informations of the current request in the last request information structure
		gfal_request_state * req_state = handle->last_request_state;
    	req_state->srmv2_token = preparetoget_output.token;
    	req_state->srmv2_pinstatuses = preparetoget_output.filestatuses;
    	req_state->number = ret;	
    	req_state->current_request_proto = PROTO_SRMv2;	
    	req_state->request_endpoint = strndup(endpoint, 2048);
    	req_state->finished = FALSE;
    	gfal_srm_external_call.srm_srm2__TReturnStatus_delete(preparetoget_output.retstatus);
	}
	free(surls_tab);
	return ret;	
}

/**
 *  return 0 if a full endpoint is contained in surl  
 * 
*/
int gfal_check_fullendpoint_in_surl(const char * surl, GError ** err){
	regex_t rex;
	int ret = regcomp(&rex, "^srm://([:alnum:]|-|/|\.|_)+:[0-9]+/([:alnum:]|-|/|\.|_)+?SFN=",REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret==0,-1,err,"[gfal_check_fullendpoint_in_surl] fail to compile regex, report this bug");
	ret=  regexec(&rex,surl,0,NULL,0);
	regfree(&rex);
	return ret;	
}

/**
 * @brief launch a surls-> turls translation in asynchronous mode
 * @warning need a initiaed gfal_handle
 * @param handle : the gfal_handle initiated ( \ref gfal_init )
 * @param surls : GList of string of the differents surls to convert
 * @param err : GError** for error report
 * @return return positive if success else -1, check GError for more information
 */
int gfal_get_asyncG(gfal_handle handle, GList* surls, GError** err){
	g_return_val_err_if_fail(handle!=NULL,-1,err,"[gfal_get_asyncG] handle passed is null");
	g_return_val_err_if_fail(surls!=NULL ,-2,err,"[gfal_get_asyncG] surls arg passed is null");
	g_return_val_err_if_fail(g_list_last(surls) != NULL,-3,err,"[gfal_get_asyncG] surls arg passed is empty");
	
	GError* tmp_err=NULL;
	GList* tmp_list = surls;
	int ret=0;
	
	
	if( !gfal_handle_checkG(handle, &tmp_err) ){	// check handle validity
		g_propagate_prefixed_error(err,tmp_err,"[gfal_get_asyncG]");
		return -1;
	}
	while(tmp_list != NULL){							// check all urls if valids
		if( gfal_surl_checker(tmp_list->data,&tmp_err) != 0){
			g_propagate_prefixed_error(err,tmp_err,"[gfal_get_asyncG]");	
			return -1;
		}
		tmp_list = g_list_next(tmp_list);		
	}
			
	char* full_endpoint=NULL;
	enum gfal_srm_proto srm_types;
	if((ret = gfal_auto_get_srm_endpoint(handle, &full_endpoint,&srm_types, surls, &tmp_err)) != 0){		// check & get endpoint										
		g_propagate_prefixed_error(err,tmp_err, "[gfal_get_asyncG]");
		return -1;
	}
	gfal_print_verbose(GFAL_VERBOSE_NORMAL, "[gfal_get_asyncG] endpoint %s", full_endpoint);
	
	if (srm_types == PROTO_SRMv2){
		ret= gfal_getasync_srmv2(handle, full_endpoint, surls,&tmp_err);
		if(ret <0)
			g_propagate_prefixed_error(err, tmp_err, "[gfal_get_asyncG]");
	} else if(srm_types == PROTO_SRM){
			g_set_error(err,0, EPROTONOSUPPORT, "[gfal_get_asyncG] support for SRMv1 is removed in 2.0, failure");
			ret =  -1;
	} else{
		ret=-1;
		g_set_error(err,0,EPROTONOSUPPORT, "[gfal_get_asyncG] Unknow SRM protocol, failure ");
	}
	free(full_endpoint);
	return ret;	
}


/**
 * convert a table a struct srmv2_pinfilestatus to (a) desired GList(s) of turl string, turl associated error code, turl associated error string
 *  @return return the number of turl in the last request
 *  @param turls pointer GList<char*> for the GList turl, need to be free. A empty turl (error case) cause an addition of a NULL pointer to the LIST. turls must be set to NULL if not used.
 *  @param turls_code pointer GList<int> for the GList error code associated to the turls, need to be free. turls_errcode must be set to NULL if not used
 *  @param turls_errstring GList<char*> pointer for the GList error string associated to the turls, need to free . A empty turl (error case) cause an addition of a NULL pointer to the LIST. turls_errstring must be set to NULL if not used.
 *  @param err : global GError system
 * */
static int gfal_convert_filestatut(gfal_handle handle, GList** turls, GList** turls_code, GList** turls_errstring, GError** err){
	g_return_val_err_if_fail( handle && handle->last_request_state, -1, err, "[gfal_convert_filestatut] invalid arguments");
	gfal_request_state* last_req = handle->last_request_state;
	GError* tmp_err=NULL;
	if(gfal_async_request_is_finishedG(handle,&tmp_err) == FALSE){
		if(tmp_err){
			g_propagate_prefixed_error(err, tmp_err, "[gfal_convert_filestatut]");
			return -1;
		}
		g_set_error(err,0, EBUSY, "[gfal_convert_filestatut] request GET is not terminated"); 
		return -2;
	}
	// part srmv2 only : support for v1 suppressed
	struct srmv2_pinfilestatus *filestatus	= last_req->srmv2_pinstatuses;
	const int ret = last_req->number;
	int i;
	GList* tmp_turls=NULL, *tmp_turls_code=NULL, *tmp_turls_errstring=NULL;
	for(i=0; i< ret; ++i){
		const int code = filestatus[i].status;
		if(turls_code){
			tmp_turls_code = g_list_append(tmp_turls_code, GINT_TO_POINTER(code));
		}
		if(turls){
			const char* turl = filestatus[i].turl;
			const char* resu = (!code && turl)?strndup(turl,2048):NULL;
			tmp_turls = g_list_append(tmp_turls, (gpointer)resu);
		}
		if(turls_errstring){
			const char* err_string = filestatus[i].explanation;
			const char* resu = (code && err_string)?strndup(err_string,2048):NULL;
			tmp_turls_errstring = g_list_append(tmp_turls_errstring,(gpointer) resu);
		}
	}
	if(turls)
		*turls = tmp_turls;
	if(turls_code)
		*turls_code = tmp_turls_code;
	if(turls_errstring)
		*turls_errstring = tmp_turls_errstring;
	return ret;
}


/**
 *  convert a srm_ifce result to the handle, update the last request info
 */
static int gfal_convert_to_handle(int ret, gfal_request_state* request_info, struct srmv2_pinfilestatus *filestatus, GError** err){
	g_return_val_err_if_fail(ret && request_info && filestatus, -1, err, "[gfal_convert_to_handle] invalid args");
	gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(request_info->srmv2_pinstatuses, request_info->number);
	request_info->srmv2_pinstatuses = filestatus;
	int i;
	gboolean finished = TRUE;
	for(i=0; i < ret; ++i){
		const int code = filestatus[i].status;		// get the current request status
		if(code == EINVAL) 		// request in progress : must wait
			finished = FALSE; 
		//g_printerr(" err code : %s", strerror(code)); 
	}
	request_info->finished = finished;
	request_info->number = ret;
	return ret;
}

/**
 * obtain the statut in the case of a srmv2 protocol request
 */
static int gfal_get_request_statusG(gfal_handle handle, GError** err){

	const int max_buffer_size = 2048;
	GError* tmp_err=NULL;
	char err_buff[max_buffer_size];
	int ret=-1;
	
	gfal_request_state* request_info = handle->last_request_state;

	if( request_info->current_request_proto == PROTO_SRMv2){ // srm v2 request executed

		struct srm_context context;
		struct srm_preparetoget_input preparetoget_input;
		struct srm_preparetoget_output preparetoget_output;
		
		gfal_srm_external_call.srm_context_init(&context, request_info->request_endpoint, err_buff,max_buffer_size, gfal_get_verbose());
		
		preparetoget_output.token = request_info->srmv2_token;			// set the token of the last request
		int ret = srm_status_of_get_request_async(&context,&preparetoget_input,&preparetoget_output);
		if( ret <0){
			g_set_error(err,0, EINVAL, "[gfal_get_request_statusG_srmv2] srm_ifce answered with the error : %d adn err : %s", ret, err_buff);
		} else{
			if ( (ret = gfal_convert_to_handle(ret, request_info, preparetoget_output.filestatuses, &tmp_err)) <0){
				g_propagate_prefixed_error(err, tmp_err, "[gfal_get_request_statusG_srmv2]");
			}
			gfal_srm_external_call.srm_srm2__TReturnStatus_delete(preparetoget_output.retstatus);
		} 
		
	}else if( request_info->current_request_proto == PROTO_SRM){
		g_set_error(err,0, EINVAL, "[gfal_get_request_statusG] The SRMv1 protocol is not supported anymore ");
		return -3;		
	} else{
		g_set_error(err, 0, EPROTONOSUPPORT, "[gfal_get_request_statusG] the protocol of the current request is not supported ");
		ret = -2;
	}
	return ret;
}

/**
 * @brief progress of the last request
 * @return return TRUE if the current request is finished else FALSE
 * @param err : Gerror** err system
 */
gboolean gfal_async_request_is_finishedG(gfal_handle handle, GError** err)
{
	g_return_val_err_if_fail(handle , -1, err, "[gfal_get_request_statusG] arg invalid value, handle invalid");	
	GError * tmp_err=NULL;
	gboolean ret = FALSE;
	gfal_request_state* last_request = handle->last_request_state;
	if(last_request == NULL){
		g_set_error(&tmp_err,0, EINVAL, " gfal_get_asyncG must be executed before");
		return -2;
	}
	if(last_request->finished){
		return TRUE;
	}else{
		gfal_get_request_statusG(handle, &tmp_err);
		ret = last_request->finished;
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}

/**
 * @brief get the result to the last get_async request
 * @return return the number of response in turls or negative value if error
 * @param handle : handle of the current context
 * @param GList** turls : GList<char*> turls with the full list of answer, an answer with error is a NULL pointer
 * @warning turls need to be free manually 
 */
int gfal_get_async_resultsG(gfal_handle handle, GList** turls, GError** err){
	g_return_val_err_if_fail(handle && turls , -1, err, "[gfal_get_request_statusG] arg invalid value");
	int ret = -1;
	GError * tmp_err=NULL;

	ret = gfal_convert_filestatut(handle, turls, NULL,NULL,&tmp_err);
	if( ret <0)
		g_propagate_prefixed_error(err, tmp_err, "[gfal_get_request_statusG]");
	return ret;
}

  /**
  * @brief get the error string of each request 
  * @param handle
  * @param turl_errcode : GList<char*>, give a string error for each turl request, char* can be NULL if no error associated
  * @return return number of request turl if success else return negative value
  * */
int gfal_get_async_results_errstringG(gfal_handle handle, GList** turls_errstring, GError** err){	
	g_return_val_err_if_fail(handle && turls_errstring , -1, err, "[gfal_get_async_results_errstringG] arg invalid value");
	int ret = -1;
	GError * tmp_err=NULL;
	ret = gfal_convert_filestatut(handle,NULL,NULL, turls_errstring, &tmp_err);
	if( ret <0)
		g_propagate_prefixed_error(err, tmp_err, "[gfal_get_async_results_errstringG]");
	return ret;
 }


/**
 * convert the results in a struct gfal_srm_result struct
 * 
 */
void gfal_srm_to_srm_result(char * turl, int err_code, char* err_string, gfal_srm_result* result){
	memset(result,0, sizeof(struct _gfal_srm_result));
	if(turl)
		g_strlcpy(result->turl,turl, GFAL_URL_MAX_LEN);
	result->err_code = err_code;
	if(err_string)
		g_strlcpy(result->err_str, err_string, GFAL_ERRMSG_LEN);
}
 
/**
 * @brief get
 * @param handle
 * @param tab_struct : struct gfal_srm_result*, give a table of a struct gfal_srm_result
 * @return return number of results in table if success else return negative value
 * @warning the *tab_struct need to be free() 
 * */
int gfal_get_async_results_structG(gfal_handle handle, gfal_srm_result** tab_struct, GError** err){
	g_return_val_err_if_fail(handle && tab_struct , -1, err, "[gfal_get_async_results_errstringG] arg invalid value");	
	int ret = -1,i;
	GError * tmp_err=NULL;
	GList* resu=NULL, *err_code=NULL, *err_string=NULL;
	ret = gfal_convert_filestatut(handle, &resu, &err_code, &err_string, &tmp_err);
	if(ret >0 && !tmp_err){
		*tab_struct = malloc(sizeof(struct _gfal_srm_result)*ret);
		GList* tmp_resu = resu, *tmp_err_code= err_code, *tmp_err_string = err_string;
		for(i=0; i<ret; ++i, tmp_resu = g_list_next(tmp_resu), tmp_err_code = g_list_next(tmp_err_code), tmp_err_string = g_list_next(tmp_err_string))
			gfal_srm_to_srm_result(tmp_resu->data, GPOINTER_TO_INT(tmp_err_code->data), tmp_err_string->data, (*tab_struct)+i);
			
		g_list_free_full(resu,free);
		g_list_free_full(err_string,free);
		g_list_free(err_code);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[gfal_get_async_results_errstringG]");
	return ret;	
}  
  
/**
 * @brief wait for the current request
 * @param handle
 * @param timeout : maximum time to wait before error
 * @param err : Error report system
 *  @return return 0 if finished correctly, return -1 if timeout is reached or if error ( err is set )
 */
 int gfal_wait_async_requestG(gfal_handle handle, long timeout, GError** err){
	 time_t timeo = time(NULL) + timeout;
	 GError* tmp_err=NULL;
	 long uswait = 500;
	 while(timeo >time(NULL)){
		 if(  gfal_async_request_is_finishedG(handle, &tmp_err)  == TRUE)
			return 0;
		 if(tmp_err){
			g_propagate_prefixed_error(err,tmp_err,"[%s]",__func__);
			return -1;
		}
		usleep(uswait<<=1);
	 }
	 g_set_error(err, 0, ETIME, "[%s] timeout expired : %d ", __func__, timeout);
	 return -1;
 }
 
 
 /**
 *  @brief errcode of each turls request
 *  @param handle : handle associated with the request
 *  @param turl_errcode :  GList<int> of all the errcode associated with the request
 *  @param err : Gerror** err system
 *  @return return number of request turl if success else return negative value
 **/
 int gfal_get_async_results_errcodesG(gfal_handle handle, GList** turls_errcode, GError** err){
	g_return_val_err_if_fail(handle && turls_errcode , -1, err, "[gfal_get_async_results_errcodesG] arg invalid value");
	int ret = -1;
	GError * tmp_err=NULL;
	ret = gfal_convert_filestatut(handle,NULL, turls_errcode, NULL, &tmp_err);
	if( ret <0)
		g_propagate_prefixed_error(err, tmp_err, "[gfal_get_async_results_errcodesG]");
	return ret;	  
 }
 

