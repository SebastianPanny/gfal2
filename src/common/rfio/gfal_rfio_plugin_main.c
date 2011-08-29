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
 * @brief file for the external rfio's plugin  for gfal ( based on the old rfio part in gfal legacy )
 * @author Devresse Adrien
 * @version 0.1
 * @date 30/06/2011
 * 
 **/
 
#define _GNU_SOURCE


#include <regex.h>
#include <time.h> 
#include <string.h>
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_catalog.h"
#include "../gfal_types.h"
#include "gfal_rfio_plugin_layer.h"
#include "gfal_rfio_plugin_main.h"
#include "gfal_rfio_plugin_bindings.h"

gboolean gfal_rfio_check_url(catalog_handle, const char* url,  catalog_mode mode, GError** err);
gboolean gfal_rfio_internal_check_url(gfal_plugin_rfio_handle rh, const char* surl, GError** err);
const char* gfal_rfio_getName();
void gfal_rfio_destroyG(catalog_handle handle);


int gfal_rfio_regex_compile(regex_t * rex, GError** err){
	int ret = regcomp(rex, "^rfio://([:alnum:]|-|/|\.|_)+$",REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret==0,-1,err,"[gfal_rfio_internal_check_url] fail to compile regex, report this bug");
	return ret;	
}

/**
 * Init function, called before all
 * */
gfal_catalog_interface gfal_plugin_init(gfal_handle handle, GError** err){
	gfal_catalog_interface rfio_catalog;
	GError* tmp_err=NULL;
	memset(&rfio_catalog,0,sizeof(gfal_catalog_interface));	// clear the catalog	
	gfal_plugin_rfio_handle h = g_new(struct _gfal_plugin_rfio_handle,1);
	h->handle = handle;
	h->rf = gfal_rfio_internal_loader(&tmp_err);
	gfal_rfio_regex_compile(&h->rex, err);
	rfio_catalog.handle = (void*) h;	
	rfio_catalog.check_catalog_url = &gfal_rfio_check_url;
	rfio_catalog.getName= &gfal_rfio_getName;
	rfio_catalog.catalog_delete= &gfal_rfio_destroyG;
	rfio_catalog.openG= &gfal_rfio_openG;
	rfio_catalog.closeG= &gfal_rfio_closeG;
	rfio_catalog.readG= &gfal_rfio_readG;
	rfio_catalog.writeG= &gfal_rfio_writeG;
	rfio_catalog.lseekG = &gfal_rfio_lseekG;
	rfio_catalog.statG = &gfal_rfio_statG;
	rfio_catalog.lstatG= &gfal_rfio_lstatG;
	rfio_catalog.opendirG = &gfal_rfio_opendirG;
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return rfio_catalog;
}


gboolean gfal_rfio_internal_check_url(gfal_plugin_rfio_handle rh, const char* surl, GError** err){
	if(surl == NULL || strnlen(surl, GFAL_URL_MAX_LEN) == GFAL_URL_MAX_LEN){
		g_set_error(err, 0, EINVAL, "[%s] Invalid surl, surl too long or NULL",__func__);
		return FALSE;
	}	
	int ret=  regexec(&rh->rex,surl,0,NULL,0);
	return (ret==0)?TRUE:FALSE;
}


/**
 *  Check the rfio url in the gfal module way
 * */
gboolean gfal_rfio_check_url(catalog_handle ch, const char* url,  catalog_mode mode, GError** err){
	int ret;
	GError* tmp_err=NULL;
	gfal_plugin_rfio_handle rh = (gfal_plugin_rfio_handle) ch;
	switch(mode){
			case GFAL_CATALOG_OPEN:
			case GFAL_CATALOG_STAT:
			case GFAL_CATALOG_LSTAT:
				ret = gfal_rfio_internal_check_url(rh, url, &tmp_err);
				break;
			default:
				ret =  FALSE;
				break;
	}	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}

void gfal_rfio_destroyG(catalog_handle handle){
	gfal_plugin_rfio_handle h = (gfal_plugin_rfio_handle) handle;
	g_free(h->rf);
	regfree(&h->rex);
	g_free(h);
}

