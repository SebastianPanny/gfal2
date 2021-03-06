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
 * gfal_common_file_handle.h
 * header file for the file handle management
 * author Devresse Adrien
 * */


#include "gfal_constants.h" 
#include "gfal_prototypes.h"


gfal_fdesc_container_handle gfal_file_handle_container_instance(gfal_descriptors_container* fdescs, GError** err);

void gfal_file_handle_container_delete(gfal_descriptors_container* fdescs);


