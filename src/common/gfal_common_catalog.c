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

static char* get_default_cat(){
	return GFAL_DEFAULT_CATALOG_TYPE;
}

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

