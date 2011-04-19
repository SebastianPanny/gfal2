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

#include "gfal_common.h"



int gfal_mds_get_se_types_and_endpoints (const char *host, char ***se_types, char ***se_endpoints, GError** err){
	
	const int ret = sd_get_se_types_and_endpoints(host, se_types, se_endpoints);
	if(ret)
		if(errno == ECOMM){
			g_set_error(err,0,errno,"[gfal_mds_get_se_types_and_endpoints] ServiceDiscovery system return a COMM error, maybe the LCG_GFAL_INFOSYS env var is not set properly ");			
		}else if(errno == EINVAL){
			g_set_error(err,0,errno,"[gfal_mds_get_se_types_and_endpoints] ServiceDiscovery system return a EINVAL error, unable to get endpoint from this host, ( maybe host doesn't exist anymore ? ) ");			
		}else{
			g_set_error(err,0,errno,"[gfal_mds_get_se_types_and_endpoints] ServiceDiscovery system return an error ");
		}
	return ret;	
}


