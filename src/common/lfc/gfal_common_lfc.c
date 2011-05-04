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
#include "lfc_ifce_ng.h"
#include "gfal_common_internal.h"




static void lfc_destroyG(catalog_handle handle){
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	free(ops->lfc_endpoint);
	free(ops);
	// do nothing for the moment, global instance mode
}

int lfc_accessG(catalog_handle handle, char* lfn, int mode, GError** err){
	g_return_val_err_if_fail(handle && lfn, -1, err, "[lfc_accessG] Invalid value in arguments handle  or/and path");
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	int ret = ops->access(lfn, mode);
	if(ret <0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err, 0, sav_errno, "[lfc_accessG] lfc access error, lfc_endpoint :%s,  file : %s, error : %s", ops->lfc_endpoint, lfn, ops->sstrerror(sav_errno) );
		return sav_errno; 
	}
	return ret;
}


/**
 * Map function for the lfc interface
 *  this function provide the generic CATALOG interface for the LFC catalog.
 *  lfc_initG do : liblfc shared library load, sym resolve, endpoint check, and catalog function map.
 * 
 * */
gfal_catalog_interface lfc_initG(gfal_handle handle, GError** err){
	gfal_catalog_interface lfc_catalog;
	GError* tmp_err=NULL;
	memset(&lfc_catalog,0,sizeof(gfal_catalog_interface));	// clear the catalog
	
	char* endpoint = gfal_setup_lfchost(handle, &tmp_err);
	if(endpoint==NULL){
		g_propagate_prefixed_error(err, tmp_err, "[lfc_initG]");
		return lfc_catalog;
	}
	
	struct lfc_ops* ops = gfal_load_lfc( "liblfc.so", &tmp_err);
	if(ops ==NULL){
		g_propagate_prefixed_error(err, tmp_err,"[lfc_initG]");
		return lfc_catalog;
	}
	ops->lfc_endpoint = endpoint;
	gfal_print_verbose(GFAL_VERBOSE_VERBOSE, "[gfal][lfc] lfc endpoint : %s", endpoint);
	lfc_catalog.handle = (void*) ops;
	lfc_catalog.catalog_delete = &lfc_destroyG;
	lfc_catalog.accessG = &lfc_accessG;
	return lfc_catalog;
}



