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
 * @file gfal_common_filedescriptor.h
 * @brief  header file for the file descriptor management
 * @author Devresse Adrien
 * @version 2.0
 * @date 22/05/2011
 * */
 
#include <glib.h>
#include "gfal_prototypes.h"

struct _gfal_file_descriptor_container{
	GHashTable* container;	
};


/**
 * Generate a new unique key for the given container
 */ 
int gfal_file_key_generatorG(gfal_file_descriptor_handle fhandle, GError** err);
