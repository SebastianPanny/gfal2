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
#include "gfal_prototypes.h"
#include "gfal_common_filedescriptor.h"

/**
 * 
 * generate a new unique key
 * */
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

/**
 * Add the given file handle to the and return a file descriptor
 * @return return the associated key if success else 0 and set err 
 * */
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

/**
 *  return the associated file handle for the given file descriptor or NULL if the key is not present and err is set
 * */
gpointer gfal_get_file_desc(gfal_file_descriptor_handle fhandle, int key, GError** err){
	GHashTable* c = fhandle->container;	
	gpointer p =  g_hash_table_lookup(c, GINT_TO_POINTER(key));
	if(!p)
		g_set_error(err,0, EBADR, "[%s] bad file descriptor");
	return p;
}

/**
 * remove the associated file handle associated with the given file descriptor
 * return true if success else false
 * */
gboolean gfal_remove_file_desc(gfal_file_descriptor_handle fhandle, int key, GError** err){
	GHashTable* c = fhandle->container;	
	gboolean p =  g_hash_table_remove(c, GINT_TO_POINTER(key));
	if(!p)
		g_set_error(err,0, EBADR, "[%s] bad fiel descriptor");
	return p;	  
 }
 
 
 /***
  * create a new file descriptor container with the given destroyer function to an element of the container
  */
 gfal_file_descriptor_handle gfal_file_descriptor_handle_create(GDestroyNotify destroyer){
	  gfal_file_descriptor_handle d = calloc(1, sizeof(struct _gfal_file_descriptor_container));
	  d->container = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, destroyer);
	  return d;	 
 }
 
 
 
 /**
 * 
 * return the file handle associated with the file_desc
 * @warning does not free the handle
 * 
 * */
gfal_directory_handle gfal_dir_handle_bind(gfal_file_descriptor_handle h, int file_desc, GError** err){
	g_return_val_err_if_fail(file_desc, 0, err, "[gfal_dir_handle_bind] invalid dir descriptor");
	GError* tmp_err = NULL;
	gfal_directory_handle resu=NULL;
	resu = gfal_get_file_desc(h, file_desc, &tmp_err);		
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return resu;
}

/**
 * 
 *  create a file handle with a given module_id ( id of the catalog) and a given file descriptor ( catalog's fiel descriptor )
 * 
 *  * */
int gfal_dir_handle_create(gfal_file_descriptor_handle h, int module_id, gpointer real_file_desc, GError** err){
	g_return_val_err_if_fail(module_id && real_file_desc, 0, err, "[gfal_dir_handle_create] invalid dir descriptor");
	GError* tmp_err = NULL;
	int resu=-1;
	gfal_directory_handle dir = malloc(sizeof(struct _gfal_file_handle_));
	dir->module_id = module_id;
	dir->fdesc= real_file_desc;
	resu = gfal_add_new_file_desc(h, dir, &tmp_err);			
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return resu;	
}




/**
 * 
 *  delete the handle associated with the given key, return TRUE if success else FALSE
 * 
 *  * */
gboolean gfal_dir_handle_delete(gfal_file_descriptor_handle h, int file_desc, GError** err){
	g_return_val_err_if_fail(file_desc, FALSE, err, "[gfal_dir_handle_delete] invalid dir descriptor");
	GError* tmp_err = NULL;
	gboolean resu=FALSE;	
	resu = gfal_remove_file_desc(h, file_desc, &tmp_err);			
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return resu;	
}



