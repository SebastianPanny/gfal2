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
 * @file gfal_common_srm_mkdir.c
 * @brief file for the mkdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 19/05/2011
 * */

#include "gfal_common_srm_mkdir.h"
#include "gfal_constants.h"
#include "gfal_common_errverbose.h"
#include "gfal_common_srm_internal_layer.h"


int gfal_mkdir_srmv2_internal(gfal_handle handle, char* endpoint, const char* path, mode_t mode, GError** err){
	struct srm_mkdir_input mkdir_input;
	struct srm_context context;	
	int res = -1;

	errno =0;	
    gfal_srm_external_call.srm_context_init(&context, endpoint, NULL, 0, gfal_get_verbose());	  
    mkdir_input.dir_name = (char*) path;
   	res  = gfal_srm_external_call.srm_mkdir(&context, &mkdir_input);

   	if(res <0){
		g_set_error(err,0, errno, "[%s] Error reported by srm_ifce ", __func__);
		res = -1;
	}
   //	g_printerr(" filename %s endpoint %s res %d mode %o \n", path, endpoint, res, mode);
	return res;
	
}


int gfal_srm_mkdirG(gfal_handle handle, char* surl, int mode, GError** err){
	int ret = -1;
	char* full_endpoint=NULL;
	GError* tmp_err=NULL;
	enum gfal_srm_proto srm_types;
	ret =gfal_auto_get_srm_endpoint_for_surl(handle, &full_endpoint, &srm_types, surl,  &tmp_err);
	
	if(ret >=0){
		if (srm_types == PROTO_SRMv2){			// check the proto version
			ret= gfal_mkdir_srmv2_internal(handle, full_endpoint, surl, mode, &tmp_err);	// execute the SRMv2 access test
	
		} else if(srm_types == PROTO_SRM){
				g_set_error(&tmp_err,0, EPROTONOSUPPORT, "support for SRMv1 is removed in 2.0, failure");
				ret =  -1;
		} else{
			g_set_error(&tmp_err,0,EPROTONOSUPPORT, "Unknow version of the protocol SRM , failure ");
			ret=-1;
		}

	}
	free(full_endpoint);
	if(tmp_err){		// check & get endpoint										
		g_propagate_prefixed_error(err,tmp_err, "[%s]", __func__);
		ret = -1;
	}	
	return ret;
}
