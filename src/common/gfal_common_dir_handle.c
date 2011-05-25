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
 * @file gfal_common_dir_handle.c
 * @brief file for the directory handle management
 * @author Devresse Adrien
 * @version 2.0
 * @date 25/05/2011
 * */

#include <stdlib.h>
#include <errno.h>
#include <glib.h>
#include "gfal_constants.h" 
#include "gfal_prototypes.h"
#include "gfal_common_filedescriptor.h"
#include "gfal_common_dir_handle.h"


static gfal_file_descriptor_handle dir_handle=NULL;




/**
 * 
 * return the singleton of the file descriptor container for the directories
 */
gfal_file_descriptor_handle gfal_dir_handle_instance(GError** err){
	if(dir_handle != NULL)
		return dir_handle;
	else{
		dir_handle = gfal_file_descriptor_handle_create(&free);
		if(!dir_handle)
			g_set_error(err, 0, EIO, "[%s] Error while init directories file descriptor container");
		return dir_handle;	
	}
}



