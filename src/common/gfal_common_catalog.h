#pragma once
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
 * @file gfal_common_catalog.h
 * @brief the header file of the common lib for the catalog management
 * @author Devresse Adrien
 * @version 0.0.1
 * @date 8/04/2011
 * */
 
 
 
#include <stdarg.h>
#include <uuid/uuid.h>
#include <glib.h>
#include <errno.h>
#include <string.h> 
#include <dirent.h>
#include <sys/stat.h>
 // protos
#include "gfal_prototypes.h"
#define MAX_CATALOG_LIST 64






/**
 * @enum list the type of the check associated with the url
 *  check_catalog_url send this mode to the catalog to know is this type of operation on it
 * */
enum _catalog_mode{
	GFAL_CATALOG_ALL=0,
	GFAL_CATALOG_ACCESS,
	GFAL_CATALOG_CHMOD,
	GFAL_CATALOG_RENAME,
	GFAL_CATALOG_SYMLINK,
	GFAL_CATALOG_STAT,
	GFAL_CATALOG_LSTAT,
	GFAL_CATALOG_MKDIR,
	GFAL_CATALOG_RMDIR,
	GFAL_CATALOG_OPENDIR, // concat of opendir readdir, closedir
	GFAL_CATALOG_OPEN, // concat of open read, close
	GFAL_CATALOG_GETSURL,
	GFAL_CATALOG_GETTURL,
	GFAL_CATALOG_PUTTURL,
	GFAL_CATALOG_RESOLVE_GUID,
	GFAL_CATALOG_GETXATTR,
	GFAL_CATALOG_LISTXATTR,
	GFAL_CATALOG_READLINK,
	GFAL_CATALOG_UNLINK
	
};

/**
 * @struct _gfal_catalog_interface 
 * 
 *  Interface to implement in each gfal_plugin_*
 *  the needed calls are : getName, catalog_delete, check_catalog_url
 */
struct _gfal_catalog_interface{
	// handle
	catalog_handle handle;
	// to have name/id of the catalog MUST be IMPLEMENTED
	const char* (*getName)();
	/**
	 * Called before the destruction of the catalog interface
	 */
	void (*catalog_delete)(catalog_handle); 
	/**
	 *  Check the url syntax and the operation, return TRUE if the operation is compatible with the plugin, else FALSE.
	 *  THIS function MUST be implemented in EACH plugin.
	 *  @warning This function is a key function of GFAL 2.0, It MUST be as fast as possible.
	 */
	gboolean (*check_catalog_url)(catalog_handle, const char* url,  catalog_mode mode, GError** err);
	/**
	 *  access function for the access for the normal associated url
	 * */
	int (*accessG)(catalog_handle, const char* path, int mode, GError** err);
	/**
	 *  function pointer for the access using the guid url
	 * */
	int	(*chmodG)(catalog_handle, const char *, mode_t, GError** err);
	int	(*renameG)(catalog_handle, const char *, const char *, GError** err);
	int (*symlinkG)(catalog_handle, const char*, const char*, GError** err);
	int (*statG)(catalog_handle, const char*, struct stat *buf, GError** err);
	int (*lstatG)(catalog_handle, const char*, struct stat *buf, GError** err);
	ssize_t (*readlinkG)(catalog_handle, const char*, char* buff, size_t buffsiz, GError** );

	// directory management
	 gfal_file_handle (*opendirG)(catalog_handle, const char*, GError**); 
	 int (*closedirG)(catalog_handle, gfal_file_handle, GError**);
	 struct dirent* (*readdirG)(catalog_handle, gfal_file_handle, GError**);
	int (*mkdirpG)(catalog_handle, const char*, mode_t, gboolean pflag, GError**);  //function pointer for the mkdir call, if pflag is set, the call is considered like a recursive call for a full path creation
	int (*rmdirG )(catalog_handle, const char*, GError**);	 	 

