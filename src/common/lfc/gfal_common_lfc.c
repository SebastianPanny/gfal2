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
  @date 06/07/2011
 */

#include <regex.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <attr/xattr.h>
#include "gfal_common_lfc.h"
#include "gfal_common_lfc_open.h"
#include "../gfal_common_internal.h"
#include "../gfal_constants.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_filedescriptor.h"
#include "lfc_ifce_ng.h"


static gboolean init_thread = FALSE;
pthread_mutex_t m_lfcinit=PTHREAD_MUTEX_INITIALIZER;

typedef struct _lfc_opendir_handle{
	char url[GFAL_URL_MAX_LEN];
	struct dirent current_dir;
} *lfc_opendir_handle;

static char* file_xattr[] = { LFC_XATTR_GUID, LFC_XATTR_SURLS, NULL };
/**
 * just return the name of the layer
 */
const char* lfc_getName(){
	return "lfc_plugin";
}

/**
 * convert the lfn url for internal usage
 * result must be free
 */
static inline  char* lfc_urlconverter(const char * lfn_url, const char* prefix){
	const int pref_len = strlen(prefix);
	const int strsize = strnlen(lfn_url, GFAL_URL_MAX_LEN-1);
	char* p = malloc(sizeof(char) * (strsize-pref_len+1));
	char* p2 = mempcpy(p,pref_len+ lfn_url,  MIN(strsize-pref_len, GFAL_URL_MAX_LEN) * sizeof(char));
	*p2 = '\0';
	p2--;
	while(*p2 == '/'){ // remove the last /
		*p2 ='\0';
		p2--;
	}
	return p;
}

static char* url_converter(catalog_handle handle, const char * url,GError** err){
	GError* tmp_err=NULL;
	if(strnlen(url, 5) != 5){ // bad string size, return empty string
		gfal_print_verbose(GFAL_VERBOSE_VERBOSE, "lfc url converter -> bad url size");
		return strdup("");
	}
	if(strncmp(url, "lfn", 3) == 0)
		return lfc_urlconverter(url, GFAL_LFC_PREFIX);
	char buff_lfn[GFAL_URL_MAX_LEN];
	int ret = gfal_convert_guid_to_lfn_r(handle, url + GFAL_LFC_GUID_PREFIX_LEN, buff_lfn, GFAL_URL_MAX_LEN, &tmp_err);
	if(ret ==0)
		return strdup(buff_lfn);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return NULL;
}



/**
 *  Deleter to unload the lfc part
 * */
static void lfc_destroyG(catalog_handle handle){
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	if(ops){
		gsimplecache_delete(ops->cache_stat);
		free(ops->lfc_endpoint);
		regfree(&(ops->rex));
		free(ops);
	}
}
/**
 * Implementation of the chmod function with the LFC catalog
 * return 0 or the errno if error, or set GError if serious error
 */
int lfc_chmodG(catalog_handle handle, const char* path, mode_t mode, GError** err){
	g_return_val_err_if_fail(handle && path, -1, err, "[lfc_chmodG] Invalid valid value in handle/path ");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);
	int  ret=-1;
	char* url = url_converter(handle, path, &tmp_err);
	if(url){
		ret = ops->chmod(url, mode);
		if(ret < 0){
			const int myerrno = gfal_lfc_get_errno(ops);
			g_set_error(&tmp_err, 0, myerrno, "Errno reported from lfc : %s ", gfal_lfc_get_strerror(ops));
		}else{
			errno =0;
			gsimplecache_remove_kstr(ops->cache_stat, url);	
		}
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
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
	gfal_lfc_init_thread(ops); 
	gfal_auto_maintain_session(ops, &tmp_err);
	char* url = url_converter(handle, lfn, &tmp_err);
	int ret=-1;
	if(url){
		ret = ops->access(url, mode);
		if(ret <0){
			int sav_errno = gfal_lfc_get_errno(ops);
			g_set_error(&tmp_err, 0, sav_errno, "lfc access error, lfc_endpoint :%s,  file : %s, error : %s", ops->lfc_endpoint, lfn, gfal_lfc_get_strerror(ops) );
		}else
			errno=0;
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
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
	GError* tmp_err=NULL;
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);
	char* surl = lfc_urlconverter(oldpath, GFAL_LFC_PREFIX);
	char* durl = lfc_urlconverter(newpath, GFAL_LFC_PREFIX);
	int ret  = ops->rename(surl, durl);
	if(ret <0){
		int sav_errno = gfal_lfc_get_errno(ops);
		g_set_error(&tmp_err,0,sav_errno, "Error report from LFC : %s",  gfal_lfc_get_strerror(ops) );
	}else{
		gsimplecache_remove_kstr(ops->cache_stat, surl);
	}
	free(surl);
	free(durl);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}


