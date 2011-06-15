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
	GFAL_CATALOG_STAT,
	GFAL_CATALOG_LSTAT,
	GFAL_CATALOG_MKDIR,
	GFAL_CATALOG_RMDIR,
	GFAL_CATALOG_OPENDIR, // concat of opendir readdir, closedir
	GFAL_CATALOG_OPEN,
	GFAL_CATALOG_GETSURL,
	GFAL_CATALOG_GETTURL
	
};

/**
 * @struct Object for a catalog type, modelise all operation that can be done by the catalog
 */
struct _gfal_catalog_interface{
	// handle
	catalog_handle handle;
	// delete
	/**
	 * Called before the destruction of the catalog interface
	 */
	void (*catalog_delete)(catalog_handle); 
	/**
	 *  must check if the url and the operation are comaptible with this type of catalog 
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
	int (*statG)(catalog_handle, const char*, struct stat *buf, GError** err);
	int (*lstatG)(catalog_handle, const char*, struct stat *buf, GError** err);
	/**
	 * function pointer for the mkdir call, if pflag is set, the call is considered like a recursive call for a full path creation
	 * */
	int (*mkdirpG)(catalog_handle, const char*, mode_t, gboolean pflag, GError**);
	int (*rmdirG )(catalog_handle, const char*, GError**);

	 DIR* (*opendirG)(catalog_handle, const char*, GError**); 
	 int (*closedirG)(catalog_handle, DIR*, GError**);
	 struct dirent* (*readdirG)(catalog_handle, DIR*, GError**);

	 int (*openG)(catalog_handle, const char* path, int flag, mode_t mode, GError**);
	 int (*closeG)(catalog_handle, int fd, GError **);
	 
	 char** (*getSURLG)(catalog_handle, const char*, GError**);
	 
	 int (*getTURLG)(catalog_handle, const char* surl, char* buff_turl, int size_turl, GError** err);
	/**
	 * return a valid url if is able to resolve the guid or return NULL pointer
	 */
	char* (*resolve_guid)(catalog_handle handle, const char* guid, GError** err);	
	
};

/**
 * container for the catalog abstraction couch parameters
 * */
struct _catalog_opts{
	gfal_catalog_interface catalog_list[MAX_CATALOG_LIST];
	int catalog_number;
};

#include "gfal_types.h"

/**
	\brief catalog type getter
	
	@return return a string of the type of the catalog
	 return NULL if an error occured and set the GError correctly
*/
extern char* gfal_get_cat_type(GError**);


int gfal_catalogs_instance(gfal_handle, GError** err);

int gfal_catalogs_accessG(gfal_handle handle, const char* path, int mode, GError** err);

int gfal_catalogs_guid_accessG(gfal_handle handle, char* guid, int mode, GError** err);

int gfal_catalogs_delete(gfal_handle, GError** err);

int gfal_catalog_statG(gfal_handle handle,const char* path, struct stat* st, GError** err);

int gfal_catalog_lstatG(gfal_handle handle,const char* path, struct stat* st, GError** err);

int gfal_catalog_mkdirp(gfal_handle handle, const char* path, mode_t mode, gboolean pflag,  GError** err);



char* gfal_catalog_resolve_guid(gfal_handle handle, const char* guid, GError** err);

gfal_file_handle gfal_catalog_opendirG(gfal_handle handle, const char* name, GError** err);

int gfal_catalog_closedirG(gfal_handle handle, gfal_file_handle fh, GError** err);

gfal_file_handle gfal_catalog_openG(gfal_handle handle, const char * path, int flag, mode_t mode, GError ** err);

gfal_file_handle gfal_catalog_open_globalG(gfal_handle handle, const char * path, int flag, mode_t mode, GError** err);

int gfal_catalog_closeG(gfal_handle handle, gfal_file_handle fh, GError** err);

char** gfal_catalog_getSURL(gfal_handle handle, const char* path, GError** err);

int getTURLG(catalog_handle, const char* surl, char* buff_turl, int size_turl, GError** err);


struct dirent* gfal_catalog_readdirG(gfal_handle handle, gfal_file_handle fh, GError** err);
 	

char *get_default_se(char *, int);
int purify_surl (const char *, char *, const int);
int setypesandendpointsfromsurl (const char *, char ***, char ***, char *, int);
int setypesandendpoints (const char *, char ***, char ***, char *, int);
int canonical_url (const char *, const char *, char *, int, char *, int);
int parseturl (const char *, char *, int, char *, int, char*, int);
int replica_exists(const char*, char*, int);
int getdomainnm (char *name, int namelen);
char **get_sup_proto ();
struct proto_ops *find_pops (const char *);
int mapposixerror (struct proto_ops *, int);
