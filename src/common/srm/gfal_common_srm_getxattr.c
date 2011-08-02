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
 * @file gfal_common_srm_getxattr.c
 * @brief file for the getxattr function on the srm url type
 * @author Devresse Adrien
 * @date 02/08/2011
 * */
 
 
#define _GNU_SOURCE
 
#include <string.h>

#include "gfal_common_srm.h"
#include "../gfal_constants.h"
#include "../gfal_common_errverbose.h"
#include "gfal_common_srm_internal_layer.h" 
#include "gfal_common_srm_getxattr.h"

static const char* srm_geturl_key = SRM_XATTR_GETURL;

static char* srm_listxattr[]= { SRM_XATTR_GETURL, NULL };


ssize_t gfal_srm_geturl_getxattrG(catalog_handle handle, const char* path, const char* name , void* buff, size_t s_buff, GError** err){
	GError* tmp_err=NULL;
	ssize_t ret = -1;
	if(s_buff ==0 || buff == NULL)
		return GFAL_URL_MAX_LEN;

	ret = gfal_srm_getTURLS_catalog(handle, path, buff, s_buff, NULL, &tmp_err);
	if(ret >= 0){
		ret = strlen(buff)* sizeof(char);
	}

	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;		
}

/**
 * implementation of the getxattr for turl resolution, pin management and spacetoken set/get
 * 
 * */
ssize_t gfal_srm_getxattrG(catalog_handle handle, const char* path, const char* name , void* buff, size_t s_buff, GError** err){
	GError* tmp_err=NULL;
	ssize_t ret = -1;
	if(strcmp(name, srm_geturl_key) == 0){
		ret = gfal_srm_geturl_getxattrG(handle, path, name, buff, s_buff, &tmp_err);
	}else{ // need to add pin and spacetoken
		g_set_error(&tmp_err, 0, ENOATTR, "no an existing extended attribute");
	}
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}



/**
 * lfc getxattr implem 
 * */
ssize_t gfal_srm_listxattrG(catalog_handle handle, const char* path, char* list, size_t size, GError** err){
	GError* tmp_err=NULL;
	ssize_t res = 0;	
	char** p= srm_listxattr;
	char* plist= list;
	while(*p != NULL){
		const int size_str = strlen(*p)+1;
		if( size > res && size - res >= size_str)
			plist = mempcpy(plist, *p, size_str* sizeof(char) );
		res += size_str;
		p++;
	}
	return res;
}
