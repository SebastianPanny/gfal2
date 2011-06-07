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
#include "../gfal_constants.h"
#include "gfal_types.h"
#include "../mds/gfal_common_mds.h"
#include "gfal_common_interface.h"
#include "lfc_ifce_ng.h"
#include "gfal_common_errverbose.h"


static int gfal_lfc_startSession(struct lfc_ops* ops, GError ** err){ 
	if (ops->startsess (ops->lfc_endpoint, (char*) gfal_version ()) < 0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno,"[%s] Error while start session with lfc, lfc_endpoint: %s, Error : %s ",
								__func__, ops->lfc_endpoint, ops->sstrerror(*ops->serrno));
		return -1;
	}
	return 0;
}

static int gfal_lfc_startTransaction(struct lfc_ops* ops, GError ** err){ 
	if (ops->starttrans(ops->lfc_endpoint, (char*) gfal_version ()) < 0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno,"[%s] Error while start transaction with lfc, lfc_endpoint: %s, Error : %s ",
										__func__, ops->lfc_endpoint, ops->sstrerror(*ops->serrno));
		return -1;
	}
	return 0;
}

static int gfal_lfc_endTransaction(struct lfc_ops* ops, GError ** err){ 
	if (ops->endtrans() < 0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno,"[%s] Error while start transaction with lfc, Error : %s ",
										__func__, ops->sstrerror(*ops->serrno));
		return -1;
	}
	return 0;
}


static int gfal_lfc_abortTransaction(struct lfc_ops* ops, GError ** err){ 
	if (ops->aborttrans() < 0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno,"[%s] Error while abort transaction with lfc,  Error : %s ",
										__func__,  ops->sstrerror(*ops->serrno));
		return -1;
	}
	return 0;
}

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
		lfc_endpoint =strndupa ( lfc_host + 6, GFAL_HOSTNAME_MAXLEN);
	else /* just a plain hostname */
		lfc_endpoint =strndupa (lfc_host, GFAL_HOSTNAME_MAXLEN);

	if ((lfc_port = strchr (lfc_endpoint, ':')) != NULL){	// register LFC_PORT var and clear port number
		if (strnlen (lfc_port+1,6) > 5  && atoi(lfc_port) ==0) {
			g_set_error(err,0, EINVAL, "[gfal_define_lfc_env_var] %s: Invalid LFC port number", lfc_port+1);
			free(lfc_endpoint);
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
	GError* tmp_err=NULL;
	char* lfn=NULL;
	int size = 0;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	struct lfc_linkinfo* links = NULL;
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
				free(lfc_host);
				return NULL;
			}	
		}
	} 

	if (strnlen (lfc_host,GFAL_MAX_LFCHOST_LEN) + 6 >= GFAL_MAX_LFCHOST_LEN) { 
		g_set_error(err, 0, ENAMETOOLONG, "[gfal_get_lfchost] lfc host name :  %s, Host name too long", lfc_host);
		free(lfc_host);
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
							(void**) &(lfc_sym->rename), (void**) &(lfc_sym->opendirg), (void**) &(lfc_sym->rmdir), (void**) &(lfc_sym->startsess), (void**) &(lfc_sym->endsess), 
							(void**) &(lfc_sym->closedir), (void**) &(lfc_sym->readdir) };
	const char* sym_list[] = { "serrno", "sstrerror", "lfc_creatg", "lfc_delreplica", "lfc_aborttrans",
						"lfc_endtrans", "lfc_getpath", "lfc_getlinks", "lfc_getreplica", "lfc_lstat", 
						"lfc_mkdirg", "lfc_seterrbuf", "lfc_setfsizeg", "lfc_setfsize", "lfc_starttrans",
						"lfc_statg", "lfc_statr", "lfc_symlink", "lfc_unlink", "lfc_access", "lfc_chmod",
						"lfc_rename", "lfc_opendirg", "lfc_rmdir", "lfc_startsess", "lfc_endsess", "lfc_closedir", "lfc_readdir" };
	ret = resolve_dlsym_listG(dlhandle, f_list, sym_list, 28, &tmp_err);
	if(ret != 0){
		g_propagate_prefixed_error(err, tmp_err,"[gfal_load_lfc]");
		free(lfc_sym);
		return NULL;
	} 
	return lfc_sym;
}

/***
 *  convert a internal lfc statg to a POSIX lfc stat
 *  struct must be already allocated
 */
