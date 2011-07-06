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

#define _GNU_SOURCE
 
 /**
  * 
  @file gfal_common_lfc.c
  @brief file for the lfc catalog module
  @author Adrien Devresse
  @version 0.0.1
  @date 29/04/2011
 */

#include <regex.h>
#include "gfal_common_lfc.h"
#include "../gfal_common_internal.h"
#include "../gfal_constants.h"
#include "../gfal_common_errverbose.h"
#include "lfc_ifce_ng.h"

/**
 * just return the name of the layer
 */
static const char* lfc_getName(){
	return "lfc_plugin";
}

/**
 * convert the lfn url for internal usage
 * result must be free
 */
static char* lfc_urlconverter(const char * lfn_url, const char* prefix){
	const int pref_len = strlen(prefix);
	return strndup(pref_len+ lfn_url, GFAL_URL_MAX_LEN );
}

/**
 *  Deleter to unload the lfc part
 * */
static void lfc_destroyG(catalog_handle handle){
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	if(ops){
		free(ops->lfc_endpoint);
		free(ops);
	}
}
/**
 * Implementation of the chmod function with the LFC catalog
 * return 0 or the errno if error, or set GError if serious error
 */
int lfc_chmodG(catalog_handle handle, const char* path, mode_t mode, GError** err){
	g_return_val_err_if_fail(handle && path, -1, err, "[lfc_chmodG] Invalid valid value in handle/path ");
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	int  ret=-1;
	char* url = lfc_urlconverter(path, GFAL_LFC_PREFIX);
	ret = ops->chmod(url, mode);
	if(ret < 0){
		const int myerrno = *(ops->serrno);
		g_set_error(err, 0, myerrno, "[lfc_chmodG] Errno reported from lfc : %s ", ops->sstrerror(myerrno));
	}else
		errno =0;
	free(url);
	return ret;
}

/**
 * 
 * implementation of the access call with the lfc catalog
 *  return 0 or -1 if error and report GError** with error code and message
 */
int lfc_accessG(catalog_handle handle, const char* lfn, int mode, GError** err){
	g_return_val_err_if_fail(handle && lfn, -1, err, "[lfc_accessG] Invalid value in arguments handle  or/and path");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	char* url = lfc_urlconverter(lfn, GFAL_LFC_PREFIX);
	int ret = ops->access(url, mode);
	if(ret <0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err, 0, sav_errno, "[lfc_accessG] lfc access error, lfc_endpoint :%s,  file : %s, error : %s", ops->lfc_endpoint, lfn, ops->sstrerror(sav_errno) );
	}else
		errno=0;
	free(url);
	return ret;
}

/**
 * Implementation of the rename call for the lfc catalog
 * return 0 if success else -1 if error and set GError
 * 
 * */
int lfc_renameG(catalog_handle handle, const char* oldpath, const char* newpath, GError** err){
	g_return_val_err_if_fail(handle && oldpath && newpath, -1, err, "[lfc_renameG] Invalid value in args handle/oldpath/newpath");
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	char* surl = lfc_urlconverter(oldpath, GFAL_LFC_PREFIX);
	char* durl = lfc_urlconverter(newpath, GFAL_LFC_PREFIX);
	int ret  = ops->rename(surl, durl);
	if(ret <0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno, "[lfc_renameG] Error report from LFC : %s",  ops->sstrerror(sav_errno) );
	}
	free(surl);
	free(durl);
	return ret;	
}

/**
 * execute a posix stat request on the lfc
 * return 0 and set struct if correct answer, else return negative value and set GError
 * 
 */
int lfc_statG(catalog_handle handle, const char* path, struct stat* st, GError** err){
	g_return_val_err_if_fail(handle && path && st, -1, err, "[lfc_statG] Invalid value in args handle/path/stat");
	struct lfc_ops* ops = (struct lfc_ops*) handle;		
	char* lfn = lfc_urlconverter(path, GFAL_LFC_PREFIX);
	struct lfc_filestatg statbuf;
	
	int ret = ops->statg(lfn, NULL, &statbuf);
	if(ret != 0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno, "[%s] Error report from LFC : %s",__func__,  ops->sstrerror(sav_errno) );
	}else{
		ret= gfal_lfc_convert_statg(st, &statbuf, err);
		errno=0;
	}
	free(lfn);
	return ret;
}
/**
 * execute a posix lstat request on the lfc ( stat request with link information)
 *  return 0 if success and set the struct buf else return negative value and set GError
 */
