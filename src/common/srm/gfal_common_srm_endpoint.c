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
 * @file gfal_common_srm_endpoint.c
 * @brief file for internal use in the srm part, endpoint determination
 * @author Devresse Adrien
 * @version 2.0
 * @date 22/06/2011
 * */
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <stdlib.h>
#include <errno.h>
#include "../gfal_common_internal.h"
#include "gfal_common_srm_endpoint.h"
#include "../gfal_common_errverbose.h"

/**
 * @brief extract endpoint and srm_type from a surl
 *  determine the best endpoint associated with the list of url and the params of the actual handle (no bdii check or not)
 *  see the diagram in doc/diagrams/surls_get_endpoint_activity_diagram.svg for more informations
 *  @return return 0 with endpoint and types set if success else -1 and set Error
 * */
int gfal_srm_determine_endpoint(gfal_handle handle, const char* surl, char* buff_endpoint, size_t s_buff, enum gfal_srm_proto* srm_type, GError** err){
	g_return_val_err_if_fail(handle && buff_endpoint && srm_type && surl && s_buff,-1, err, "[gfal_srm_determine_endpoint] invalid value in params"); // check params
	
	GError* tmp_err=NULL;
	int ret = -1;
	char * tmp_endpoint=NULL;;
	gboolean isFullEndpoint = gfal_check_fullendpoint_in_surlG(surl, &tmp_err);		// check if a full endpoint exist
	if(!tmp_err){
			if( isFullEndpoint == TRUE  ){ // if full endpoint contained in url, get it and set type to default type
				/*if( (tmp_endpoint = gfal_get_fullendpoint(surl,&tmp_err) ) != NULL){
					*endpoint = tmp_endpoint;
					*srm_type= handle->srm_proto_type;
					return 0;
				}*/

			}
			if(handle->no_bdii_check == FALSE){
				/*ret = gfal_get_endpoint_and_setype_from_bdii(handle, endpoint, srm_type, surls, &tmp_err)  */
			}else
				g_set_error(&tmp_err,0,EINVAL," no_bdii_check option need a full endpoint in the first surl");				
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret;
}


/**
 *  return TRUE if a full endpoint is contained in surl  else FALSE
 * 
*/
gboolean gfal_check_fullendpoint_in_surlG(const char * surl, GError ** err){
	regex_t rex;
	int ret = regcomp(&rex, "^srm://([:alnum:]|-|/|\.|_)+:[0-9]+/([:alnum:]|-|/|\.|_)+?SFN=",REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret==0,-1,err,"[gfal_check_fullendpoint_in_surl] fail to compile regex, report this bug");
	ret=  regexec(&rex,surl,0,NULL,0);
	regfree(&rex);
	return (ret==0)?TRUE:FALSE;	
}


/**
 *  @brief create a full endpoint from a "full-surl
 * */
int gfal_get_fullendpointG(const char* surl, char* buff_endpoint, size_t s_buff, GError** err){
	char* p = strstr(surl,"?SFN=");
	const int len_prefix = strlen(GFAL_PREFIX_SRM);						// get the srm prefix length
	const int len_endpoint_prefix = strlen(GFAL_ENDPOINT_DEFAULT_PREFIX); // get the endpoint protocol prefix len 
	g_return_val_err_if_fail(p && len_prefix && (p>(surl+len_prefix)) && len_endpoint_prefix, -1,err,"[gfal_get_fullendpoint] full surl must contain ?SFN= and a valid prefix, fatal error");	// assertion on params

	size_t need_size = p- surl-len_prefix +len_endpoint_prefix;
	if(s_buff > need_size){
		strncpy(buff_endpoint, GFAL_ENDPOINT_DEFAULT_PREFIX, len_endpoint_prefix);	// copy prefix
		strncpy(buff_endpoint + len_endpoint_prefix, surl+len_prefix, p- surl-len_prefix);		// copy endpoint
		return 0;
	}
	g_set_error(err, 0, ENOBUFS, "[%s] buffer too small", __func__);	
	return -1;
}
