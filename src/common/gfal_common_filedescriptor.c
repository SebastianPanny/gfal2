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
 * @file gfal_common_filedescriptor.c
 * @brief file for the file descriptor management
 * @author Devresse Adrien
 * @version 2.0
 * @date 22/05/2011
 * */

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "gfal_common_filedescriptor.h"

int gfal_file_key_generatorG(gfal_file_descriptor_handle fhandle, GError** err){
	g_return_val_err_if_fail(fhandle, 0, err, "[gfal_file_descriptor_generatorG] Invalid  arg file handle");
	int ret= rand();
	GHashTable* c = fhandle->container;
	if(g_hash_table_size(c) > G_MAXINT/2 ){
		g_set_error(err, 0, EMFILE, " [%s] too many files open", __func__);
		ret = 0;
	}else {
		while(ret ==0 || g_hash_table_lookup(c, GINT_TO_POINTER(ret)) != NULL){
			ret = rand();
		}
	}
	return ret;
}


int gfal_add_new_file_desc(gfal_file_descriptor_handle fhandle, gpointer pfile, GError** err){
	g_return_val_err_if_fail(fhandle && pfile, 0, err, "[gfal_add_new_file_desc] Invalid  arg fhandle and/or pfile");
	GError* tmp_err=NULL;
	GHashTable* c = fhandle->container;
	int key = gfal_file_descriptor_generatorG(fhandle, &tmp_err);
	if(key !=0){
		g_hash_table_insert(c, GINT_TO_POINTER(key), pfile);
	} 
	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	}
	return (key)?0:-1;
}
