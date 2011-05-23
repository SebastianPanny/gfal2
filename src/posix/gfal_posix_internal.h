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
 * @file gfal_posix_local_file.c
 * @brief header file for the internal func of the posix interface
 * @author Devresse Adrien
 * @version 2.0
 * @date 06/05/2011
 * */


#include "gfal_prototypes.h"
#include "gfal_types.h"


gfal_handle gfal_posix_instance();
 
int gfal_posix_internal_access (const char *path, int amode);

int gfal_posix_internal_chmod(const char* path, mode_t mode);

int gfal_posix_internal_stat(const char* path, struct stat* buf);

int gfal_posix_internal_lstat(const char* path, struct stat* buf);

int gfal_posix_internal_mkdir(const char* path, mode_t mode);

int gfal_posix_internal_rmdir(const char* path);

void gfal_posix_register_internal_error(gfal_handle handle, const char* prefix, GError * tmp_err);
 

