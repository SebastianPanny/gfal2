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
 * @file gfal_common.c
 * @brief file for the open/read/write srm
 * @author Devresse Adrien
 * @date 06/07/2011
 * */
 
 #define _GNU_SOURCE 

#include <regex.h>
#include <time.h> 
#include <fcntl.h> 
#include "gfal_common_srm.h"
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_catalog.h"
#include "../gfal_common_filedescriptor.h"
#include "gfal_common_srm.h"


typedef struct _gfal_srm_handle_open{
	gfal_file_handle internal_handle;
	char surl[GFAL_URL_MAX_LEN];
	
}*gfal_srm_handle_open;

static gfal_file_handle gfal_srm_file_handle_create(gfal_file_handle fh, char* surl){
	if(fh== NULL)
		return NULL;
	gfal_srm_handle_open sh = g_new(struct _gfal_srm_handle_open,1);
	sh->internal_handle = fh;
	g_strlcpy(sh->surl, surl, GFAL_URL_MAX_LEN);
	return gfal_file_handle_new(gfal_srm_getName(), sh);
}

static gfal_file_handle gfal_srm_file_handle_map(gfal_file_handle fh){
	return ((gfal_srm_handle_open) fh->fdesc)->internal_handle;
}

static gfal_srm_file_handle_delete(gfal_file_handle fh){
	free(fh->fdesc);
	free(fh);
}

/**
 * open function for the srm  plugin
 */
gfal_file_handle gfal_srm_openG(catalog_handle ch, const char* path, int flag, mode_t mode, GError** err){
	gfal_file_handle ret = NULL;
	gfal_handle handle = (gfal_handle)	ch;
	GError* tmp_err=NULL;
	char* p = (char*)path;
	char turl[GFAL_URL_MAX_LEN];
	g_clear_error(&tmp_err);
	int tmp_ret;
	if(flag & O_CREAT) // create turl if file is not existing else get one for this file
		tmp_ret= gfal_srm_putTURLS_catalog(ch, p, turl, GFAL_URL_MAX_LEN, &tmp_err);
	else
		tmp_ret= gfal_srm_getTURLS_catalog(ch, p, turl, GFAL_URL_MAX_LEN, &tmp_err);
	if(tmp_ret == 0){
		ret = gfal_catalog_openG(handle, turl, flag, mode, &tmp_err);
		ret= gfal_srm_file_handle_create(ret, p);
	}

	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
	
}

/**
 * read function for the srm  plugin
 */
int gfal_srm_readG(catalog_handle ch, gfal_file_handle fd, void* buff, size_t count, GError** err){
	gfal_handle handle = (gfal_handle)	ch;	
	GError* tmp_err=NULL;
	int ret =  gfal_catalog_readG(handle, gfal_srm_file_handle_map(fd), buff, count, &tmp_err);	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;		
}

/**
 * write function for the srm  plugin
 */
int gfal_srm_writeG(catalog_handle ch, gfal_file_handle fd, void* buff, size_t count, GError** err){
	gfal_handle handle = (gfal_handle)	ch;	
	GError* tmp_err=NULL;
	int ret = gfal_catalog_writeG(handle, gfal_srm_file_handle_map(fd), buff, count, &tmp_err);	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}

int gfal_srm_closeG(catalog_handle ch, gfal_file_handle fh, GError ** err){
	gfal_handle handle = (gfal_handle)	ch;	
	GError* tmp_err=NULL;
	int ret = gfal_catalog_closeG(handle, gfal_srm_file_handle_map(fh), &tmp_err);
	if(ret ==0){
		// put done !!
		gfal_srm_file_handle_delete(fh);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
		
	return ret;	
}


