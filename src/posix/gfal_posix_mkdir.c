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
 * @file gfal_posix_mkdir.c
 * @brief file for the internal mkdir func of the posix interface
 * @author Devresse Adrien
 * @version 2.0
 * @date 18/05/2011
 * */

#include <stdio.h>
#include <errno.h>
#include "gfal_posix_api.h"
#include <glib.h>
#include "../common/gfal_types.h"
#include "gfal_posix_internal.h"
#include "gfal_posix_local_file.h"
#include  "../common/gfal_common_catalog.h"
#include "../common/gfal_common_guid.h"
#include "../common/gfal_common.h"
#include "../common/gfal_common_srm_access.h"
#include "../common/gfal_constants.h"


/**
 * Implementation of the mkdir function
 * 
 * */
 int gfal_posix_internal_mkdir(const char* path, mode_t mode){
	 GError* tmp_err=NULL;
	 gfal_handle handle;
	 int res= -1;

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	
	if(path == NULL){
		g_set_error(&tmp_err, 0, EFAULT, " path is an incorrect argument");
	}else{
		if( gfal_check_local_url(path, NULL) == TRUE){
			res = gfal_local_mkdir(path, mode, &tmp_err);
		}else if(gfal_guid_checker(path, NULL) == TRUE){
			res = -1;
			g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "Protocol guid is not supported for directory creation");
		}else if( gfal_surl_checker(path, NULL) == 0 ){
			res = gfal_srm_mkdirG(handle, path, mode, &tmp_err);	
		}else{
			res = gfal_catalog_mkdirp(handle, path, mode, TRUE, &tmp_err);
		}
	}
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_mkdir]", tmp_err);
		errno = tmp_err->code;	
	}
	return res; 
	 
 }
 
