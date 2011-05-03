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

#define MAX_CATALOG_LIST 64



#include <glib.h>
#include <errno.h>
#include <string.h>
#include "gfal_types.h"	
#include "lfc/lfc_ifce.h"
#include <stdarg.h>
#include <uuid/uuid.h>
#include "gfal_common.h"



typedef gpointer catalog_handle;

/**
 * @struct Object for a catalog type, modelise all operation that can be done by the catalog
 */
typedef struct{
	// handle
	catalog_handle handle;
	// delete
	/**
	 * Called before the destruction of the catalog interface
	 */
	void (*catalog_delete)(catalog_handle); 
	// members
	gboolean (*check_catalog_url)(catalog_handle, char* url,  catalog_mode mode);
	/**
	 *  access function for the access for the normal associated url
	 * */
	int (*accessG)(catalog_handle, char* path, int mode, GError** err);
	/**
	 *  function pointer for the access using the guid url
	 * */
	int (*access_guidG)(catalog_handle, char* path, int mode, GError** err);
	int	(*chmodG)(catalog_handle, const char *, mode_t, GError** err);
	int	(*renameG)(catalog_handle, const char *, const char *, GError** err);	
	
} gfal_catalog_interface;

// include the catalogs
#include "lfc/gfal_common_lfc.h"

/**
	\brief catalog type getter
	
	@return return a string of the type of the catalog
	 return NULL if an error occured and set the GError correctly
*/
extern char* gfal_get_cat_type(GError**);


void gfal_catalogs_instance(gfal_handle, GError** err);

void gfal_catalogs_delete();
 	

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
