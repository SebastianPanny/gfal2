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
 * @file gfal_posix_open.c
 * @brief file for the internal open function for the posix interface
 * @author Devresse Adrien
 * @version 2.0
 * @date 31/05/2011
 * */

#include <glib.h>
#include <stdlib.h>
#include "../common/gfal_types.h"
#include "gfal_posix_internal.h"
#include "../common/gfal_constants.h"
#include "../common/gfal_common_errverbose.h"
#include "../common/gfal_common_file_handle.h"
#include "../common/gfal_common_catalog.h"
#include "../common/gfal_common_srm_open.h"
#include "gfal_posix_local_file.h"



/**
 *  store a gfal_file_handle in the base, in a key/value model
 *  @return the key, else 0 if error occured and err is set correctly
 * */
static int gfal_posix_file_handle_store(gfal_handle handle, gfal_file_handle fhandle, GError** err){
	g_return_val_err_if_fail( handle && fhandle, -1, err, "[gfal_posix_file_handle_store] invalid args");
	GError* tmp_err=NULL;
	int key=0;
	gfal_fdesc_container_handle container= gfal_file_handle_container_instance(&(handle->fdescs), &tmp_err);
	if(container)
		key = gfal_add_new_file_desc(container, (gpointer) fhandle, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return key;
}

/**
 * Try to resolve the file to a surl link, if fail try to open directly on the catalog layer
 *  @return pointer to file handle if success else error
 * 
 */ 
static gfal_file_handle gfal_posix_catalog_open(gfal_handle handle, const char * path, int flag, mode_t mode, GError** err){
	char** res_surl=NULL;
	GError* tmp_err=NULL;
	gfal_file_handle ret = NULL;
	if( (res_surl = gfal_catalog_getSURL(handle, path, &tmp_err)) != NULL){ // try a surl resolution on the catalogs
		if(gfal_surl_checker(path, NULL) == 0 )
			ret = gfal_srm_openG(handle, res_url, flag, mode, &tmp_err);
		else
			g_set_error(&tmp_err, 0, ECOMM, "bad surl value retrived from catalog : %s ", res_surl);
		g_strfreev(res_surl);
	}else if(!tmp_err){ // try std open on the catalogs		
		ret = gfal_catalog_openG(handle, path, flag, mode, &tmp_err);
	}

	if(tmp_err)
		gfal_posix_register_internal_error(handle, "[gfal_open]", tmp_err);	
	return ret;
}



/**
 * 
 *  Implementation of gfal_open
 * 
 * */
int gfal_posix_internal_open(const char* path, int flag, mode_t mode){
	GError* tmp_err=NULL;

	gfal_handle handle;
	gfal_file_handle fhandle=NULL;
	int ret= -1;
	int key = -1;

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return -1;
	}
	if(path == NULL){
		g_set_error(&tmp_err, 0, EFAULT, " name is empty");
	}else{
		if( gfal_check_local_url(path, NULL) == TRUE){
			fhandle = gfal_local_open(path, flag, mode, &tmp_err);
		}else if(gfal_guid_checker(path, NULL) == TRUE){
			fhandle = NULL;
			g_set_error(&tmp_err, 0, ENOSYS, "not implemented");
		}else if( gfal_surl_checker(path, NULL) == 0 ){
			fhandle = gfal_srm_openG(handle, path, flag, mode, &tmp_err);
		}else{
			fhandle = gfal_posix_catalog_open(handle, path, flag, mode, &tmp_err);
		}
	}


	if(fhandle)
		key = gfal_posix_file_handle_store(handle, fhandle, &tmp_err);

	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_open]", tmp_err);
		errno = tmp_err->code;	
	}else{
		errno=0;
	}
	return key; 	
}
