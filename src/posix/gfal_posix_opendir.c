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
 * @file gfal_posix_opendir.c
 * @brief file for the opendir posix func
 * @author Devresse Adrien
 * @version 2.0
 * @date 25/05/2011
 * */


#include "../common/gfal_constants.h" 
#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"
#include "gfal_common_filedescriptor.h"



DIR* gfal_posix_internal_opendir(const char* name){
	GError* tmp_err=NULL;

	gfal_handle handle;
	DIR* res= NULL;

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return NULL;
	}

	if(name == NULL){
		g_set_error(&tmp_err, 0, EFAULT, " name is empty");
	}else{
		if( gfal_check_local_url(name, NULL) == TRUE){
			res = NULL;
			g_set_error(&tmp_err, 0, ENOSYS, "not implemented");	
		}else if(gfal_guid_checker(name, NULL) == TRUE){
			res = NULL;
			g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "Protocol guid is not supported for directory creation");
		}else if( gfal_surl_checker(name, NULL) == 0 ){
			res = NULL;
			g_set_error(&tmp_err, 0, ENOSYS, "not implemented");	
		}else{
			res = NULL;
			g_set_error(&tmp_err, 0, ENOSYS, "not implemented");	
		}
	}
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_opendir]", tmp_err);
		errno = tmp_err->code;	
	}
	return res; 
	 
	
}

