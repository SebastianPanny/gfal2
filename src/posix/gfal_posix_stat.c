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
#include "../common/gfal_prototypes.h"
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
	if(!path || !buff){
		errno = EFAULT;
		return -1;
	}
	
	if( (handle = gfal_initG(&tmp_err)) == NULL){
		errno = EIO;
		return -1;
	}
	
	if( gfal_check_local_url(path, NULL) ){
		ret= gfal_local_stat(path, buf, &tmp_err));
	} else if( gfal_guid_checker(path, NULL) ){
		g_error(" epic fail, not implemented");
	} else if( gfal_surl_checker(path, NULL){
		g_error(" epic fail, not implemented");
	} else {
		gfal_catalog_statG(handle, path, buf, &tmp_err);
	}

	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}