	 // basic file operation, 
	 gfal_file_handle (*openG)(catalog_handle, const char* path, int flag, mode_t mode, GError**);
	 int (*readG)(catalog_handle, gfal_file_handle fd, void* buff, size_t count, GError**);
	 int (*writeG)(catalog_handle, gfal_file_handle fd, void* buff, size_t count, GError**);
	 int (*closeG)(catalog_handle, gfal_file_handle fd, GError **);
	 int (*lseekG)(catalog_handle, gfal_file_handle fd, off_t offset, int whence, GError** err);
	 
	 // remove operations
	 int (*unlinkG)(catalog_handle, const char* path, GError**);
	 
	 // advanced attributes management
	 ssize_t (*getxattrG)(catalog_handle, const char*, const char*, void* buff, size_t s_buff, GError** err);
	 ssize_t (*listxattrG)(catalog_handle, const char*, char* list, size_t s_list, GError** err);
	 
	 char** (*getSURLG)(catalog_handle, const char*, GError**);	 
	 int (*getTURLG)(catalog_handle, const char* surl, char* buff_turl, int size_turl, char** reqtoken, GError** err);
	 int (*putTURLG)(catalog_handle, const char* surl, char* buff_turl, int size_turl, char** reqtoken, GError** err);
	
};

/**
 * container for the catalog abstraction couch parameters
 * */
struct _catalog_opts{
	gfal_catalog_interface catalog_list[MAX_CATALOG_LIST];
	int catalog_number;
};

#include "gfal_types.h"


extern char* gfal_get_cat_type(GError**);


inline int gfal_catalogs_instance(gfal_handle, GError** err);
char** gfal_catalogs_get_list(gfal_handle, GError** err);
int gfal_catalogs_delete(gfal_handle, GError** err);


int gfal_catalogs_accessG(gfal_handle handle, const char* path, int mode, GError** err);






int gfal_catalog_statG(gfal_handle handle,const char* path, struct stat* st, GError** err);
int gfal_catalog_renameG(gfal_handle handle, const char* oldpath, const char* newpath, GError** err);
int gfal_catalog_symlinkG(gfal_handle handle, const char* oldpath, const char* newpath, GError** err);
int gfal_catalog_lstatG(gfal_handle handle,const char* path, struct stat* st, GError** err);
int gfal_catalog_mkdirp(gfal_handle handle, const char* path, mode_t mode, gboolean pflag,  GError** err);


gfal_file_handle gfal_catalog_opendirG(gfal_handle handle, const char* name, GError** err);
int gfal_catalog_closedirG(gfal_handle handle, gfal_file_handle fh, GError** err);
struct dirent* gfal_catalog_readdirG(gfal_handle handle, gfal_file_handle fh, GError** err);
 	

gfal_file_handle gfal_catalog_openG(gfal_handle handle, const char * path, int flag, mode_t mode, GError ** err);
int gfal_catalog_closeG(gfal_handle handle, gfal_file_handle fh, GError** err);
int gfal_catalog_writeG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err);
int gfal_catalog_lseekG(gfal_handle handle, gfal_file_handle fh, off_t offset, int whence, GError** err);
int gfal_catalog_readG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err);


int gfal_catalog_unlinkG(catalog_handle handle, const char* path, GError** err);


ssize_t gfal_catalog_getxattrG(gfal_handle, const char*, const char*, void* buff, size_t s_buff, GError** err);
ssize_t gfal_catalog_listxattrG(gfal_handle , const char*, char* list, size_t s_list, GError** err);

char** gfal_catalog_getSURL(gfal_handle handle, const char* path, GError** err);
int gfal_catalog_getTURLG(gfal_handle handle, const char* surl, char* buff_turl, int size_turl, char** reqtoken, GError** err);
int gfal_catalog_putTURLG(gfal_handle handle, const char* surl, char* turl_buff, int turl, char** reqtoken, GError** err);



