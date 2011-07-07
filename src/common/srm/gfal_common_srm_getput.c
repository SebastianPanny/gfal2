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
 * @brief file with the get/getasync srm funcs 
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/06/2011
 * */
 
 #define _GNU_SOURCE 

#include <regex.h>
#include <time.h> 
 
#include "gfal_common_srm.h"
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_catalog.h"
#include "gfal_common_srm_internal_layer.h"

typedef enum _srm_req_type{
	SRM_GET,
	SRM_PUT
} srm_req_type;

static gboolean gfal_srm_surl_group_checker(char** surls, GError** err){
	GError* tmp_err=NULL;
	if(surls == NULL ){
		g_set_error(err, 0, EINVAL, "[%s] Invalid argument surls ", __func__);
		return FALSE;
	}
	while(*surls != NULL){
		if( gfal_surl_checker(*surls, &tmp_err) != 0){
			g_propagate_prefixed_error(err,tmp_err,"[%s]",__func__);	
			return FALSE;
		}
		surls++;
	}
	return TRUE;
}


static int gfal_srm_convert_filestatuses_to_srm_result(struct srmv2_pinfilestatus* statuses, char* reqtoken, int n, gfal_srm_result** resu, GError** err){
	g_return_val_err_if_fail(statuses && n && resu, -1, err, "[gfal_srm_convert_filestatuses_to_srm_result] args invalids");
	*resu = calloc(n, sizeof(gfal_srm_result));
	int i=0;
	for(i=0; i< n; ++i){
		if(statuses[i].turl)
			g_strlcpy((*resu)[i].turl, statuses[i].turl, GFAL_URL_MAX_LEN);
		if(statuses[i].explanation)
			g_strlcpy((*resu)[i].err_str, statuses[i].explanation, GFAL_URL_MAX_LEN);
		(*resu)[i].err_code = statuses[i].status;	
		(*resu)[i].reqtoken = reqtoken;
	}
	return 0;
}

static int gfal_srm_convert_filestatuses_to_GError(struct srmv2_filestatus* statuses, int n, GError** err){
	g_return_val_err_if_fail(statuses && n, -1, err, "[gfal_srm_convert_filestatuses_to_GError] args invalids");	
	int i;
	int ret =0;
	for(i=0; i< n; ++i){
		if(statuses[i].status != 0){
			g_set_error(err, 0, statuses[i].status, "[%s] Error on the surl %s while putdone : %s", __func__,
				statuses[i].surl, statuses[i].explanation);
			ret = -1;			
		}
	}
	return ret;
}

