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
#include "lfc/gfal_common_lfc.h"







/**
	\brief catalog type getter
	
	@return return a string of the type of the catalog
	 return NULL if an error occured and set the GError correctly
*/
extern char* gfal_get_cat_type(GError**);


void gfal_catalogs_instance(GError** err);

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
