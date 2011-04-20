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
 
#include "gfal_common.h"
#include "gfal_common_srm.h"

/**
 * list of the protols in the order of preference
 */
static enum gfal_srm_proto gfal_proto_list_pref[]= { PROTO_SRMv2, PROTO_SRM, PROTO_ERROR_UNKNOW };

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
 * set the bdii value of the handle specified
 */
void gfal_set_nobdiiG(gfal_handle handle, gboolean no_bdii_chk){
	handle->no_bdii_check = no_bdii_chk;
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
	return ret;	
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
		return -2;
	}
	if( gfal_select_best_protocol_and_endpoint(handle, endpoint, srm_type, tab_se_type, tab_endpoint, &tmp_err) != 0){ // map the response if correct
		g_propagate_prefixed_error(err, tmp_err, "[gfal_get_endpoint_and_setype_from_bdii]");
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
 * @brief get endpoint
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
	return handle;
}

/**
 * @brief accessor for the default storage type definition
 * */
void gfal_set_default_storageG(gfal_handle handle, enum gfal_srm_proto proto){
	handle->srm_proto_type = proto;
}

/**
 * @biref convert glist to surl char* to char**
 *  @return return NULL if error or pointer to char**
 */
char** gfal_GList_to_tab(GList* surls){
	int surl_size = g_list_length(surls);
	int i;
	char **resu = surl_size?((char**)calloc(surl_size+1, sizeof(char*))):NULL;
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
	g_return_val_err_if_fail(surl != NULL,-1,err,"[gfal_surl_checker_] check URL failed : surl is empty");
	regex_t rex;
	int ret = regcomp(&rex, "^srm://([:alnum:]|-|/|\.|_)+$",REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret==0,-1,err,"[gfal_surl_checker_] fail to compile regex, report this bug");
	ret=  regexec(&rex,surl,0,NULL,0);
	if(ret) 
		g_set_error(err,0,EINVAL,"[gfal_surl_checker] Incorrect surl, impossible to parse surl %s :", surl);
	return ret;
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
	srm_context_init(&context, endpoint, errbuf, size, gfal_get_verbose());	
	
	
	ret = srm_prepare_to_get_async(&context,&preparetoget_input,&preparetoget_output);
	if(ret < 0){
		g_set_error(err,0,errno,"[gfal_srmv2_getasync] call to srm_ifce error: %s",errbuf);
	} else{
		if(handle->last_request_state){
			gfal_print_verbose(GFAL_VERBOSE_NORMAL, "[GFAL] [gfal_srmv2_getasync] last request params deleted not properly ! ");
			free(handle->last_request_state);		// free if notthe first one
		}

		handle->last_request_state = calloc(1, sizeof(struct _gfal_request_state));			// copy the informations of the current request in the last request information structure
		gfal_request_state * req_state = handle->last_request_state;
    	req_state->srmv2_token = preparetoget_output.token;
    	req_state->srmv2_pinstatuses = preparetoget_output.filestatuses;	
    	req_state->current_request_proto = PROTO_SRMv2;	
    	req_state->request_endpoint = strndup(endpoint, 2048);
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
			g_set_error(err,0, EPROTONOSUPPORT, "[gfal_get_asyncG] Tentative d'utilisation de SRMv1 déprécié, Seul SRMv2 est autorisé : echec");
			ret =  -1;
	} else{
		ret=-1;
		g_set_error(err,0,EPROTONOSUPPORT, "[gfal_get_asyncG] Type de protocole spécifié non supporté ( Supportés :  SRMv2 ) ");
	}
	return ret;	
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
/**
 * convert a table a struct srmv2_pinfilestatus in a GList<char*>* turls and GList<GErro*> * gerr_turls
 * 
 * */
static int gfal_convert_filestatut(int ret, GList** turls, GList** gerr_turls, struct srmv2_pinfilestatus *filestatus, GError** err){
			int i;
			GList* tmp_turls = NULL;
			GList* tmp_gerr_turls=NULL;
			for(i=0; i< ret; ++i){
				const int surl_err_code = filestatus[i].status; // get the status of the current turl
				GError* surl_err=NULL;
				char* turl = NULL;
				if(  surl_err_code != 0){  // error for this turl
					if(gerr_turls){
						const char * explanation = (filestatus[i].explanation)?(strndup(filestatus[i].explanation,2048)):"No explanation given by srm_ifce";
						g_set_error(&surl_err,0, surl_err_code, explanation); 
					}
				} else{
					const char* turl_tmp = filestatus[i].turl;
					if( turl_tmp == NULL || strnlen(turl_tmp,2048) == 2048){
						g_set_error(err,0, EINVAL, "[gfal_get_request_statusG_srmv2] turl value from  srm_ifce corrupted"); // fatal error leave
						ret = -2;
						return ret;
					}
				    turl = strdup(turl_tmp);	
				}				 
				tmp_turls =  g_list_append(tmp_turls, turl);
				tmp_gerr_turls = g_list_append(tmp_gerr_turls,  surl_err);
			}
	if(gerr_turls)
		*gerr_turls = tmp_gerr_turls;
	*turls = tmp_turls;
	return ret;	
}
/**
 * obtain the statut in the case of a srmv2 protocol request
 */
static int gfal_get_request_statusG_srmv2(gfal_handle handle, GList** turls, GList** gerr_turls, GError** err){
	    struct srm_context context;
    	struct srm_preparetoget_input preparetoget_input;
    	struct srm_preparetoget_output preparetoget_output;
    	const int max_buffer_size = 2048;
    	GError* tmp_err=NULL;
    	char err_buff[max_buffer_size];
    	
		gfal_request_state* request_info = handle->last_request_state;
    	srm_context_init(&context, request_info->request_endpoint, err_buff,max_buffer_size, gfal_get_verbose());

        preparetoget_output.token = request_info->srmv2_token;			// set the token of the last request
        int ret = srm_status_of_get_request_async(&context,&preparetoget_input,&preparetoget_output);
        request_info->srmv2_pinstatuses = preparetoget_output.filestatuses;
        if( ret <0){
			g_set_error(err,0, EINVAL, "[gfal_get_request_statusG_srmv2] srm_ifce answered with the error : %d adn err : %s", ret, err_buff);
		} else{
			if ( (ret = gfal_convert_filestatut(ret, turls, gerr_turls, request_info->srmv2_pinstatuses, &tmp_err)) <0){
				g_propagate_prefixed_error(err, tmp_err, "[gfal_get_request_statusG_srmv2]");
			}
		} 
		free(request_info);
		handle->last_request_state = NULL;
		return ret;
}

/**
 * get the state of the current async request
 * @return return the number of response in turls or negativevalue if error
 * @param handle : handle of the current context
 * @param GList** turls : GList of char* with the full list of answer, answer can be 0 if error
 * @param Optional GList**gerr_turls : GList of GError*, contain the error report for each url ( see GError report system ), A turl element with no error is associated with NULL pointer for safety reason
 *  @warning turls need to be free manually after use
 */
int gfal_get_async_resultsG(gfal_handle handle, GList** turls, GList** gerr_turls,  GError** err){
	g_return_val_err_if_fail(handle && turls , -1, err, "[gfal_get_request_statusG] arg invalid value");
	gfal_request_state* last_request = handle->last_request_state;
	if(last_request == NULL){
		g_set_error(err,0, EINVAL, "[gfal_get_request_statusG] gfal_get_asyncG executed before");
		return -2;
	}
	int ret = -1;
	GError * tmp_err=NULL;
	if( last_request->current_request_proto == PROTO_SRMv2){ // srm v2 request executed
		ret = gfal_get_request_statusG_srmv2(handle, turls, gerr_turls, &tmp_err);
		if( ret <0)
			g_propagate_prefixed_error(err, tmp_err, "[gfal_get_request_statusG]");
	}else if( last_request->current_request_proto == PROTO_SRM){
		g_set_error(err,0, EINVAL, "[gfal_get_request_statusG] The SRMv1 protocol is not supported anymore ");
		return -3;		
	} else{
		g_set_error(err, 0, EPROTONOSUPPORT, "[gfal_get_request_statusG] the protocol of the current request is not supported ");
		ret = -2;
	}
	return ret;
	
}
