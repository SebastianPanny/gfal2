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

#define GFAL_LOCAL_PREFIX "file:"

 int gfal_local_access(const char *path, int amode, GError** err){
	const int res = access(path+strlen(GFAL_LOCAL_PREFIX), amode);
	if(res <0){
		g_set_error(err,0,errno, "[%s] errno reported by local system call", __func__, strerror(errno));
	}
	return res;
 }
 
int gfal_local_chmod(const char* path, mode_t mode,GError** err){
	const int res = chmod(path+strlen(GFAL_LOCAL_PREFIX),mode);
	if(res <0){
		g_set_error(err,0,errno, "[%s] errno reported by local system call", __func__, strerror(errno));
	}
	return res;
}


int gfal_local_rename(const char* oldpath, const char* newpath, GError** err){
	const int res = rename(oldpath+strlen(GFAL_LOCAL_PREFIX), newpath + strlen(GFAL_LOCAL_PREFIX));
	if(res <0){
		g_set_error(err, 0, errno, "[%s] errno reported by local system call ", __func__, strerror(errno));
	}
	return res;
 }
 
int gfal_local_stat(const char* path, struct stat* buf, GError ** err){
	const int res = stat(path + strlen(GFAL_LOCAL_PREFIX) , buf);
	if(res <0){
		g_set_error(err, 0, errno, "[%s] errno reported by local system call", __func__, strerror(errno));
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
	return (!ret)?TRUE:FALSE;		
}
