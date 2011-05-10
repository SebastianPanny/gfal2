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
 * @file gfal_common_catalog.c
 * @brief the file of the common lib for the catalog management
 * @author Devresse Adrien
 * @version 0.0.1
 * @date 8/04/2011
 * */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfal_common_catalog.h"
#include "lfc/gfal_common_lfc.h"
#include "gfal_constants.h"
#include "gfal_common_errverbose.h"


/**
 *  Note that hte default catalog is the first one
 */
static gfal_catalog_interface (*constructor[])(gfal_handle,GError**)  = { &lfc_initG}; // JUST MODIFY THIS TWO LINE IN ORDER TO ADD CATALOG
static const int size_catalog = 1;

/**
 * Instance all catalogs for use if it's not the case
 *  return the number of catalog available
 */
int gfal_catalogs_instance(gfal_handle handle, GError** err){
	g_return_val_err_if_fail(handle, -1, err, "[gfal_catalogs_instance]  invalid value of handle");
	const int catalog_number = handle->catalog_opt.catalog_number;
	if(catalog_number <= 0){
		GError* tmp_err=NULL;
		int i;
		for(i=0; i < size_catalog ;++i){
			gfal_catalog_interface catalog = constructor[i](handle, &tmp_err);
			handle->catalog_opt.catalog_list[i] = catalog;
			if(tmp_err){
				g_propagate_prefixed_error(err, tmp_err, "[gfal_catalogs_instance]");
				return -1;
			}
		}
		handle->catalog_opt.catalog_number=i;
	}
	return handle->catalog_opt.catalog_number;
}

/**
 *  Execute an access function on the first catalog compatible in the catalog list
 *  return the result of the first valid catalog for a given URL
 *  @return result of the access method or -1 if error and set GError with the correct value
 *  error : EPROTONOSUPPORT means that the URL is not matched by a catalog
 *  */
int gfal_catalogs_accessG(gfal_handle handle, char* path, int mode, GError** err){
	g_return_val_err_if_fail(handle && path, EINVAL, err, "[gfal_catalogs_accessG] Invalid arguments");
	GError* tmp_err=NULL;
	int i;
	const int n_catalogs = gfal_catalogs_instance(handle, &tmp_err);
	if(n_catalogs <= 0){
		g_propagate_prefixed_error(err, tmp_err, "[gfal_catalogs_accessG]");
		return -1;
	}
	gfal_catalog_interface* cata_list = handle->catalog_opt.catalog_list;
	for(i=0; i < n_catalogs; ++i, ++cata_list){
		gboolean comp =  cata_list->check_catalog_url(cata_list->handle, path, GFAL_CATALOG_ACCESS, &tmp_err);
		if(tmp_err){
			g_propagate_prefixed_error(err, tmp_err,"[gfal_catalogs_accessG]");
			return -1;
		}
		if(comp){
			int ret = cata_list->accessG(cata_list->handle, path, mode, &tmp_err);
			if(tmp_err)
				g_propagate_prefixed_error(err, tmp_err,"[gfal_catalogs_accessG]"); 
			return ret;			
		}	
	}
	g_set_error(err,0,EPROTONOSUPPORT, "[gfal_catalogs_accessG] Protocol not supported or path/url invalid");
	return -1;
}
/**
 * Execute a guid access on the default specified catalog
 * @param handle 
 * @param guid of the element to check
 * @param mode : mode of the access
 * @param err : Error report system
 * @return return the content of the access call, or negative value if error
 * @warning no url syntaxe checking
 * */
int gfal_catalogs_guid_accessG(gfal_handle handle, char* guid, int mode, GError** err){
	g_return_val_err_if_fail(handle && guid, EINVAL, err, "[gfal_catalogs_guid_accessG] Invalid arguments");	
	GError* tmp_err=NULL;
	int i;
	const int n_catalogs = gfal_catalogs_instance(handle, &tmp_err);
	if(n_catalogs <= 0){
		g_propagate_prefixed_error(err, tmp_err, "[gfal_catalogs_accessG]");
		return -1;
	}	
	gfal_catalog_interface* cata_list = handle->catalog_opt.catalog_list;
	int ret = cata_list->access_guidG(cata_list->handle, guid, mode, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err,"[gfal_catalogs_accessG]"); 
	return ret;		
}

/**
 * Delete all instance of catalogs 
 */
int gfal_catalogs_delete(gfal_handle handle, GError** err){
	g_return_val_err_if_fail(handle, -1, err, "[gfal_catalogs_delete] Invalid value of handle");
	const int catalog_number = handle->catalog_opt.catalog_number;
	if(catalog_number > 0){
			int i;
			for(i=0; i< catalog_number; ++i){
				handle->catalog_opt.catalog_list[i].catalog_delete( handle->catalog_opt.catalog_list[i].handle );
			}
		
		handle->catalog_opt.catalog_number =-1;
	}
	return 0;
}
/**
 *  Execute an access methode on ALL the compatible catalogs
 *  return the result of the first valid catalog for a given URL
 *  @return result of the chmod func or the errno if error occured like a POSIX method. If No catalog can resolve this link EPROTONOSUPPORT is returned
 * */
 int gfal_catalog_chmodG(gfal_handle handle, const char* path, mode_t mode, GError** err){
	g_return_val_err_if_fail(handle && path, -1, err, "[gfal_catalog_chmodG] Invalid arguments");	
	GError* tmp_err = NULL;	
	int ret= EPROTONOSUPPORT;
	int i;
	const int n_catalogs = gfal_catalogs_instance(handle, &tmp_err);	
	if(n_catalogs > 0 && !tmp_err){
		gfal_catalog_interface* cata_list = handle->catalog_opt.catalog_list;
		for(i=0; i< n_catalogs; ++i, ++cata_list){
			gboolean b = cata_list->check_catalog_url(cata_list->handle, path, GFAL_CATALOG_CHMOD, &tmp_err);
			if(tmp_err)
				break;
			if(b){ //no error and valid catalog
				ret = cata_list->chmodG(cata_list->handle, path, mode, &tmp_err);
				break;		
			}
		}
		
	}
	if(tmp_err){
		g_propagate_prefixed_error(err, tmp_err, "[gfal_catalog_chmodG]");	
		ret = -1;
	}
	return ret;		 
 }

/**
 * return the catalog type configured at compilation time
 */
static char* get_default_cat(){
	return GFAL_DEFAULT_CATALOG_TYPE;
}

/***
 *  return the name of the current selected default catalog in a string form
 * */
extern char* gfal_get_cat_type(GError** err) {
    char *cat_env;
    char *cat_type;

    if((cat_env = getenv ("LCG_CATALOG_TYPE")) == NULL) {
		gfal_print_verbose(GFAL_VERBOSE_VERBOSE, "[get_cat_type] LCG_CATALOG_TYPE env var is not defined, use default var instead");
        cat_env = get_default_cat(); 
	}
    if((cat_type = strndup(cat_env, 50)) == NULL) {
		g_set_error(err,0,EINVAL,"[get_cat_type] invalid env var LCG_CATALOG_TYPE, please set it correctly or delete it");
        return NULL;
    }
    return cat_type;
}

