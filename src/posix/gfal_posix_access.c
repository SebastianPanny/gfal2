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
 * @file gfal_posix_access.c
 * @brief file for the internal access func of the posix interface
 * @author Devresse Adrien
 * @version 2.0
 * @date 06/05/2011
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


int gfal_posix_internal_access (const char *path, int amode){
	int resu = -1, ret;
	GError* tmp_err=NULL;
	gfal_handle handle;
	if(path == NULL){
		errno = EFAULT;
		return -1;
	}
	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	const int check = gfal_surl_checker(path, NULL);
	if(check==0){
		resu = gfal_srm_accessG(handle, (char*) path, amode, &tmp_err );	// srm:// -> send to the srm part
	}else if(gfal_guid_checker(path, NULL)){
		resu = gfal_guid_accessG(handle, (char*) path, amode, &tmp_err);	// guid: -> send to the first catalog
	}else if(gfal_check_local_url(path, NULL)){
		resu = gfal_local_access(path, amode, &tmp_err);									// file:// -> send to the local system call
	}else {
		resu = gfal_catalogs_accessG(handle, (char*) path, amode, &tmp_err );		// if registered url ( lfn:// ) resolve,
	}
	
	if(resu){ // error reported
		gfal_posix_register_internal_error(handle, "[gfal_access]", tmp_err);
		errno = tmp_err->code;			
	}
	return (resu)?(-1):0;
}
