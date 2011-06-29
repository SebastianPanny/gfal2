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
  * @brief  file for the bdii request part of gfal
  * @author : Devresse Adrien
  * @version 2.0.0
  * @date 18/04/2011
  * */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include "gfal_common_mds_layer.h"
#include "../gfal_common.h"
#include "../gfal_common_internal.h"
#include "../voms/gfal_voms_if.h"

/**
 * set the bdii value of the handle specified
 */
void gfal_set_nobdiiG(gfal_handle handle, gboolean no_bdii_chk){
	handle->no_bdii_check = no_bdii_chk;
}

gboolean gfal_get_nobdiiG(gfal_handle handle){
	return handle->no_bdii_check;
}



int gfal_mds_get_se_types_and_endpoints (const char *host, char ***se_types, char ***se_endpoints, GError** err){
	
	const int ret = gfal_mds_external_call.sd_get_se_types_and_endpoints(host, se_types, se_endpoints, NULL,0);
	if(ret)
		if(errno == ECOMM){
			g_set_error(err,0,errno,"[gfal_mds_get_se_types_and_endpoints] ServiceDiscovery system return a COMM error, maybe the LCG_GFAL_INFOSYS env var is not set properly ");			
		}else if(errno == EINVAL){
			g_set_error(err,0,errno,"[gfal_mds_get_se_types_and_endpoints] ServiceDiscovery system : EINVAL error, unable to get endpoint for this host : %s ( maybe the host doesn't exist anymore ? ) ", host);			
		}else{
			g_set_error(err,0,errno,"[gfal_mds_get_se_types_and_endpoints] ServiceDiscovery system return an error ( maybe voms-proxy is no initiated properly ? )");
		}
	return ret;	
}

/**
 *  try to get lfc hostname from bdii
 *  @return string if success or NULL & set the err if fail
 * 
 */
 char * gfal_get_lfchost_bdii(gfal_handle handle, GError** err){
		char* lfc_host = NULL;
		GError* tmp_err = NULL;
		char* vo = gfal_get_voG(&tmp_err);		// get vo and fqans from voms module

		if( gfal_get_nobdiiG(handle) ){		// check the bdii
			g_set_error(err, 0, EPROTONOSUPPORT, "[gfal_setup_lfchost] no_bdii_set : you must define the LFC_HOST env var correctly");
			free(vo);
			return NULL;
		}
		if(!vo || tmp_err){
			g_propagate_prefixed_error(err, tmp_err, "[gfal_get_lfchost_bdii]");
			free(vo);
			return NULL;
		}
		GList* fqan = gfal_get_fqanG(&tmp_err);
		if(!fqan || tmp_err){
			g_propagate_prefixed_error(err, tmp_err, "[gfal_get_lfchost_bdii]");
			free(vo);	
			return NULL;		
		}
		char** fqantab= gfal_GList_to_tab(fqan);
		gfal_mds_external_call.set_gfal_vo(vo);
		gfal_mds_external_call.set_gfal_fqan(fqantab, g_list_length(fqan));
		//g_printerr(" vo : %s, fqann %d, fqan : %s \n", vo, g_list_length(fqan), *fqantab); 
		const int ret =  gfal_mds_external_call.sd_get_lfc_endpoint(&lfc_host, NULL, 0);
		if(!lfc_host || ret <0){
			g_set_error(err, 0, errno, "[gfal_get_lfchost_bdii] Error while get lfc endpoint from bdii system : %d & %s ", ret, strerror(errno) );
			lfc_host = NULL;
		}
		free(fqantab);
		free(vo);				  
		return lfc_host;
 } 