static int lfc_lstatG(catalog_handle handle, const char* path, struct stat* st, GError** err){
	g_return_val_err_if_fail(handle && path && st, -1, err, "[lfc_lstatG] Invalid value in args handle/path/stat");
	struct lfc_ops* ops = (struct lfc_ops*) handle;		
	char* lfn = lfc_urlconverter(path, GFAL_LFC_PREFIX);
	struct lfc_filestat statbuf;
	
	int ret = ops->lstat(lfn, &statbuf);
	if(ret != 0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0,sav_errno, "[%s] Error report from LFC : %s", __func__, ops->sstrerror(sav_errno) );
	}else{
		ret= gfal_lfc_convert_lstat(st, &statbuf, err);
		errno=0;
	}
	free(lfn);
	return ret;
}
/**
 *  Execute a posix mkdir on the lfc
 *  @return 0 on success else -1 and err is set with the correct value
 * 
 * */
 static int lfc_mkdirpG(catalog_handle handle, const char* path, mode_t mode, gboolean pflag, GError** err){
	g_return_val_err_if_fail(handle && path , -1, err, "[lfc_mkdirpG] Invalid value in args handle/path");	
	GError* tmp_err = NULL; 
	struct lfc_ops* ops = (struct lfc_ops*) handle;		
	char* lfn = lfc_urlconverter(path, GFAL_LFC_PREFIX);
	
	int ret =gfal_lfc_ifce_mkdirpG(ops, lfn, mode, pflag, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	free(lfn);
	return ret; 
 }
 
/**
 * Execute a rmdir on the lfc
 * @return 0 on success else -1 and err is set with the correct value
 * */
 static int lfc_rmdirG(catalog_handle handle, const char* path, GError** err){
	 g_return_val_err_if_fail( handle && path , -1, err, "[lfc_rmdirG] Invalid value in args handle/path");	
	struct lfc_ops* ops = (struct lfc_ops*) handle;		
	char* lfn = lfc_urlconverter(path, GFAL_LFC_PREFIX);
	const int ret = ops->rmdir(lfn);
	if( ret < 0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		sav_errno = (sav_errno==EEXIST)?ENOTEMPTY:sav_errno;		// convert wrong reponse code
		g_set_error(err,0, sav_errno, "[%s] Error report from LFC %s", __func__, ops->sstrerror(sav_errno) );
	}
	free(lfn);
	return ret;	 
 }
 
/**
 * execute an opendir func to the lfc
 * */
static DIR* lfc_opendirG(catalog_handle handle, const char* name, GError** err){
	g_return_val_err_if_fail( handle && name , NULL, err, "[lfc_rmdirG] Invalid value in args handle/path");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;

	char* lfn = lfc_urlconverter(name, GFAL_LFC_PREFIX);
	DIR* d  = (DIR*) ops->opendirg(lfn,NULL);	
	if(d==NULL){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0, sav_errno, "[%s] Error report from LFC %s", __func__, ops->sstrerror(sav_errno) );
	}	
	free(lfn);
	return d;		
}

/**
 * Execute a readdir func on the lfc
 * */
static struct dirent* lfc_readdirG(catalog_handle handle, DIR* d, GError** err){
	g_return_val_err_if_fail( handle && d , NULL, err, "[lfc_rmdirG] Invalid value in args handle/path");
	GError* tmp_err=NULL;	
	struct lfc_ops *ops = (struct lfc_ops*) handle;
	
	struct dirent* ret = ops->readdir( (lfc_DIR*)d);
	if(ret ==NULL && *ops->serrno ){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0, sav_errno, "[%s] Error report from LFC %s", __func__, ops->sstrerror(sav_errno) );
	}
	return ret;
}

/**
 * execute an closedir func on the lfc
 * */
static int lfc_closedirG(catalog_handle handle, DIR* d, GError** err){
	g_return_val_err_if_fail( handle && d , -1, err, "[lfc_rmdirG] Invalid value in args handle/path");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;

	int ret = ops->closedir(d);	
	if(ret != 0){
		int sav_errno = *ops->serrno < 1000 ? *ops->serrno : ECOMM;
		g_set_error(err,0, sav_errno, "[%s] Error report from LFC %s", __func__, ops->sstrerror(sav_errno) );
	}	
	return ret;		
}

