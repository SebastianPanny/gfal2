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
  @file lfc_ifce_ng.c
  @brief main internal file of the lfc catalog module
  @author Adrien Devresse
  @version 0.0.1
  @date 02/05/2011
 */
#define _GNU_SOURCE
#define GFAL_LFN_MAX_LEN	2048

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "gfal_types.h"
#include "gfal_constants.h"
#include "../mds/gfal_common_mds.h"
#include "gfal_common_interface.h"
#include "lfc_ifce_ng.h"
#include "gfal_common_errverbose.h"


char* gfal_get_lfchost_envar(GError** err){

	char *lfc_host=NULL, *lfc_port=NULL;
	char *lfc_endpoint=NULL;
	int port=0;
	if( (lfc_host = getenv ("LFC_HOST")) !=NULL){
		if (strnlen (lfc_host,GFAL_MAX_LFCHOST_LEN) + 6 >= GFAL_MAX_LFCHOST_LEN)  {
				g_set_error(err, 0, ENAMETOOLONG, "[gfal_get_lfchost_envar] Host name from LFC_HOST env var too long");
				return NULL;
		}

		lfc_port = getenv ("LFC_PORT");
		if ( lfc_port  && (strnlen (lfc_port,6) > 5  || (port = atoi(lfc_port)) == 0) ) {
			g_set_error(err, 0, EINVAL , "[gfal_get_lfchost_envar]  Invalid LFC port number in the LFC_PORT env var");
			return NULL;
		}

		if (lfc_port)
			lfc_endpoint = g_strdup_printf("%s:%d", lfc_host, port);
		else
			lfc_endpoint = g_strdup_printf("%s", lfc_host);	

	}
	//g_printerr("my host : %s", lfc_endpoint);
	return lfc_endpoint;
}

static int gfal_define_lfc_env_var(char* lfc_host, GError** err){
	char* lfc_endpoint=NULL, *lfc_port = NULL;
	if (strncmp (lfc_host, "lfc://", 6) == 0)				// correct the url begining with lfc://
		strncpy (lfc_endpoint, lfc_host + 6, GFAL_HOSTNAME_MAXLEN);
	else /* just a plain hostname */
		strncpy (lfc_endpoint, lfc_host, GFAL_HOSTNAME_MAXLEN);

	if ((lfc_port = strchr (lfc_endpoint, ':')) != NULL){	// register LFC_PORT var and clear port number
		if (strnlen (lfc_port+1,6) > 5  && atoi(lfc_port) ==0) {
			g_set_error(err,0, EINVAL, "[gfal_define_lfc_env_var] %s: Invalid LFC port number", lfc_port+1);
			return (-1);
		}
		setenv("LFC_PORT", lfc_port+1,1);
		*lfc_port = '\0';	
		setenv("LFC_HOST", lfc_host, 1);
	}
	return 0;
}

/**
 * convert a guid to a lfn link with a call to the lfclib
 * @param gfal handle
 * @param string of the guid
 * @param err : Error report system
 * @return : string of the lfn if success or NULL char* if error
 * */
 char* gfal_convert_guid_to_lfn(catalog_handle handle, char* guid, GError ** err){
	char* lfn=NULL;
	int size = 0;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	struct lfc_linkinfo* links = NULL;
	if (ops->startsess (ops->lfc_endpoint, (char*) gfal_version ()) < 0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno,"[gfal_convert_guid_to_lfn] Error while start session with lfclib, lfc_endpoint: %s, guid : %s, Error : %s ", ops->lfc_endpoint, guid,ops->sstrerror(*ops->serrno));
		return NULL;
	}
	if(ops->getlinks(NULL, guid, &size, &links) <0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno, "[gfal_convert_guid_to_lfn] Error while getlinks() with lfclib, lfc_endpoint: %s, guid : %s, Error : %s ", ops->lfc_endpoint,guid, ops->sstrerror(*ops->serrno));
		return NULL;
	}
	if(!links || strnlen(links[0].path, GFAL_LFN_MAX_LEN) >= GFAL_LFN_MAX_LEN){
		g_set_error(err,0,EINVAL, "[gfal_convert_guid_to_lfn] Error no links associated with this guid or corrupted one : %s", guid);
		return NULL;
	}
	lfn =  strdup(links[0].path);
	free(links);
	return lfn;
 }

/**
 * setup the lfc_host correctly for the lfc calls 
 * @param err GError report system if 
 * @return  string of the endpoint, need to be free or NULL if error
 */
