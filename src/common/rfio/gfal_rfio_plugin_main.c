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
 * @file gfal_rfio_plugin_main.c
 * @brief file for the external plugion rfio for gfal ( based on the old rfio part in gfal legacy )
 * @author Devresse Adrien
 * @version 0.1
 * @date 30/06/2011
 * 
 **/


#include <regex.h>
#include <time.h> 
 
#include "gfal_common_srm.h"
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_catalog.h"

gboolean gfal_rfio_check_url(catalog_handle, const char* url,  catalog_mode mode, GError** err);

/**
 * Init function, called before all
 * */
gfal_catalog_interface gfal_plugin_init(gfal_handle handle, GError** err){
	gfal_catalog_interface rfio_catalog;
	GError* tmp_err=NULL;
	memset(&srm_catalog,0,sizeof(gfal_catalog_interface));	// clear the catalog	

	rfio_catalog.handle = (void*) handle;	
	rfio_catalog.check_catalog_url = &gfal_rfio_check_url;

	return srm_catalog;
}



gboolean gfal_rfio_check_url(catalog_handle, const char* url,  catalog_mode mode, GError** err){
	
	
	
}
