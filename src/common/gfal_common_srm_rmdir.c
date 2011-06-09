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
 * @file gfal_common_srm_rmdir.c
 * @brief file for the rmdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 24/05/2011
 * */

#include "gfal_common_srm_rmdir.h"
#include "gfal_constants.h"
#include "gfal_common_errverbose.h"
#include "gfal_common_srm_internal_layer.h"


int gfal_srmv2_rmdir_internal(gfal_handle handle, char* endpoint, const char* surl, GError** err){
	struct srm_context context;
	struct srm_rmdir_input rmdir_input;
	struct srm_rmdir_output rmdir_output;
	GError* tmp_err=NULL;
	int ret = -1;
	
	srm_context_init(&context, endpoint, NULL, 0, gfal_get_verbose());

	rmdir_input.recursive = 0;
	rmdir_input.surl = (char*)surl;

	if( (ret = gfal_srm_external_call.srm_rmdir(&context, &rmdir_input, &rmdir_output)) >=0){
		const int sav_errno = rmdir_output.statuses[0].status;
		if( sav_errno ){
			g_set_error(&tmp_err, 0, sav_errno, " Error report from the srm_ifce %s ", strerror(sav_errno));
			ret = -1;
		}else{
			ret =0;
		}
		gfal_srmv2_statuses_delete(rmdir_output.statuses,1);
	}else{
		g_set_error(&tmp_err, 0, ECOMM, " COMM error reported with the srm_ifce %s",  strerror(errno));
		ret=-1;		
	}
	
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}


int gfal_srm_rmdirG(gfal_handle handle, const char* surl, GError** err){
	int ret = -1;
	char* endpoint=NULL;
	GError* tmp_err=NULL;
	enum gfal_srm_proto srm_type;
	
	ret = gfal_auto_get_srm_endpoint_for_surl(handle, &endpoint, &srm_type, surl, &tmp_err);
	if( ret >=0 ){
		if(srm_type == PROTO_SRMv2){
			ret = gfal_srmv2_rmdir_internal(handle, endpoint, surl, &tmp_err);
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