char* gfal_setup_lfchost(gfal_handle handle, GError ** err){
	g_return_val_err_if_fail(handle && err, NULL, err, "[gfal_setup_lfchost] Invalid parameters handle & err");
	char* lfc_host = NULL;
	GError* tmp_err = NULL;
	
	if ( (lfc_host = gfal_get_lfchost_envar(&tmp_err)) == NULL ) { // if env var not specified got one from bdii, and setup the env var
		if(tmp_err){
			g_propagate_prefixed_error(err, tmp_err, "[gfal_get_lfchost]");
			return NULL;
		}
		
		if( (lfc_host = gfal_get_lfchost_bdii(handle, &tmp_err)) ==NULL ){ // call the bdii 
			g_propagate_prefixed_error(err, tmp_err, "[gfal_get_lfchost]");
			return NULL;	
		}else{				
			gfal_define_lfc_env_var(lfc_host, &tmp_err);		// define the env var if sucess
			if(tmp_err){
				g_propagate_prefixed_error(err, tmp_err,"[gfal_get_lfchost]");
				return NULL;
			}	
		}
	} 

	if (strnlen (lfc_host,GFAL_MAX_LFCHOST_LEN) + 6 >= GFAL_MAX_LFCHOST_LEN) { 
		g_set_error(err, 0, ENAMETOOLONG, "[gfal_get_lfchost] lfc host name :  %s, Host name too long", lfc_host);
		return (NULL);		
	}	
	//g_printerr(" my host : %s", lfc_host);
	return lfc_host;
}

/**
 * load the shared library and link the symbol for the LFC usage
 * @param name : name of the library
 * @param err:  error report
*/
struct lfc_ops* gfal_load_lfc(const char* name, GError** err){
	struct lfc_ops* lfc_sym=NULL;
	GError* tmp_err=NULL;
	int ret = -1;
	void* dlhandle=NULL;
	if ((dlhandle = dlopen (NULL, RTLD_LAZY)) == NULL) {		// try to dlopen on the dependencies
		g_set_error(err, 0, EPROTONOSUPPORT, "[gfal_load_lfc] Error in lfc module shared library was not open properly : %s",dlerror () );
		return NULL;
	}
	lfc_sym = calloc(1, sizeof(struct lfc_ops));
	lfc_sym->addreplica = (int (*) (const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *)) dlsym (dlhandle, "lfc_addreplica");	
	if (lfc_sym->addreplica == NULL) {
		if ((dlhandle = dlopen (name, RTLD_LAZY)) == NULL) { // try to dlopen on a fixed name shared library
			g_set_error(err, 0, EPROTONOSUPPORT, "[gfal_load_lfc] Error in lfc module shared library was not open properly : %s",dlerror () );
			return NULL;
		}

		lfc_sym->addreplica = (int (*) (const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *)) dlsym (dlhandle, "lfc_addreplica");
	}
	
	void** f_list[] = {  (void**)&(lfc_sym->serrno), (void**) &(lfc_sym->sstrerror), (void**) &(lfc_sym->creatg), (void**) &(lfc_sym->delreplica), (void**) &(lfc_sym->aborttrans), 
							(void**) &(lfc_sym->endtrans), (void**) &(lfc_sym->getpath), (void**) &(lfc_sym->getlinks), (void**) &(lfc_sym->getreplica), (void**) &(lfc_sym->lstat), 
							(void**) &(lfc_sym->mkdirg), (void**) &(lfc_sym->seterrbuf), (void**) &(lfc_sym->setfsizeg), (void**) &(lfc_sym->setfsize), (void**) &(lfc_sym->starttrans),
							(void**) &(lfc_sym->statg), (void**) &(lfc_sym->statr), (void**) &(lfc_sym->symlink), (void**) &(lfc_sym->unlink), (void**) &(lfc_sym->access), (void**) &(lfc_sym->chmod),
							(void**) &(lfc_sym->rename), (void**) &(lfc_sym->opendirg), (void**) &(lfc_sym->rmdir), (void**) &(lfc_sym->startsess), (void**) &(lfc_sym->endsess) };
	const char* sym_list[] = { "serrno", "sstrerror", "lfc_creatg", "lfc_delreplica", "lfc_aborttrans",
						"lfc_endtrans", "lfc_getpath", "lfc_getlinks", "lfc_getreplica", "lfc_lstat", 
						"lfc_mkdirg", "lfc_seterrbuf", "lfc_setfsizeg", "lfc_setfsize", "lfc_starttrans",
						"lfc_statg", "lfc_statr", "lfc_symlink", "lfc_unlink", "lfc_access", "lfc_chmod",
						"lfc_rename", "lfc_opendirg", "lfc_rmdir", "lfc_startsess", "lfc_endsess" };
	ret = resolve_dlsym_listG(dlhandle, f_list, sym_list, 26, &tmp_err);
	if(ret != 0){
		g_propagate_prefixed_error(err, tmp_err,"[gfal_load_lfc]");
		free(lfc_sym);
		return NULL;
	} 
	return lfc_sym;
}

