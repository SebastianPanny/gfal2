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
 * @file gfal_common_plugin.h
 * @brief the header file of the common lib for the plugin management
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
#define MAX_PLUGIN_LIST 64






/**
 * @enum list the type of the check associated with the url
 *  check_plugin_url send this mode to the plugin to know is this type of operation on it
 * */
enum _plugin_mode{
	GFAL_PLUGIN_ALL=0,
	GFAL_PLUGIN_ACCESS,
	GFAL_PLUGIN_CHMOD,
	GFAL_PLUGIN_RENAME,
	GFAL_PLUGIN_SYMLINK,
	GFAL_PLUGIN_STAT,
	GFAL_PLUGIN_LSTAT,
	GFAL_PLUGIN_MKDIR,
	GFAL_PLUGIN_RMDIR,
	GFAL_PLUGIN_OPENDIR, // concat of opendir readdir, closedir
	GFAL_PLUGIN_OPEN, // concat of open read, close
	GFAL_PLUGIN_RESOLVE_GUID,
	GFAL_PLUGIN_GETXATTR,
	GFAL_PLUGIN_SETXATTR,
	GFAL_PLUGIN_LISTXATTR,
	GFAL_PLUGIN_READLINK,
	GFAL_PLUGIN_UNLINK
	
};

/**
 * @struct _gfal_plugin_interface 
 * 
 *  Interface to implement in each gfal_plugin_*
 *  the minimums calls are : getName, plugin_delete, check_plugin_url
 *  all the unused function pointers must be set to NULL
 */
struct _gfal_plugin_interface{
	// handle
	plugin_handle handle;
	// to have name/id of the plugin MUST be IMPLEMENTED
	const char* (*getName)();
	/**
	 * Called before the destruction of the plugin interface
	 */
	void (*plugin_delete)(plugin_handle); 
	/**
	 *  Check the url syntax and the operation, return TRUE if the operation is compatible with the plugin, else FALSE.
	 *  THIS function MUST be implemented in EACH plugin.
	 *  @warning This function is a key function of GFAL 2.0, It MUST be as fast as possible.
	 */
	gboolean (*check_plugin_url)(plugin_handle, const char* url,  plugin_mode mode, GError** err);
	/**
	 *  access function for the access for the normal associated url
	 * */
	int (*accessG)(plugin_handle, const char* path, int mode, GError** err);
	/**
	 *  function pointer for the access using the guid url
	 * */
	int	(*chmodG)(plugin_handle, const char *, mode_t, GError** err);
	int	(*renameG)(plugin_handle, const char *, const char *, GError** err);
	int (*symlinkG)(plugin_handle, const char*, const char*, GError** err);
	int (*statG)(plugin_handle, const char*, struct stat *buf, GError** err);
	int (*lstatG)(plugin_handle, const char*, struct stat *buf, GError** err);
	ssize_t (*readlinkG)(plugin_handle, const char*, char* buff, size_t buffsiz, GError** );

	// directory management
	 gfal_file_handle (*opendirG)(plugin_handle, const char*, GError**); 
	 int (*closedirG)(plugin_handle, gfal_file_handle, GError**);
	 struct dirent* (*readdirG)(plugin_handle, gfal_file_handle, GError**);
	int (*mkdirpG)(plugin_handle, const char*, mode_t, gboolean pflag, GError**);  //function pointer for the mkdir call, if pflag is set, the call is considered like a recursive call for a full path creation
	int (*rmdirG )(plugin_handle, const char*, GError**);	 	 

	 // basic file operation, 
	 gfal_file_handle (*openG)(plugin_handle, const char* path, int flag, mode_t mode, GError**);
	 int (*readG)(plugin_handle, gfal_file_handle fd, void* buff, size_t count, GError**);
	 int (*writeG)(plugin_handle, gfal_file_handle fd, void* buff, size_t count, GError**);
	 int (*closeG)(plugin_handle, gfal_file_handle fd, GError **);
	 int (*lseekG)(plugin_handle, gfal_file_handle fd, off_t offset, int whence, GError** err);
	 
	 // remove operations
	 int (*unlinkG)(plugin_handle, const char* path, GError**);
	 
	 // advanced attributes management
	 ssize_t (*getxattrG)(plugin_handle, const char*, const char*, void* buff, size_t s_buff, GError** err);
	 ssize_t (*listxattrG)(plugin_handle, const char*, char* list, size_t s_list, GError** err);
	 int (*setxattrG)(plugin_handle, const char*, const char*, const void*, size_t , int, GError** );
	 
	
};

/**
 * container for the plugin abstraction couch parameters
 * */
struct _plugin_opts{
	gfal_plugin_interface plugin_list[MAX_PLUGIN_LIST];
	int plugin_number;
};

#include "gfal_types.h"


extern char* gfal_get_cat_type(GError**);


inline int gfal_plugins_instance(gfal_handle, GError** err);
char** gfal_plugins_get_list(gfal_handle, GError** err);
int gfal_plugins_delete(gfal_handle, GError** err);


int gfal_plugins_accessG(gfal_handle handle, const char* path, int mode, GError** err);
int gfal_plugin_rmdirG(gfal_handle handle, const char* path, GError** err);
ssize_t gfal_plugin_readlinkG(gfal_handle handle, const char* path, char* buff, size_t buffsiz, GError** err);




int gfal_plugin_chmodG(gfal_handle handle, const char* path, mode_t mode, GError** err);
int gfal_plugin_statG(gfal_handle handle,const char* path, struct stat* st, GError** err);
int gfal_plugin_renameG(gfal_handle handle, const char* oldpath, const char* newpath, GError** err);
int gfal_plugin_symlinkG(gfal_handle handle, const char* oldpath, const char* newpath, GError** err);
int gfal_plugin_lstatG(gfal_handle handle,const char* path, struct stat* st, GError** err);
int gfal_plugin_mkdirp(gfal_handle handle, const char* path, mode_t mode, gboolean pflag,  GError** err);


gfal_file_handle gfal_plugin_opendirG(gfal_handle handle, const char* name, GError** err);
int gfal_plugin_closedirG(gfal_handle handle, gfal_file_handle fh, GError** err);
struct dirent* gfal_plugin_readdirG(gfal_handle handle, gfal_file_handle fh, GError** err);
 	

gfal_file_handle gfal_plugin_openG(gfal_handle handle, const char * path, int flag, mode_t mode, GError ** err);
int gfal_plugin_closeG(gfal_handle handle, gfal_file_handle fh, GError** err);
int gfal_plugin_writeG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err);
int gfal_plugin_lseekG(gfal_handle handle, gfal_file_handle fh, off_t offset, int whence, GError** err);
int gfal_plugin_readG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err);


int gfal_plugin_unlinkG(gfal_handle handle, const char* path, GError** err);


ssize_t gfal_plugin_getxattrG(gfal_handle, const char*, const char*, void* buff, size_t s_buff, GError** err);
ssize_t gfal_plugin_listxattrG(gfal_handle, const char*, char* list, size_t s_list, GError** err);
int gfal_plugin_setxattrG(gfal_handle, const char*, const char*, const void*, size_t, int, GError**);





