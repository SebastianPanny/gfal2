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
 * @file gfal_posix_local_file.c
 * @brief file for the local access file map for the gfal_posix call
 * @author Devresse Adrien
 * @version 2.0
 * @date 06/05/2011
 * */
#include <unistd.h>
#include <glib.h>
#include <sys/stat.h>
#include <regex.h>
#include "../common/gfal_common_errverbose.h"
#include "../common/gfal_common_filedescriptor.h"
#include "../common/gfal_types.h"

#define GFAL_LOCAL_PREFIX "file:"


void gfal_local_report_error(const char* funcname, GError** err){
	g_set_error(err,0,errno, "[%s] errno reported by local system call %s", funcname, strerror(errno));	
}

 int gfal_local_access(const char *path, int amode, GError** err){
	const int res = access(path+strlen(GFAL_LOCAL_PREFIX), amode);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}
	return res;
 }
 
int gfal_local_chmod(const char* path, mode_t mode,GError** err){
	const int res = chmod(path+strlen(GFAL_LOCAL_PREFIX),mode);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}
	return res;
}


int gfal_local_rename(const char* oldpath, const char* newpath, GError** err){
	const int res = rename(oldpath+strlen(GFAL_LOCAL_PREFIX), newpath + strlen(GFAL_LOCAL_PREFIX));
	if(res <0){
		gfal_local_report_error(__func__, err);
	}
	return res;
 }
 
int gfal_local_stat(const char* path, struct stat* buf, GError ** err){
	const int res = stat(path + strlen(GFAL_LOCAL_PREFIX) , buf);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}
	return res;
} 

int gfal_local_lstat(const char* path, struct stat* buf, GError ** err){
	const int res = lstat(path + strlen(GFAL_LOCAL_PREFIX), buf);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}
	return res;
}

int gfal_local_mkdir_rec(const char* full_path, mode_t mode){
	char *p;
	int res = -1, i;
	
	if( (res = mkdir(full_path, mode)) ==0 ||  errno != ENOENT )// try to create without recursive mode
		return res;	

	errno =0;
	size_t len = strnlen(full_path, GFAL_URL_MAX_LEN);
	char buff[len+1];
	
	i=0;
	p = (char*) full_path;
	while((p-full_path) < len){ // remove '/{2,+}' and last char if =='/'
		if( ( *p == '/' && ( *(p+1) == '/' || *(p+1) == '\0')) == FALSE)
			buff[i++] = *p;
		++p;
	}
	buff[i] = '\0';
				
	for(p = buff+1 ; *p != '\0'; p++){ // begin the recursive mode
		if(*p == '/' && *(p+1) != '/') { // check the '/' but skip the '//////' sequencies'
			*p = '\0';
			if( ((res =mkdir(buff, ( 0700 | mode) )) !=0) && errno != EEXIST && errno != EACCES)
				return res;
			*p = '/';
			errno =0;
		}
	}
	return mkdir(buff, mode);
}

int gfal_local_mkdir(const char* path, mode_t mode, GError** err){
	const int res = gfal_local_mkdir_rec(path + strlen(GFAL_LOCAL_PREFIX), mode);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}
	return res;	
}

gfal_file_handle gfal_local_opendir(const char* path, GError** err){
	DIR* ret = opendir(path+strlen(GFAL_LOCAL_PREFIX));
	gfal_file_handle resu = NULL;
	if(ret == NULL){
		gfal_local_report_error(__func__, err);
	}
	if(ret)
		resu = gfal_file_handle_new(GFAL_MODULEID_LOCAL, (gpointer) ret);
	return resu;
}

struct dirent* gfal_local_readdir(DIR* d, GError** err){
	errno=0;
	struct dirent* res = readdir(d);
	if(res== NULL && errno){
		gfal_local_report_error(__func__, err);
	}
	return res;	 	
}

gfal_file_handle gfal_local_open(const char* path, int flag, mode_t mode, GError** err){
	errno =0;
	const int ret = open(path + strlen(GFAL_LOCAL_PREFIX), flag, mode);
	if(ret <=0){
		gfal_local_report_error(__func__, err);	
		return NULL;	
	}else{
		return gfal_file_handle_new(GFAL_MODULEID_LOCAL, GINT_TO_POINTER(ret));
	}
}

int gfal_local_read(gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
	errno=0;
	const int ret = read(GPOINTER_TO_INT(fh->fdesc), buff, s_buff);
	if(ret <0)
		gfal_local_report_error(__func__, err);
	return ret;
}

int gfal_local_write(gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
	errno=0;
	const int ret = write(GPOINTER_TO_INT(fh->fdesc), buff, s_buff);
	if(ret <0)
		gfal_local_report_error(__func__, err);
	return ret;
}

int gfal_local_close(gfal_file_handle fh, GError** err){
	errno =0;
	const int ret = close(GPOINTER_TO_INT(fh->fdesc));
	if(ret !=0){
		gfal_local_report_error(__func__, err);		
	}
	return ret;
}


/**
 * local rmdir mapper
 * */
int gfal_local_rmdir(const char* path, GError** err){
	const int res = rmdir(path+ strlen(GFAL_LOCAL_PREFIX));
	if(res<0){
			g_set_error(err,0 ,errno , "[%s] errno reported by local system call %s", __func__, strerror(errno));
	}
	return res;
}

/**
 *  local closedir mapper
 * 
 * */
int gfal_local_closedir(DIR* d, GError** err){
	const int res = closedir(d);
	if(res<0){
			g_set_error(err,0 ,errno , "[%s] errno reported by local system call", __func__, strerror(errno));
	}
	return res;	 
 }
 
/**
 * check the validity of a classique file url
 * */ 
gboolean gfal_check_local_url(const char* path, GError** err){
	regex_t rex;
	int ret = regcomp(&rex, "^file:([:print:]|/)+",REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret==0,-1,err,"[gfal_check_local_url] fail to compile regex, report this bug");
	ret=  regexec(&rex, path,0,NULL,0);
	regfree(&rex);
	return (!ret)?TRUE:FALSE;		
}
