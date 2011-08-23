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
#include <pthread.h>
#include "../gfal_prototypes.h"
#include "../gfal_types.h"
#include "gfal_common_mds_layer.h"
#include "../gfal_common_internal.h"
#include "../voms/gfal_voms_if.h"

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



int gfal_mds_get_se_types_and_endpoints (const char *host, char ***se_types, char ***se_endpoints, GError** err){
	pthread_mutex_lock(&m_mds);
	char err_buff[GFAL_ERRMSG_LEN];
	memset(err_buff,'\0',GFAL_ERRMSG_LEN*sizeof(char));
	const int ret = gfal_mds_external_call.sd_get_se_types_and_endpoints(host, se_types, se_endpoints, err_buff, GFAL_ERRMSG_LEN);
	if(ret)
		if(errno == ECOMM){
			g_set_error(err,0,errno,"[gfal_mds_get_se_types_and_endpoints] ServiceDiscovery system return a COMM error, maybe the LCG_GFAL_INFOSYS env var is not set properly :%s ", err_buff);			
		}else if(errno == EINVAL){
			g_set_error(err,0,errno,"[gfal_mds_get_se_types_and_endpoints] ServiceDiscovery system : EINVAL error, unable to get endpoint for this host : %s ( maybe the host doesn't exist anymore ? ) :%s", host, err_buff);			
		}else{
			g_set_error(err,0,errno,"[gfal_mds_get_se_types_and_endpoints] ServiceDiscovery system return an error ( maybe voms-proxy is no initiated properly ? ) :%s", err_buff);
		}
	pthread_mutex_unlock(&m_mds);
	return ret;	
}

/**
 *  try to get lfc hostname from bdii
 *  @return string if success or NULL & set the err if fail
 * 
 */
 char * gfal_get_lfchost_bdii(gfal_handle handle, GError** err){
	 	pthread_mutex_lock(&m_mds);
		char* lfc_host = NULL;
		GError* tmp_err = NULL;
		size_t s_errbuff = GFAL_ERRMSG_LEN;
		char errbuff[s_errbuff];
		memset(errbuff, '\0', sizeof(char)*s_errbuff);

		const int ret =  gfal_mds_external_call.sd_get_lfc_endpoint(&lfc_host, errbuff, s_errbuff);
		if(!lfc_host || ret <0){
			g_set_error(err, 0, errno, "[gfal_get_lfchost_bdii] Error while get lfc endpoint from bdii system : %d & %s, %s", ret, strerror(errno), errbuff );
			lfc_host = NULL;
		}
		pthread_mutex_unlock(&m_mds);			  
		return lfc_host;
 } 


