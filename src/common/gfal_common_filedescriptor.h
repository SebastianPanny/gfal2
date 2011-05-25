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

struct _gfal_file_handle_{
	int module_id;
	gpointer fdesc;	
};

 // low level funcs
gfal_file_descriptor_handle gfal_file_descriptor_handle_create(GDestroyNotify destroyer);

int gfal_add_new_file_desc(gfal_file_descriptor_handle fhandle, gpointer pfile, GError** err);

gboolean gfal_remove_file_desc(gfal_file_descriptor_handle fhandle, int key, GError** err);


gpointer gfal_get_file_desc(gfal_file_descriptor_handle fhandle, int key, GError** err);


// high level funcs
gboolean gfal_file_handle_delete(gfal_file_descriptor_handle h, int file_desc, GError** err);


int gfal_dir_handle_create(gfal_file_descriptor_handle h, int module_id, gpointer real_file_desc, GError** err);


gfal_file_handle gfal_dir_handle_bind(gfal_file_descriptor_handle h, int file_desc, GError** err);