int gfal_lfc_convert_statg(struct stat* output, struct lfc_filestatg* input, GError** err){
	g_return_val_err_if_fail(output && input, -1, err, "[gfal_lfc_convert_statg] Invalid args statg/stat");
	output->st_mode = input->filemode;
	output->st_nlink = input->nlink;
	output->st_uid = input->uid;
	output->st_gid = input->gid;
	output->st_size = input->filesize;
	output->st_atime = input->atime;
	output->st_ctime = input->ctime;
	output->st_mtime = input->mtime;
	return 0;
}

/***
 *  convert a internal lfc lstat to a POSIX lfc stat
 *  struct must be already allocated
 */
int gfal_lfc_convert_lstat(struct stat* output, struct lfc_filestat* input, GError** err){
	g_return_val_err_if_fail(output && input, -1, err, "[gfal_lfc_convert_lstat] Invalid args statg/stat");
	output->st_mode = input->filemode;
	output->st_nlink = input->nlink;
	output->st_uid = input->uid;
	output->st_gid = input->gid;
	output->st_size = input->filesize;
	output->st_atime = input->atime;
	output->st_ctime = input->ctime;
	output->st_mtime = input->mtime;
	return 0;
}



/**
 * basic wrapper mkdir to the lfc api
 */
static int gfal_lfc_mkdir(struct lfc_ops* ops, const char* path, mode_t mode, GError** err){
	
	char struid[37];
	gfal_generate_guidG(struid,NULL);
	
	if(ops->mkdirg (path, struid, mode)){ 
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno,"[%s] Error while mkdir call in the lfc %s", __func__,strerror(sav_errno));
		return -1;
	}				
	return 0;	
}


/**
 * Begin a recursive call on mkdir to create a full tree path
 * @warning not safe, please ensure that string begin by "/"
 * 
 */
int gfal_lfc_mkdir_rec(struct lfc_ops* ops, char* browser_path, const char* full_path, mode_t mode, GError** err){
	int ret=-1;
	char* next_sep= strchr(browser_path, G_DIR_SEPARATOR); 
	if(  next_sep == NULL || *(next_sep+1) == '\0'){ // last folder
		return gfal_lfc_mkdir(ops, full_path, mode, err);
	}else{
		const int path_size =next_sep - full_path; 
		GError* tmp_err = NULL;
		char path[ path_size+1];
		
		*((char*) mempcpy(path, full_path, path_size )) = '\0';
		ret = gfal_lfc_mkdir(ops, path, ( 0700 | mode) , &tmp_err);
		if( ret== 0 || tmp_err->code == EEXIST || tmp_err->code == EACCES){
			g_clear_error(&tmp_err);
			return gfal_lfc_mkdir_rec(ops, next_sep+1, full_path, mode, err);
		}
		g_propagate_error(err, tmp_err);
		return ret;
	}
	
} 

/**
 *  Implementation of mkdir -p call on the lfc
 * 
 * */
int gfal_lfc_ifce_mkdirpG(struct lfc_ops* ops,const char* path, mode_t mode, gboolean pflag, GError**  err){
	g_return_val_err_if_fail( ops && path, -1, err, "[gfal_lfc_ifce_mkdirpG] Invalid args in ops or/and path");
	int ret, end_trans;
	GError* tmp_err = NULL;
	
	ret = gfal_lfc_startTransaction(ops, &tmp_err);
	if(ret >=0 ){
		ret = gfal_lfc_mkdir(ops, path, mode, &tmp_err); // try to create the directory, suppose the non-recursive case
		if( tmp_err && tmp_err->code == ENOENT
					&& pflag){ // failure on the simple call, try to do a recursive create
			errno = 0;
			g_clear_error(&tmp_err);
			ret = gfal_lfc_mkdir_rec(ops, (char*)path+1, path,  mode, &tmp_err);
		}
		if( ret == 0)
			ret =gfal_lfc_endTransaction(ops,&tmp_err);
		else
			gfal_lfc_abortTransaction(ops,NULL);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	else
		errno = 0;
	return ret;
}
/**
 * return a list of surls from a getreplica request
 * 
 */
char ** gfal_lfc_getSURL(struct lfc_ops* ops, const char* path, GError** err){
	struct lfc_filereplica* list = NULL;
	char **replicas = NULL;
	int size=0,i;
	
	if (ops->getreplica (path, NULL, NULL, &size, &list) < 0) {
		errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err, 0, errno, "[%s] error reported from lfc : %s", __func__, ops->sstrerror (*ops->serrno));
		return NULL;
	}
	replicas = malloc( sizeof(char)* (size+1));
	replicas[size]= NULL;
	for(i=0; i< size; ++i){
		replicas[i] = strndup(list[i].sfn, GFAL_URL_MAX_LEN);
	}
	return replicas;
	
}