/**
 * Implementation of the symlinkG call for the lfc catalog
 * return 0 if success else -1 if error and set GError
 * 
 * */
int lfc_symlinkG(catalog_handle handle, const char* oldpath, const char* newpath, GError** err){
	g_return_val_err_if_fail(handle && oldpath && newpath, -1, err, "[lfc_symlinkG] Invalid value in args handle/oldpath/newpath");
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	GError* tmp_err=NULL;
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);
	char* surl = lfc_urlconverter(oldpath, GFAL_LFC_PREFIX);
	char* durl = lfc_urlconverter(newpath, GFAL_LFC_PREFIX);
	int ret  = ops->symlink(surl, durl);
	if(ret <0){
		int sav_errno = gfal_lfc_get_errno(ops);
		g_set_error(&tmp_err,0,sav_errno, "Error report from LFC : %s",  gfal_lfc_get_strerror(ops) );
	}
	free(surl);
	free(durl);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}

/**
 * execute a posix stat request on the lfc
 * return 0 and set struct if correct answer, else return negative value and set GError
 * 
 */
int lfc_statG(catalog_handle handle, const char* path, struct stat* st, GError** err){
	g_return_val_err_if_fail(handle && path && st, -1, err, "[lfc_statG] Invalid value in args handle/path/stat");
	GError* tmp_err=NULL;
	int ret=-1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;		
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);
	char* lfn = url_converter(handle, path, &tmp_err);
	struct lfc_filestatg statbuf;	
	if(lfn){
		ret = gfal_lfc_statg(ops, lfn, &statbuf, &tmp_err);
		if(ret == 0){
			ret= gfal_lfc_convert_statg(st, &statbuf, err);
			errno=0;
		}
		free(lfn);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}
/**
 * execute a posix lstat request on the lfc ( stat request with link information)
 *  return 0 if success and set the struct buf else return negative value and set GError
 */
