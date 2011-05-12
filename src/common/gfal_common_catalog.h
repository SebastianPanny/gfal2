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
 // protos
#include "gfal_prototypes.h"
#define MAX_CATALOG_LIST 64






/**
 * @enum list the type of the check associated with the url
 *  GFAL_CATALOG_ALL : general check, if this url is associated with this catalog
 *  GFAL_CATALOG_ACCESS : check for a access request, check if this url is a correct url for a access request
 *  GFAL_CATALOG_CHMOD : check for a chmod request, check if this url is correct for a chmod request
 * */
enum _catalog_mode{
	GFAL_CATALOG_ALL=0,
	GFAL_CATALOG_ACCESS,
	GFAL_CATALOG_CHMOD,
	GFAL_CATALOG_RENAME
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
	int (*accessG)(catalog_handle, char* path, int mode, GError** err);
	/**
	 *  function pointer for the access using the guid url
	 * */
	int	(*chmodG)(catalog_handle, const char *, mode_t, GError** err);
	int	(*renameG)(catalog_handle, const char *, const char *, GError** err);
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

int gfal_catalogs_accessG(gfal_handle handle, char* path, int mode, GError** err);

int gfal_catalogs_guid_accessG(gfal_handle handle, char* guid, int mode, GError** err);

int gfal_catalogs_delete(gfal_handle, GError** err);

//char* gfal_catalog_resolve_guid(gfal_handle handle, const char* guid, GError** err);
 	

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
