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
#include <attr/xattr.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_common_plugin_interface.h"




enum _GFAL_TYPE { GFAL_TYPE_INT=0, GFAL_TYPE_STRING=1 };

enum _GFAL_PARAM_FUNC{ GFAL_PARAM_SET=0, GFAL_PARAM_GET=1 };



struct _gfal_descriptors_container{
	gfal_fdesc_container_handle dir_container;
	gfal_fdesc_container_handle file_container;
	
};

struct _gfal_conf_container{
	GData*  conf;
	GMutex* mux;	
};


 
struct gfal_handle_ {		// define the protocole version of SRM choosen by default
	gboolean initiated; 					// 1 if initiated, else error
	// struct of the plugin opts
	struct _plugin_opts plugin_opt;
	//struct for the file descriptors
	gfal_descriptors_container fdescs;
	int no_bdii_check;
	gfal_conf_container st_config;
};





#ifdef __cplusplus
}
#endif

#endif
