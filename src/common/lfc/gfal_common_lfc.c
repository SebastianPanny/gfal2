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
  * 
  @file gfal_common_lfc.c
  @brief file for the lfc catalog module
  @author Adrien Devresse
  @version 0.0.1
  @date 29/04/2011
 */

#include "gfal_common_lfc.h"




static void lfc_destroy(gpointer handle){
	
	// do nothing for the moment, global instance mode
}

int lfc_accessG(char* path, int mode, GError** err){
	g_error(" not implemented");
	
}


/**
 * Map function for the lfc interface, need to be rewrite without old error management
 * 
 * */
gfal_catalog_interface lfc_initG(GError** err){
	gfal_catalog_interface lfc_catalog;
	const int err_size = GFAL_ERRMSG_LEN;
	char errbuff[err_size];

	memset(errbuff,0, sizeof(char)* err_size);	
	/*const int ret = lfc_init(errbuff,err_size);
	if(ret < 0){
		g_set_error(err,0,errno,"[lfc_initG] LFC Error :%s ",errbuff);
		return lfc_catalog;
	}*/
	lfc_catalog.catalog_handle = NULL;
	lfc_catalog.catalog_delete = &lfc_destroy;
	lfc_catalog.accessG = &lfc_accessG;
	g_error(" not implemented");
	return ;
}