static int lfc_lstatG(catalog_handle handle, const char* path, struct stat* st, GError** err){
	g_return_val_err_if_fail(handle && path && st, -1, err, "[lfc_lstatG] Invalid value in args handle/path/stat");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	int ret=-1;	
	char* lfn = url_converter(handle, path, &tmp_err);
	struct lfc_filestat statbuf;
	
	if(lfn){
		if( ( ret= gsimplecache_take_one_kstr(ops->cache_stat, lfn, st)) == 0){ // take the version of the buffer
			gfal_print_verbose(GFAL_VERBOSE_TRACE, " lfc_lstatG -> value taken from cache");
		}else{	
			gfal_print_verbose(GFAL_VERBOSE_TRACE, " lfc_lstatG -> value not in cache, do normal call");
			gfal_lfc_init_thread(ops);
			gfal_auto_maintain_session(ops, &tmp_err);
			if(!tmp_err){
				ret = ops->lstat(lfn, &statbuf);
				if(ret != 0){
					int sav_errno = gfal_lfc_get_errno(ops);
					g_set_error(&tmp_err,0,sav_errno, "Error report from LFC : %s", __func__, gfal_lfc_get_strerror(ops) );
				}else{
					ret= gfal_lfc_convert_lstat(st, &statbuf, err);
					errno=0;
				}
			}
		}
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
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
	int ret= -1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;		
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);
	char* lfn = url_converter(handle, path, &tmp_err);
	if(lfn){
		 ret =gfal_lfc_ifce_mkdirpG(ops, lfn, mode, pflag, &tmp_err);

		free(lfn);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret; 
 }
 
/**
 * Execute a rmdir on the lfc
 * @return 0 on success else -1 and err is set with the correct value
 * */
 static int lfc_rmdirG(catalog_handle handle, const char* path, GError** err){
	g_return_val_err_if_fail( handle && path , -1, err, "[lfc_rmdirG] Invalid value in args handle/path");	
	GError* tmp_err=NULL;
	int ret = -1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;		
	gfal_lfc_init_thread(ops);
	char* lfn = url_converter(handle, path, &tmp_err);
	if(lfn){
		ret = ops->rmdir(lfn);
		if( ret < 0){
			int sav_errno = gfal_lfc_get_errno(ops);
			sav_errno = (sav_errno==EEXIST)?ENOTEMPTY:sav_errno;		// convert wrong reponse code
			g_set_error(err,0, sav_errno, "Error report from LFC %s", __func__, gfal_lfc_get_strerror(ops) );
		}
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	free(lfn);
	return ret;	 
 }
 
/**
 * execute an opendir func to the lfc
 * */
static gfal_file_handle lfc_opendirG(catalog_handle handle, const char* name, GError** err){
	g_return_val_err_if_fail( handle && name , NULL, err, "[lfc_rmdirG] Invalid value in args handle/path");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	lfc_opendir_handle oh;
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);
	char* lfn = url_converter(handle, name, &tmp_err);
	DIR* d = NULL;
	if(lfn){
		d  = (DIR*) ops->opendirg(lfn,NULL);	
		if(d==NULL){
			int sav_errno = gfal_lfc_get_errno(ops);
			g_set_error(err,0, sav_errno, "Error report from LFC %s", __func__, gfal_lfc_get_strerror(ops) );
		}else{	
			oh = g_new0(struct _lfc_opendir_handle,1);
			g_strlcpy(oh->url, lfn, GFAL_URL_MAX_LEN );
		}
		free(lfn);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return (d)?(gfal_file_handle_ext_new(lfc_getName(), (gpointer) d, (gpointer) oh)):NULL;		
}

static struct dirent* lfc_convert_dirent_struct(struct lfc_ops *ops , struct dirent* dir , struct Cns_direnstat* filestat, const char* url){
	if(filestat == NULL)
		return NULL;
	GSimpleCache* cache = ops->cache_stat;
	char fullurl[GFAL_URL_MAX_LEN];
	g_strlcpy(fullurl, url, GFAL_URL_MAX_LEN);
	g_strlcat(fullurl, "/", GFAL_URL_MAX_LEN);
	g_strlcat(fullurl, filestat->d_name, GFAL_URL_MAX_LEN);
	
	struct stat st;
	st.st_mode = (mode_t) filestat->filemode;
	st.st_nlink = (nlink_t) filestat->nlink;
	st.st_uid = (uid_t)filestat->uid;
	st.st_gid = (gid_t)filestat->gid;
	st.st_size = (off_t) filestat->filesize;
	st.st_blocks = 0;
	st.st_blksize = 0;
	gsimplecache_add_item_kstr(cache, fullurl, (void*) &st);
	dir->d_off +=1;
	g_strlcpy(dir->d_name, filestat->d_name, NAME_MAX);
	return dir;
}

/**
 * Execute a readdir func on the lfc
 * */
static struct dirent* lfc_readdirG(catalog_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail( handle && fh , NULL, err, "[lfc_rmdirG] Invalid value in args handle/path");
	GError* tmp_err=NULL;	
	struct lfc_ops *ops = (struct lfc_ops*) handle;
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);
	lfc_opendir_handle oh = (lfc_opendir_handle )fh->ext_data;
	struct dirent* ret=  lfc_convert_dirent_struct(ops, ((struct dirent*) &oh->current_dir), (ops->readdirx( (lfc_DIR*)fh->fdesc)), oh->url);
	if(ret ==NULL && *ops->serrno ){
		int sav_errno = gfal_lfc_get_errno(ops);
		g_set_error(err,0, sav_errno, "[%s] Error report from LFC %s", __func__, gfal_lfc_get_strerror(ops) );
	}
	return ret;
}

/**
 * execute an closedir func on the lfc
 * */
static int lfc_closedirG(catalog_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail( handle && fh , -1, err, "[lfc_rmdirG] Invalid value in args handle/path");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	gfal_lfc_init_thread(ops);
	int ret = ops->closedir(fh->fdesc);	
	if(ret != 0){
		int sav_errno = gfal_lfc_get_errno(ops);
		g_set_error(err,0, sav_errno, "[%s] Error report from LFC %s", __func__, gfal_lfc_get_strerror(ops) );
	}else{
		free(fh->ext_data);
		free(fh);
	}
	return ret;		
}

/**
 * resolve the lfc link to the surls
 */
char ** lfc_getSURLG(catalog_handle handle, const char * path, GError** err){
	g_return_val_err_if_fail( handle && path , NULL, err, "[lfc_getSURLG] Invalid value in args handle/path");
	GError* tmp_err=NULL;
	char** resu = NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	gfal_lfc_init_thread(ops);
	char * lfn = url_converter(handle, path, &tmp_err);
	if(lfn)
		resu = gfal_lfc_getSURL(ops, lfn, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	free(lfn);
	return resu;
}

/**
 * lfc getxattr for the path -> surls resolution
 * */
ssize_t lfc_getxattr_getsurl(catalog_handle handle, const char* path, void* buff, size_t size, GError** err){
	GError* tmp_err=NULL;
	ssize_t res = -1;

	char** tmp_ret = lfc_getSURLG(handle, path, &tmp_err);
	if(tmp_ret != NULL){
		res = g_strv_catbuff(tmp_ret, buff, size);
		g_strfreev(tmp_ret);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);	
	return res;
}


/**
 * lfc getxattr for the path -> guid resolution
 * */
ssize_t lfc_getxattr_getguid(catalog_handle handle, const char* path, void* buff, size_t size, GError** err){
	GError* tmp_err=NULL;
	ssize_t res = -1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	if(size == 0 || buff ==NULL){ // just return the size of a guid
		res = sizeof(char) * 36; // strng uuid are 36 bytes long
	}else{
		char* lfn = url_converter(handle, path, &tmp_err);
		if(lfn){
			struct lfc_filestatg statbuf;
			int tmp_ret = gfal_lfc_statg(ops, lfn, &statbuf, &tmp_err);
			if(tmp_ret == 0){
				res = strnlen(statbuf.guid, GFAL_URL_MAX_LEN);
				g_strlcpy(buff,statbuf.guid, size);
			}
			free(lfn);
		}
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return res;
}

/**
 * lfc getxattr implem 
 * */
ssize_t lfc_getxattrG(catalog_handle handle, const char* path, const char* name, void* buff, size_t size, GError** err){
	GError* tmp_err=NULL;
	ssize_t res = -1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);
	if( strncmp(name, LFC_XATTR_GUID, LFC_MAX_XATTR_LEN) == 0){
		res = lfc_getxattr_getguid(handle, path, buff, size, &tmp_err );
	}else if(strncmp(name, LFC_XATTR_SURLS, LFC_MAX_XATTR_LEN) == 0){
		res = lfc_getxattr_getsurl(handle, path, buff, size, &tmp_err);
	}else{
		g_set_error(&tmp_err,0, ENOATTR, "axttr not found");
		res = -1;
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return res;
}

/**
 * lfc getxattr implem 
 * */
ssize_t lfc_listxattrG(catalog_handle handle, const char* path, char* list, size_t size, GError** err){
	GError* tmp_err=NULL;
	ssize_t res = 0;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	char** p= file_xattr;
	char* plist= list;
	while(*p != NULL){
		const int size_str = strlen(*p)+1;
		if( size > res && size - res >= size_str)
			plist = mempcpy(plist, *p, size_str* sizeof(char) );
		res += size_str;
		p++;
	}
	return res;
}

/**
 * Convert a guid to a catalog url if possible
 *  return the link in a catalog's url string or err and NULL if not found
 */
char* lfc_resolve_guid(catalog_handle handle, const char* guid, GError** err){
	g_return_val_err_if_fail( handle && guid, NULL, err, "[lfc_resolve_guid] Invalid args in handle and/or guid ");
	char* tmp_guid = lfc_urlconverter(guid, GFAL_LFC_GUID_PREFIX);
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

static int lfc_unlinkG(catalog_handle handle, const char* path, GError** err){
	g_return_val_err_if_fail(path, -1, err, "[lfc_unlink] Invalid value in args handle/path/stat");	
	GError* tmp_err=NULL;
	char buff_key[GFAL_URL_MAX_LEN];
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	int ret = -1;
	char* lfn = url_converter(handle, path, &tmp_err);
	if(lfn){
		ret = ops->unlink(lfn);
		if(ret != 0){
			int sav_errno = gfal_lfc_get_errno(ops);
			g_set_error(&tmp_err,0,sav_errno, "Error report from LFC : %s", gfal_lfc_get_strerror(ops) );
		}else{
			gsimplecache_remove_kstr(ops->cache_stat, lfn);	// remove the key associated in the buffer	
			errno=0;
		}
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	free(lfn);
	return ret;
}
  
/**
 * execute a posix readlink request on the lfc 
 *  return size of the buffer if success and set the struct buf else return negative value and set GError
 */
static ssize_t lfc_readlinkG(catalog_handle handle, const char* path, char* buff, size_t buffsiz, GError** err){
	g_return_val_err_if_fail(handle && path && buff, -1, err, "[lfc_readlinkG] Invalid value in args handle/path/stat");
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	GError* tmp_err=NULL;
	char res_buff[LFC_BUFF_SIZE];
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);	
	char* lfn = lfc_urlconverter(path, GFAL_LFC_PREFIX);

	ssize_t ret = ops->readlink(lfn, res_buff, LFC_BUFF_SIZE );
	if(ret == -1){
		int sav_errno = gfal_lfc_get_errno(ops);
		g_set_error(err,0,sav_errno, "Error report from LFC : %s", __func__, gfal_lfc_get_strerror(ops) );
	}else{
		errno=0;
		if(buffsiz > 0)
			memcpy(buff, GFAL_LFC_PREFIX, MIN(buffsiz,GFAL_LFC_PREFIX_LEN) );
		if(buffsiz - GFAL_LFC_PREFIX_LEN > 0)
			memcpy(buff+ GFAL_LFC_PREFIX_LEN, res_buff, MIN(ret,buffsiz-GFAL_LFC_PREFIX_LEN) );
		ret += GFAL_LFC_PREFIX_LEN;
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);	
	free(lfn);
	return ret;
}

static void internal_stat_copy(gpointer original, gpointer copy){
	memcpy(copy, original, sizeof(struct stat));
}

/**
 * Map function for the lfc interface
 * this function provide the generic CATALOG interface for the LFC catalog.
 * lfc_initG do : liblfc shared library load, sym resolve, endpoint check, and catalog function map.
 * 
 * */
gfal_catalog_interface gfal_plugin_init(gfal_handle handle, GError** err){
	pthread_mutex_lock(&m_lfcinit);
	gfal_catalog_interface lfc_catalog;
	GError* tmp_err=NULL;
	memset(&lfc_catalog,0,sizeof(gfal_catalog_interface));	// clear the catalog
	
	char* endpoint = gfal_setup_lfchost(handle, &tmp_err); // load the endpoint
	if(endpoint==NULL){
		g_propagate_prefixed_error(err, tmp_err, "[lfc_initG]");
		pthread_mutex_unlock(&m_lfcinit);
		return lfc_catalog;
	}
	
	struct lfc_ops* ops = gfal_load_lfc(GFAL_LFC_LIBRARY_NAME, &tmp_err); // load library
	if(ops ==NULL){
		g_propagate_prefixed_error(err, tmp_err,"[%s]", __func__);
		pthread_mutex_unlock(&m_lfcinit);
		return lfc_catalog;
	}
	ops->lfc_endpoint = endpoint;
	ops->handle = handle;
	ops->cache_stat = gsimplecache_new(50000000,&internal_stat_copy, sizeof(struct stat) );
	gfal_lfc_regex_compile(&(ops->rex), err);
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
	lfc_catalog.rmdirG = &lfc_rmdirG;
	lfc_catalog.opendirG = &lfc_opendirG;
	lfc_catalog.closedirG = &lfc_closedirG;
	lfc_catalog.readdirG = &lfc_readdirG;
	lfc_catalog.getSURLG = &lfc_getSURLG;
	lfc_catalog.getName = &lfc_getName;
	lfc_catalog.openG = &lfc_openG;
	lfc_catalog.symlinkG= &lfc_symlinkG;
	lfc_catalog.getxattrG= &lfc_getxattrG;
	lfc_catalog.listxattrG = &lfc_listxattrG;
	lfc_catalog.readlinkG = &lfc_readlinkG;
	lfc_catalog.unlinkG = &lfc_unlinkG;
	
	if(init_thread== FALSE){ // initiate Cthread system
		ops->Cthread_init();	// must be called one time for DPM thread safety	
		init_thread = TRUE;
	}
	gfal_lfc_init_thread(ops);	
	pthread_mutex_unlock(&m_lfcinit);
	return lfc_catalog;
}


 /**
 * parse a guid to check the validity
 */
gboolean gfal_checker_guid(const char* guid, GError** err){
	g_return_val_err_if_fail(guid != NULL,FALSE,err,"[gfal_checker_guid] check URL failed : guid is empty");
	const size_t sguid = strnlen(guid, GFAL_URL_MAX_LEN);
	return ( sguid < GFAL_URL_MAX_LEN && sguid > 5 && strncmp(guid, "guid:",5)== 0);
}  

/**
 * Check if the passed url and operation is compatible with lfc
 * 
 * */
 gboolean gfal_lfc_check_lfn_url(catalog_handle handle, const char* url, catalog_mode mode, GError** err){
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	int ret;
	switch(mode){
		case GFAL_CATALOG_RESOLVE_GUID:
			return TRUE;	
			
		case GFAL_CATALOG_ACCESS:
		case GFAL_CATALOG_CHMOD:	
		case GFAL_CATALOG_STAT:
		case GFAL_CATALOG_LSTAT:		
		case GFAL_CATALOG_OPEN:
		case GFAL_CATALOG_GETXATTR:
		case GFAL_CATALOG_LISTXATTR:
		case GFAL_CATALOG_UNLINK:
			ret= regexec(&(ops->rex), url, 0, NULL, 0);
			return (!ret || gfal_checker_guid(url, err ))?TRUE:FALSE;	

		case GFAL_CATALOG_RENAME:
		case GFAL_CATALOG_MKDIR:
		case GFAL_CATALOG_RMDIR:
		case GFAL_CATALOG_OPENDIR:

		case GFAL_CATALOG_GETSURL:
		case GFAL_CATALOG_SYMLINK:
		case GFAL_CATALOG_READLINK:
			ret= regexec(&(ops->rex), url, 0, NULL, 0);
			return (!ret)?TRUE:FALSE;	
		default:
			return FALSE;
	}
}
 

