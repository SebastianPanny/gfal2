#pragma once
/* 
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
* 
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at 
*
*    http://www.apache.org/licenses/LICENSE-2.0 
* 
* Unless required by applicable law or agreed to in writing, software 
* distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/
 
 
 
/*
 * gfal_common_filedescriptor.h
 * header file for the file descriptor management
 * author Devresse Adrien
 */
 
#include <glib.h>
#include <stdlib.h>
#include <pthread.h>
#include <common/gfal_prototypes.h>
#include <common/gfal_constants.h>
#include <fdesc/gfal_file_handle.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct _gfal_file_descriptor_container{
	GHashTable* container;
	pthread_mutex_t m_container;	
};

struct _gfal_file_handle_{
	char module_name[GFAL_MODULE_NAME_SIZE]; // This MUST be the Name of the plugin associated with this handle !
	GMutex* lock;
	off_t offset;
	gpointer ext_data;
	gpointer fdesc;	
    gchar* path;
};

 // low level funcs
gfal_fdesc_container_handle gfal_file_descriptor_handle_create(GDestroyNotify destroyer);

int gfal_add_new_file_desc(gfal_fdesc_container_handle fhandle, gpointer pfile, GError** err);

gboolean gfal_remove_file_desc(gfal_fdesc_container_handle fhandle, int key, GError** err);


gpointer gfal_get_file_desc(gfal_fdesc_container_handle fhandle, int key, GError** err);


// high level funcs

gfal_file_handle gfal_file_handle_bind(gfal_fdesc_container_handle h, int file_desc, GError** err);

// convenience funcs


void gfal_file_handle_delete(gfal_file_handle fh);

void gfal_file_handle_lock(gfal_file_handle fh);

void gfal_file_handle_unlock(gfal_file_handle fh);

#ifdef __cplusplus
}
#endif

