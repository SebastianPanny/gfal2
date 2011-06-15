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
 * @file gfal_common_srm_opendir.c
 * @brief file for the opendir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 09/06/2011
 * */


#include <dirent.h>
#include <stdio.h>
#include "gfal_common_srm.h"
#include "gfal_common_srm_opendir.h"
#include "../gfal_common_errverbose.h"
#include "gfal_common_srm_internal_layer.h"





DIR* gfal_srmv2_opendir_internal(gfal_handle handle, char* endpoint, const char* surl, GError** err){
	g_return_val_err_if_fail(handle && endpoint && surl, NULL, err, "[gfal_srmv2_opendir_internal] invaldi args");
	struct srm_context context;
	struct srm_ls_input input;
	struct srm_ls_output output;
	struct srmv2_mdfilestatus *srmv2_mdstatuses=NULL;
	char errbuf[GFAL_ERRMSG_LEN];
	int i;
	DIR* resu=NULL;
	int ret =-1;
	char* tab_surl[] = { (char*)surl, NULL};
	const int nb_request=1;
	int tab_resu[nb_request];
	
	gfal_srm_external_call.srm_context_init(&context, endpoint, errbuf, GFAL_ERRMSG_LEN, gfal_get_verbose());	// init context
	
	input.nbfiles = nb_request;
	input.surls = tab_surl;
	input.numlevels = 1;
	input.offset = 0;
	input.count = 0;

	ret = gfal_srm_external_call.srm_ls(&context,&input,&output);					// execute ls

	if(ret >=0){
		srmv2_mdstatuses = output.statuses;
		if(srmv2_mdstatuses->status != 0){
			g_set_error(err, 0, srmv2_mdstatuses->status, "[%s] Error reported from srm_ifce : %d %s", __func__, 
							srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
			resu = NULL;
			gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(output.statuses,1);
		} else {
			gfal_srm_opendir_handle oh = calloc(sizeof(struct _gfal_srm_opendir_handle),1);
			oh->srm_ls_resu = srmv2_mdstatuses;
			resu = (DIR*) oh;
		}
	}else{
		g_set_error(err,0, ECOMM, "[%s] Bad answer from srm_ifce, maybe voms-proxy is not initiated properly", __func__);
		resu=NULL;
	}

	gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
	return resu;	
}
	


DIR* gfal_srm_opendirG(catalog_handle ch, const char* surl, GError ** err){
	g_return_val_err_if_fail(ch && surl, NULL, err, "[gfal_srm_opendirG] Invalid args");
	gfal_handle handle = ch;
	DIR* resu = NULL;
	char* endpoint=NULL;
	GError* tmp_err=NULL;
	int ret = -1;
	enum gfal_srm_proto srm_type;
	
	ret = gfal_auto_get_srm_endpoint_for_surl(handle, &endpoint, &srm_type, surl, &tmp_err);
	if( ret >=0 ){
		if(srm_type == PROTO_SRMv2){
			resu = gfal_srmv2_opendir_internal(handle, endpoint, surl, &tmp_err);
		}else if (srm_type == PROTO_SRM){
			g_set_error(err, 0, EPROTONOSUPPORT, "[%s] support for SRMv1 is removed in 2.0, failure");
			resu = NULL;
		}else {
			g_set_error(err, 0, EPROTONOSUPPORT, "[%s] Unknow version of the protocol SRM , failure");
			resu = NULL;			
		}
		
	}
	
	free(endpoint);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return resu;
}


int gfal_srm_closedirG(catalog_handle handle, DIR* fh, GError** err){
	g_return_val_err_if_fail(handle && fh, -1, err, "[gfal_srm_opendirG] Invalid args");
	gfal_srm_opendir_handle oh = (gfal_srm_opendir_handle) fh;	
	gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(oh->srm_ls_resu, 1);
	free(oh);
	return 0;
}