/**
 *  @brief execute a srmv2 request sync "GET" on the srm_ifce
*/
static int gfal_srm_getTURLS_srmv2_internal(gfal_handle handle, char* endpoint, char** surls, gfal_srm_result** resu,  GError** err){
	g_return_val_err_if_fail(surls!=NULL,-1,err,"[gfal_srmv2_getasync] GList passed null");
			
	GError* tmp_err=NULL;
	char** p;
	struct srm_context context;
	int ret=0,i=0;
	struct srm_preparetoget_input preparetoget_input;
	struct srm_preparetoget_output preparetoget_output;
	const int err_size = 2048;
	
	char errbuf[err_size] ; memset(errbuf,0,err_size*sizeof(char));
	const gfal_srmv2_opt* opts = handle->srmv2_opt;							// get default opts for srmv2
	size_t n_surl = g_strv_length (surls);									// n of surls
		
	// set the structures datafields	
	preparetoget_input.desiredpintime = opts->opt_srmv2_desiredpintime;		
	preparetoget_input.nbfiles = n_surl;
	preparetoget_input.protocols = opts->opt_srmv2_protocols;
	preparetoget_input.spacetokendesc = opts->opt_srmv2_spacetokendesc;
	preparetoget_input.surls = surls;	
	gfal_srm_external_call.srm_context_init(&context, endpoint, errbuf, err_size, gfal_get_verbose());	
	
	
	ret = gfal_srm_external_call.srm_prepare_to_get(&context,&preparetoget_input,&preparetoget_output);
	if(ret < 0){
		g_set_error(&tmp_err,0,errno,"call to srm_ifce error: %s",errbuf);
	} else{
		gfal_srm_convert_filestatuses_to_srm_result(preparetoget_output.filestatuses, preparetoget_output.token, ret, resu,  &tmp_err);
    	gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(preparetoget_output.filestatuses, ret);
    	gfal_srm_external_call.srm_srm2__TReturnStatus_delete(preparetoget_output.retstatus);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}


/**
 *  @brief execute a srmv2 request sync "PUT" on the srm_ifce
*/
static int gfal_srm_putTURLS_srmv2_internal(gfal_handle handle, char* endpoint, char** surls, gfal_srm_result** resu,  GError** err){
	g_return_val_err_if_fail(surls!=NULL,-1,err,"[gfal_srm_putTURLS_srmv2_internal] GList passed null");
			
	GError* tmp_err=NULL;
	char** p;
	struct srm_context context;
	int ret=0,i=0;
	struct srm_preparetoput_input preparetoput_input;
	struct srm_preparetoput_output preparetoput_output;
	const int err_size = 2048;

	
	char errbuf[err_size] ; memset(errbuf,0,err_size*sizeof(char));
	const gfal_srmv2_opt* opts = handle->srmv2_opt;							// get default opts for srmv2
	size_t n_surl = g_strv_length (surls);									// n of surls
	gint64 filesize_tab[n_surl];
	for(i=0; i < n_surl;++i)
		filesize_tab[i] = opts->filesizes;
		
	// set the structures datafields	
	preparetoput_input.desiredpintime = opts->opt_srmv2_desiredpintime;		
	preparetoput_input.nbfiles = n_surl;
	preparetoput_input.protocols = opts->opt_srmv2_protocols;
	preparetoput_input.spacetokendesc = opts->opt_srmv2_spacetokendesc;
	preparetoput_input.surls = surls;	
	preparetoput_input.filesizes = filesize_tab;
	gfal_srm_external_call.srm_context_init(&context, endpoint, errbuf, err_size, gfal_get_verbose());	
	
	
	ret = gfal_srm_external_call.srm_prepare_to_put(&context,&preparetoput_input,&preparetoput_output);
	if(ret < 0){
		g_set_error(&tmp_err,0,errno,"call to srm_ifce error: %s",errbuf);
	} else{
		gfal_srm_convert_filestatuses_to_srm_result(preparetoput_output.filestatuses, preparetoput_output.token, ret, resu, &tmp_err);
    	gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(preparetoput_output.filestatuses, ret);
    	gfal_srm_external_call.srm_srm2__TReturnStatus_delete(preparetoput_output.retstatus);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}

/***
 * Internal function of gfal_srm_getTurls without argument check for internal usage
 * 
 * */
int gfal_srm_mTURLS_internal(gfal_handle handle, srm_req_type req_type, char** surls, gfal_srm_result** resu,   GError** err){
	GError* tmp_err=NULL;
	int ret=-1;	

	char full_endpoint[2048];
	enum gfal_srm_proto srm_types;
	if((gfal_srm_determine_endpoint(handle, *surls, full_endpoint, GFAL_URL_MAX_LEN, &srm_types, &tmp_err)) == 0){		// check & get endpoint										
		gfal_print_verbose(GFAL_VERBOSE_NORMAL, "[gfal_get_asyncG] endpoint %s", full_endpoint);

		if (srm_types == PROTO_SRMv2){
			if(req_type == SRM_GET)
				ret= gfal_srm_getTURLS_srmv2_internal(handle, full_endpoint, surls, resu,  &tmp_err);
			else
				ret= gfal_srm_putTURLS_srmv2_internal(handle, full_endpoint, surls, resu,  &tmp_err);
		} else if(srm_types == PROTO_SRM){
			g_set_error(&tmp_err,0, EPROTONOSUPPORT, "support for SRMv1 is removed in gfal 2.0, failure");
		} else{
			g_set_error(&tmp_err,0,EPROTONOSUPPORT, "[gfal_get_asyncG] Unknow SRM protocol, failure ");
		}		
	}

	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;		
}

/**
 *  simple wrapper to getTURLs for the gfal_module layer
 * */
int gfal_srm_getTURLS_catalog(catalog_handle ch, const char* surl, char* buff_turl, int size_turl, char** reqtoken,  GError** err){
	gfal_handle handle = (gfal_handle)ch;
	gfal_srm_result* resu=NULL;
	GError* tmp_err=NULL;
	char* surls[]= { (char*)surl, NULL };
	int ret = -1;
	ret= gfal_srm_mTURLS_internal(handle, SRM_GET, surls, &resu, &tmp_err);
	if(ret >=0){
		if(resu[0].err_code == 0){
			g_strlcpy(buff_turl, resu[0].turl, size_turl);			
			if(reqtoken)
				*reqtoken = resu[0].reqtoken;
			ret=0;			
		}else{
			g_set_error(&tmp_err,0 , resu[0].err_code, " error on the turl request : %s ", resu[0].err_str);
			ret = -1;
		}
	
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;			
}

/**
 * @brief launch a surls-> turls translation in the synchronous mode
 * @warning need a initiaed gfal_handle
 * @param handle : the gfal_handle initiated ( \ref gfal_init )
 * @param surls : tab of string, last char* must be NULL
 * @param err : GError** for error report
 * @return return positive if success else -1, check GError for more information
 */
int gfal_srm_getTURLS(gfal_handle handle, char** surls, gfal_srm_result** resu,  GError** err){
	g_return_val_err_if_fail(handle!=NULL,-1,err,"[gfal_get_asyncG] handle passed is null");
	
	GError* tmp_err=NULL;
	int ret=-1;
	if( gfal_handle_checkG(handle, &tmp_err) ){	// check handle validity
		if( gfal_srm_surl_group_checker	(surls, &tmp_err) == TRUE){				
			ret = gfal_srm_mTURLS_internal(handle, SRM_GET, surls, resu,  &tmp_err);
		}
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}


/**
 *  simple wrapper to putTURLs for the gfal_module layer
 * */
int gfal_srm_putTURLS_catalog(catalog_handle ch, const char* surl, char* buff_turl, int size_turl, char** reqtoken, GError** err){
	gfal_handle handle = (gfal_handle)ch;
	gfal_srm_result* resu=NULL;
	GError* tmp_err=NULL;
	char* surls[]= { (char*)surl, NULL };
	int ret = -1;
	ret= gfal_srm_mTURLS_internal(handle, SRM_PUT, surls, &resu,  &tmp_err);
	if(ret >=0){
		if(resu[0].err_code == 0){
			g_strlcpy(buff_turl, resu[0].turl, size_turl);
			if(reqtoken)
				*reqtoken = resu[0].reqtoken;
			ret=0;			
		}else{
			g_set_error(&tmp_err,0 , resu[0].err_code, " error on the turl request : %s ", resu[0].err_str);
			ret = -1;
		}
	
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;			
}

/**
 * @brief launch a surls-> turls translation in the synchronous mode for file creation
 * @warning need a initiaed gfal_handle
 * @param handle : the gfal_handle initiated ( \ref gfal_init )
 * @param surls : tab of string, last char* must be NULL
 * @param err : GError** for error report
 * @return return positive if success else -1, check GError for more information
 */
int gfal_srm_putTURLS(gfal_handle handle, char** surls, gfal_srm_result** resu,  GError** err){
	g_return_val_err_if_fail(handle!=NULL,-1,err,"[gfal_srm_putTURLS] handle passed is null");
	
	GError* tmp_err=NULL;
	int ret=-1;
	if( gfal_handle_checkG(handle, &tmp_err) ){	// check handle validity
		if( gfal_srm_surl_group_checker	(surls, &tmp_err) == TRUE){				
			ret = gfal_srm_mTURLS_internal(handle, SRM_PUT, surls, resu, &tmp_err);
		}
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}


/**
 *  execute a srm put done on the specified surl and token, return 0 if success else -1 and errno is set
*/
static int gfal_srm_putdone_srmv2_internal(gfal_handle handle, char* endpoint, char** surls, char* token,  GError** err){
	g_return_val_err_if_fail(surls!=NULL,-1,err,"[gfal_srmv2_getasync] GList passed null");
			
	GError* tmp_err=NULL;
	char** p;
	struct srm_context context;
	int ret=0,i=0;
	struct srm_putdone_input putdone_input;
	struct srmv2_filestatus *statuses;
	const int err_size = 2048;
	gfal_srm_result* resu=NULL;
	
	char errbuf[err_size] ; memset(errbuf,0,err_size*sizeof(char));
	const gfal_srmv2_opt* opts = handle->srmv2_opt;							// get default opts for srmv2
	size_t n_surl = g_strv_length (surls);									// n of surls
		
	// set the structures datafields	
	putdone_input.nbfiles = n_surl;
	putdone_input.reqtoken = token;
	putdone_input.surls = surls;

	gfal_srm_external_call.srm_context_init(&context, endpoint, errbuf, err_size, gfal_get_verbose());	
	
	
	ret = gfal_srm_external_call.srm_put_done(&context,&putdone_input, &statuses);
	if(ret < 0){
		g_set_error(&tmp_err,0,errno,"call to srm_ifce error: %s",errbuf);
	} else{
    	gfal_srm_external_call.srm_srmv2_filestatus_delete(statuses, n_surl);
    	return gfal_srm_convert_filestatuses_to_GError(statuses, ret, &tmp_err);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}





