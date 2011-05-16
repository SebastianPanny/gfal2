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
 * @file gfal_posix_stat.c
 * @brief file for the internal function of the POSIX stat/stat64 functions
 * @author Devresse Adrien
 * @version 2.0
 * @date 13/05/2011
 * 
 **/
 
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <glib.h>
#include "gfal_posix_internal.h"
#include "../common/gfal_types.h"
#include "../common/gfal_common_guid.h"
#include "../common/gfal_common_srm.h"


 /**
  *  internal implementation of gfal_access
  * */
int gfal_posix_internal_stat(const char* path, struct stat* buf){
	gfal_handle handle;
	GError* tmp_err = NULL;
	int ret = -1;
	if(!path || !buf){
		errno = EFAULT;
		return -1;
	}
	
	if( (handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	
	if( gfal_check_local_url(path, NULL) ){
		ret = gfal_local_stat(path, buf, &tmp_err);
	} else if( gfal_guid_checker(path, NULL) ){
		ret = gfal_guid_statG(handle, path, buf, &tmp_err);
	} else if( gfal_surl_checker(path, NULL) == 0){
		g_set_error(&tmp_err, 0, ENOSYS, "[%s] not implementated", __func__);
		ret =-1;
	} else {
		ret = gfal_catalog_statG(handle, path, buf, &tmp_err);
	}
	
	if(ret){ // error reported
		gfal_posix_register_internal_error(handle, "[gfal_stat]", tmp_err);
		errno = tmp_err->code;			
	}
	return ret;
}
