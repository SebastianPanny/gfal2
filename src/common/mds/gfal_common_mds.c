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
  * @author Devresse Adrien
  * @version 2.0.0
  * @date 18/04/2011
  * */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>
#include "../gfal_prototypes.h"
#include "../gfal_types.h"
#include "../gfal_common_internal.h"
#include "gfal_common_mds_ldap_internal.h"

pthread_mutex_t m_mds =PTHREAD_MUTEX_INITIALIZER; 

/**
 * set the bdii value of the handle specified
 */
void gfal_set_nobdiiG(gfal_handle handle, gboolean no_bdii_chk){
	handle->no_bdii_check = no_bdii_chk;
}

gboolean gfal_get_nobdiiG(gfal_handle handle){
	return handle->no_bdii_check;
}


/**
 * return the srm endpoints and their types, in the old way
 * */
int gfal_mds_get_se_types_and_endpoints (const char *host, char ***se_types, char ***se_endpoints, GError** err){
	GError* tmp_err=NULL;
	gfal_mds_endpoint tabend[GFAL_MDS_MAX_SRM_ENDPOINT];
	
	
	int n = gfal_mds_resolve_srm_endpoint(host, tabend, GFAL_MDS_MAX_SRM_ENDPOINT, &tmp_err);
	if( n > 0){
		int i;
		*se_types = calloc(n+1, sizeof(char*));
		*se_endpoints = calloc(n+1, sizeof(char*));	
		for(i=0; i< n; ++i){
			(*se_endpoints)[i] = strdup(tabend[i].url);
			(*se_types)[i] = strdup(((tabend[i].type == SRMv2)?"srm_v2":"srm_v1"));
		}
	}
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return (n >0)?0:-1;	
}

/**
 *  try to get lfc hostname from bdii
 *  @return string if success or NULL & set the err if fail
 * 
 */
 char * gfal_get_lfchost_bdii(gfal_handle handle, GError** err){
		size_t s_errbuff = GFAL_ERRMSG_LEN;
		char errbuff[s_errbuff];
		memset(errbuff, '\0', sizeof(char)*s_errbuff);
		g_set_error(err, 0, EPROTONOSUPPORT, "[%s] disable in gfal 2.0, api broken in is interface",__func__);
		return NULL;
 } 
 
 
 int gfal_mds_resolve_srm_endpoint(const char* base_url, gfal_mds_endpoint* endpoints, size_t s_endpoint, GError** err){
		return gfal_mds_bdii_get_srm_endpoint(base_url, endpoints, s_endpoint, err);
 }


