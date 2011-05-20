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
 * @file gfal_common_srm_access.c
 * @brief file for the access function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 05/05/2011
 * */

#include "gfal_common_srm_access.h"
#include "gfal_constants.h"
#include "gfal_common_errverbose.h"
#include <gfal_srm_ifce_types.h> 


static int gfal_access_srmv2_internal(gfal_handle handle, char* endpoint, char* surl, int mode,  GError** err){
	struct srm_context context;
	struct srm_checkpermission_input checkpermission_input;
	struct srmv2_filestatus *resu;
	const int nb_request=1;
	char errbuf[GFAL_ERRMSG_LEN];
	int i;
	int ret=-1;
	char* tab_surl[] = { surl, NULL};
	int tab_resu[nb_request];


	srm_context_init(&context, endpoint, errbuf, GFAL_ERRMSG_LEN, gfal_get_verbose());

	checkpermission_input.nbfiles = nb_request;
	checkpermission_input.amode = mode;
	checkpermission_input.surls = tab_surl;

	ret = srm_check_permission(&context,&checkpermission_input, &resu);	
	if(ret != nb_request){
		g_set_error(err, 0,ECOMM, "[gfal_access_srmv2_internal] Bad answer of the libgfal_srm_ifce, Maybe voms-proxy is not set properly : %d", ret);
		return ECOMM;
	}
	for(i=0; i< nb_request; ++i){
		if( resu[i].status ){
			if( strnlen(resu[i].surl, GFAL_URL_MAX_LEN) >= GFAL_URL_MAX_LEN || strnlen(resu[i].explanation, GFAL_URL_MAX_LEN) >= GFAL_URL_MAX_LEN){
				g_set_error(err, 0, resu[i].status, "[gfal_access_srmv2_internal] Memory corruption in the libgfal_srm_ifce answer, fatal");			
			}else{
				g_set_error(err, 0, resu[i].status, "[gfal_access_srmv2_internal] Error %d : %s  \
, file %s: %s", resu[i].status, strerror(resu[i].status), resu[i].surl, resu[i].explanation);
			}
			ret= -1;
			break;
		}
		ret = resu[i].status;
	}
	//g_printerr(" resu : %d , status %d, strerror : %s, explanation : %s \n", ret, resu[0].status, strerror(resu[0].status), resu[0].explanation);
	free(resu);
	return ret;
}


/**
 * @brief access method for SRMv2
 * check the right for a given SRM url, work only for SRMv2, V1 deprecated.
 * @param handle
 * @param surl srm url of a given file
 * @param mode, access mode to check
 * @param err : GError error reprot system
 * @warning : not safe, surl must be verified
 */ 
int gfal_srm_accessG(gfal_handle handle, char* surl, int mode, GError** err){			// execute an access method on a srm url
	g_return_val_err_if_fail(handle && surl, EINVAL, err, "[gfal_srm_accessG] Invalid value handle and/or surl");
	GError* tmp_err=NULL;
	int ret=-1;
	char* full_endpoint=NULL;
	enum gfal_srm_proto srm_types;
	ret =gfal_auto_get_srm_endpoint_for_surl(handle, &full_endpoint, &srm_types, surl,  &tmp_err); // get the associated endpoint
	if( ret != 0){		// check & get endpoint										
		g_propagate_prefixed_error(err,tmp_err, "[%s]", __func__);
		return -1;
	}
	
	if (srm_types == PROTO_SRMv2){			// check the proto version
		ret= gfal_access_srmv2_internal(handle, full_endpoint, surl, mode,&tmp_err);	// execute the SRMv2 access test
		if(tmp_err)
			g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	} else if(srm_types == PROTO_SRM){
			g_set_error(err,0, EPROTONOSUPPORT, "[%s] support for SRMv1 is removed in 2.0, failure", __func__);
			ret =  -1;
	} else{
		g_set_error(err,0,EPROTONOSUPPORT, "[%s] Unknow version of the protocol SRM , failure ", __func__);
		ret=-1;
	}
	free(full_endpoint);
	return ret;
}
