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

/***
 * @brief  gfal_types.h
 * @author  Adrien Devresse
 * types declaration for gfal  
 * */
#ifndef _GFAL_TYPES_H
#define _GFAL_TYPES_H

/* enforce proper calling convention */
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <glib.h>
#endif

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <gfal_srm_ifce_types.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_common_catalog.h"




//typedef struct srm_spacemd gfal_spacemd;
enum status_type {DEFAULT_STATUS = 0, MD_STATUS, PIN_STATUS};

enum se_type {TYPE_NONE = 0, TYPE_SRM, TYPE_SRMv2, TYPE_SE};
enum gfal_srm_proto {PROTO_SRM=0, PROTO_SRMv2, PROTO_ERROR_UNKNOW};


struct _gfal_descriptors_container{
	gfal_fdesc_container_handle dir_container;
	gfal_fdesc_container_handle file_container;
	
} ;

 
 /**
  * @struct structure for the srmv2 option management
  *  set to 0 by default
  */
struct _gfal_srmv2_opt{
	int opt_srmv2_desiredpintime;			//	optional desired default endpoint
	char** opt_srmv2_protocols;				// optional protocols list for manual set
	char * opt_srmv2_spacetokendesc;		// optional spacetokens desc for srmv2	 
	gint64 filesizes;
};

 /**
  * symbolise the state of a request
  *  needed to get the response
  */ 
struct _gfal_request_state{
	char *						srmv2_token;
	struct srmv2_filestatus *	srmv2_statuses;
	struct srmv2_pinfilestatus *srmv2_pinstatuses;
	enum gfal_srm_proto current_request_proto;
	char * 						request_endpoint; 
	gboolean					finished;			// finished or not
	int							number;				// number of files in request
 };
 
struct gfal_handle_ {
	enum gfal_srm_proto srm_proto_type;		// define the protocole version of SRM choosen by default
	gboolean initiated; 					// 1 if initiated, else error
	// pointer to srmv2 set option
	gfal_srmv2_opt* srmv2_opt;
	// pointer to the last request resu
	gfal_request_state* last_request_state;
	// struct of the catalog opts
	struct _catalog_opts catalog_opt;
	//struct for the file descriptors
	gfal_descriptors_container fdescs;
	int no_bdii_check;
};





#ifdef __cplusplus
}
#endif

#endif
