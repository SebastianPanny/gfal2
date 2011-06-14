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
 * @file gfal_common_srm_readdir.c
 * @brief file for the readdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 14/06/2011
 * */

#include <stdio.h>
#include "gfal_common_srm_readdir.h"
#include "gfal_common_errverbose.h"
#include "gfal_common_srm_opendir.h" 
#include "gfal_common_srm_internal_layer.h"
 
#define GFAL_FILENAME_MAX FILENAME_MAX

struct dirent* gfal_srm_readdirG(gfal_handle handle, gpointer fh, GError** err){
	g_return_val_err_if_fail( handle && fh, NULL, err, "[gfal_srm_readdirG] Invaldi args");
	GError* tmp_err=NULL;
	gfal_srm_opendir_handle oh = fh;
	struct dirent* current = &oh->current_readdir;
	const int nbsub = oh->srm_ls_resu->nbsubpaths;
	const int dir_offset = oh->dir_offset;
	if( dir_offset < nbsub){
		if( g_strlcpy(current->d_name, oh->srm_ls_resu->subpaths[dir_offset].surl, GFAL_FILENAME_MAX) >= GFAL_FILENAME_MAX){
			g_set_error(&tmp_err, 0, ENAMETOOLONG, "filename truncated, name too long : %s", current->d_name);	
			current = NULL;
		}else
			oh->dir_offset +=1;
	}else
		current = NULL;
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return current;
}