/**
 * resolve the lfc link to the surls
 */
static char ** lfc_getSURLG(catalog_handle handle, const char * path, GError** err){
	g_return_val_err_if_fail( handle && path , NULL, err, "[lfc_getSURLG] Invalid value in args handle/path");
	GError* tmp_err=NULL;
	char** resu = NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	char * lfn = lfc_urlconverter(path,  GFAL_LFC_PREFIX);
	resu = gfal_lfc_getSURL(ops, lfn, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	free(lfn);
	return resu;
}

/**
 * Convert a guid to a catalog url if possible
 *  return the link in a catalog's url string or err and NULL if not found
 */
char* lfc_resolve_guid(catalog_handle handle, const char* guid, GError** err){
	g_return_val_err_if_fail( handle && guid, NULL, err, "[lfc_resolve_guid] Invalid args in handle and/or guid ");
	char* tmp_guid = lfc_urlconverter(guid, GFAL_GUID_PREFIX);
	char* res =gfal_convert_guid_to_lfn(handle, tmp_guid, err);
	if(res){
		const int size_res = strnlen(res, GFAL_URL_MAX_LEN);
		const int size_pref = strlen(GFAL_LFC_PREFIX);
		res =  g_renew(char, res, size_res + size_pref+1); 
		memmove(res+ size_pref, res, size_res) ;
		*((char*)mempcpy(res, GFAL_LFC_PREFIX, size_pref)+size_res) ='\0';
	}
	free(tmp_guid);
	return res;
}
  


/**
 * Map function for the lfc interface
 *  this function provide the generic CATALOG interface for the LFC catalog.
 *  lfc_initG do : liblfc shared library load, sym resolve, endpoint check, and catalog function map.
 * 
 * */
gfal_catalog_interface gfal_plugin_init(gfal_handle handle, GError** err){
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
		g_propagate_prefixed_error(err, tmp_err,"[%s]", __func__);
		return lfc_catalog;
	}
	ops->lfc_endpoint = endpoint;
	gfal_print_verbose(GFAL_VERBOSE_VERBOSE, "[gfal][lfc] lfc endpoint : %s", endpoint);
	lfc_catalog.handle = (void*) ops;
	lfc_catalog.check_catalog_url= &gfal_lfc_check_lfn_url;
	lfc_catalog.catalog_delete = &lfc_destroyG;
	lfc_catalog.accessG = &lfc_accessG;
	lfc_catalog.chmodG = &lfc_chmodG;
	lfc_catalog.renameG = &lfc_renameG;
	lfc_catalog.statG = &lfc_statG;
	lfc_catalog.lstatG = &lfc_lstatG;
	lfc_catalog.mkdirpG = &lfc_mkdirpG;
	lfc_catalog.resolve_guid = &lfc_resolve_guid;
	lfc_catalog.rmdirG = &lfc_rmdirG;
	lfc_catalog.opendirG = &lfc_opendirG;
	lfc_catalog.closedirG = &lfc_closedirG;
	lfc_catalog.readdirG = &lfc_readdirG;
	lfc_catalog.getSURLG = &lfc_getSURLG;
	lfc_catalog.getName = &lfc_getName;
	return lfc_catalog;
}



/**
 * Check if the passed url and operation is compatible with lfc
 * 
 * */
 gboolean gfal_lfc_check_lfn_url(catalog_handle handle, const char* lfn_url, catalog_mode mode, GError** err){
	regex_t rex;
	int ret;
	switch(mode){
		case GFAL_CATALOG_ACCESS:
		case GFAL_CATALOG_CHMOD:
		case GFAL_CATALOG_RENAME:
		case GFAL_CATALOG_STAT:
		case GFAL_CATALOG_LSTAT:
		case GFAL_CATALOG_MKDIR:
		case GFAL_CATALOG_RMDIR:
		case GFAL_CATALOG_OPENDIR:
		case GFAL_CATALOG_GETSURL:
			ret = regcomp(&rex, "^lfn:/([:alnum:]|-|/|\.|_)+", REG_ICASE | REG_EXTENDED);
			g_return_val_err_if_fail(ret ==0,-1,err,"[gfal_lfc_check_lfn_url] fail to compile regex, report this bug");
			ret= regexec(&rex, lfn_url, 0, NULL, 0);
			regfree(&rex);
			return (!ret)?TRUE:FALSE;	
		default:
			return FALSE;
	}
}
 

