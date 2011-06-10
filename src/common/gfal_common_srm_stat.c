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
 * @file gfal_common_srm_stat.c
 * @brief file for the stat function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 16/05/2011
 * */

#include "gfal_common_srm_access.h"
#include "gfal_constants.h"
#include "gfal_common_errverbose.h"
#include "gfal_common_srm_internal_layer.h" 



static int gfal_statG_srmv2_internal(gfal_handle handle, struct stat* buf, const char* endpoint, const char* surl, GError** err){
	g_return_val_err_if_fail( handle && endpoint && surl 
								 && buf && (sizeof(struct stat) == sizeof(struct stat64)),
								-1, err, "[gfal_statG_srmv2_internal] Invalid args handle/endpoint or invalid stat sturct size");
	struct srm_context context;
	struct srm_ls_input input;
	struct srm_ls_output output;
	struct srmv2_mdfilestatus *srmv2_mdstatuses=NULL;
	char * srmv2_token;
	const int nb_request=1;
	char errbuf[GFAL_ERRMSG_LEN];
	int i;
	int ret=-1;
	char* tab_surl[] = { (char*)surl, NULL};
	int tab_resu[nb_request];
	
	gfal_srm_external_call.srm_context_init(&context, endpoint, errbuf, GFAL_ERRMSG_LEN, gfal_get_verbose());	// init context
	
	input.nbfiles = nb_request;
	input.surls = tab_surl;
	input.numlevels = 0;
	input.offset = 0;
	input.count = 0;

	ret = gfal_srm_external_call.srm_ls(&context,&input,&output);					// execute ls

	if(ret >=0){
		srmv2_mdstatuses = output.statuses;
		if(srmv2_mdstatuses->status != 0){
			g_set_error(err, 0, srmv2_mdstatuses->status, "[%s] Error reported from srm_ifce : %d %s", __func__, 
							srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
			ret = -1;
		} else {
			memcpy(buf, &(srmv2_mdstatuses->stat), sizeof(struct stat));
			ret = 0;
		}
	}else{
		g_set_error(err,0, ECOMM, "[%s] Bad answer from srm_ifce, maybe voms-proxy is not initiated properly", __func__);
		ret=-1;
	}
	gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(srmv2_mdstatuses, 1);
	gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
	return ret;	
}

int gfal_srm_statG(gfal_handle handle, const char* surl, struct stat* buf, GError** err){
	g_return_val_err_if_fail( handle && surl && buf, -1, err, "[gfal_srm_statG] Invalid args in handle/surl/bugg");
	GError* tmp_err = NULL;
	int ret =-1;
	char* endpoint=NULL;
	enum gfal_srm_proto srm_type;
	
	ret = gfal_auto_get_srm_endpoint_for_surl(handle, &endpoint, &srm_type, surl, &tmp_err);
	if( ret >=0 ){
		if(srm_type == PROTO_SRMv2){
			ret = gfal_statG_srmv2_internal(handle, buf, endpoint, surl, &tmp_err);
		}else if (srm_type == PROTO_SRM){
			g_set_error(err, 0, EPROTONOSUPPORT, "[%s] support for SRMv1 is removed in 2.0, failure");
			ret = -1;
		}else {
			g_set_error(err, 0, EPROTONOSUPPORT, "[%s] Unknow version of the protocol SRM , failure");
			ret = -1;			
		}
		
	}
	free(endpoint);
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}
