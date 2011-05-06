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
#include <glib.h>
#include "gfal_posix_internal.h"
#include "gfal_posix_local_file.h"
#include "../common/gfal_types.h"
#include  "../common/gfal_common_catalog.h"
#include "../common/gfal_common.h"
#include "../common/gfal_common_srm_access.h"
#include "../common/gfal_constants.h"


int gfal_access_posix_internal(const char *path, int amode){
	int resu = -1;
	gfal_handle handle;
	if(path == NULL)
		return EFAULT;
	if((handle = gfal_posix_instance()) == NULL){
		return -1;
	}
	const int check = gfal_surl_checker(path, NULL);
	if(check==0){
		resu = gfal_srm_accessG(handle, (char*) path, amode, gfal_get_last_gerror(handle) );	// srm:// -> send to the srm part
	}else if(gfal_guid_checker(path, NULL)){
		resu = gfal_catalogs_guid_accessG(handle, (char*) path, amode, gfal_get_last_gerror(handle));	// guid: -> send to the first catalog
	}else if(gfal_check_local_url(path, NULL)){
		resu = gfal_local_access(path, amode);									// file:// -> send to the local system call
	}else {
		resu = gfal_catalogs_accessG(handle, (char*) path, amode, gfal_get_last_gerror(handle) );		// if registered url ( lfn:// ) resolve, else return EPROTONOSUPPORT
	}
	return resu;
}
