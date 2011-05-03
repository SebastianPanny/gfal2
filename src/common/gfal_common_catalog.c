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

#include "gfal_common_catalog.h"

static gfal_catalog_interface catalog_list[MAX_CATALOG_LIST];
static int Catalog_number = -1;

/**
 * Instance all catalogs for use if it's not the case
 */
void gfal_catalogs_instance(gfal_handle handle, GError** err){
	if(Catalog_number <= 0){
		GError* tmp_err=NULL;
		static gfal_catalog_interface (*constructor[])(gfal_handle,GError**)  = { &lfc_initG}; // JUST MODIFY THIS LINE IN ORDER TO ADD CATALOG
		const int size_catalog = 1;
		int i;
		for(i=0; i < size_catalog ;++i){
			gfal_catalog_interface catalog = constructor[i](handle, &tmp_err);
			if(tmp_err){
				g_propagate_prefixed_error(err, tmp_err, "[gfal_catalogs_instance]");
				return;
			}
		}
		Catalog_number=i;
	}
}

/**
 * Delete all instance of catalogs 
 */
void gfal_catalogs_delete(){
	if(Catalog_number > 0){
			int i;
			for(i=0; i< Catalog_number; ++i){
				catalog_list[i].catalog_delete(catalog_list[i].handle);
			}
		
		Catalog_number =-1;
	}
	
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

