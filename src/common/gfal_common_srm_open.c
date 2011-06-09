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
 * @file gfal_common_srm_open.c
 * @brief file for the open function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 06/06/2011
 * */
 
 #define _GNU_SOURCE

#include <gfal_srm_ifce_types.h> 
#include <string.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "gfal_common_srm.h"
#include "gfal_common_srm_open.h"
#include "gfal_constants.h"
#include "gfal_common_errverbose.h"

static gfal_file_handle gfal_srm_results_to_filehandle(gfal_srm_result* results, GError** err){
	g_return_val_err_if_fail(results && err, NULL, err, "[gfal_turl_to_filehandle] invalid args");
	if(results->err_code ==0)	
		return gfal_file_handle_new(GFAL_MODULEID_SRM, strndup(results->turl, GFAL_URL_MAX_LEN));
	g_set_error(err, 0, results->err_code, "[%s] error from srmifce : %s", __func__, results->err_str);
	return NULL;
}

static gfal_file_handle gfal_open_read_internal(gfal_handle handle,  
									const char *path, int flag, mode_t mode, GError** err){
	gfal_file_handle fh = NULL;
	GError* tmp_err=NULL;
	GList* l = g_list_append(NULL, (char*)path);

	int ret = gfal_get_asyncG(handle, l, &tmp_err);
	if(ret >=0){
		gfal_srm_result* results=NULL;
		gfal_wait_async_requestG(handle, 50, &tmp_err);
		if(!tmp_err){
			ret = gfal_get_async_results_structG(handle, &results, &tmp_err);
			if(ret >=0)
				fh = gfal_srm_results_to_filehandle(results, &tmp_err);
			free(results);
		}
	}
	g_list_free(l);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return fh;
}

static gfal_file_handle gfal_open_create_internal(gfal_handle handle, 
									const char *path, int flag, mode_t mode, GError** err){
				
	g_set_error(err, 0, ENOSYS, "Not implemented");						
	return NULL;									
}





gfal_file_handle gfal_srm_openG(gfal_handle handle, const char* path, int flag, mode_t mode, GError** err){
	g_return_val_err_if_fail( path && handle , NULL, err, "[gfal_srm_openG] Invalid args");
	gfal_file_handle fh = NULL;
	GError * tmp_err=NULL;

	if( (flag & O_ACCMODE) ==0)
		fh = gfal_open_read_internal(handle, path, flag, mode, &tmp_err);
	else
		fh = gfal_open_create_internal(handle, path, flag, mode, &tmp_err);
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return fh;
}


int gfal_srm_closeG(gfal_file_handle fh, GError** err){
	free(fh->fdesc);	
	return 0;
}
