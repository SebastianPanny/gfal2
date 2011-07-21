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
 * @file gfal_comm_srm_open.h
 * @brief header for open/read/write srm
 * @author Devresse Adrien
 * @date 07/06/2011
 * */
 
 #define _GNU_SOURCE 

#include <regex.h>
#include <time.h> 
 
#include "gfal_common_srm.h"
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_catalog.h"
#include "gfal_common_srm.h"



gfal_file_handle gfal_srm_openG(catalog_handle, const char* path, int flag, mode_t mode, GError**);

int gfal_srm_readG(catalog_handle, gfal_file_handle fd, void* buff, size_t count, GError**);

int gfal_srm_writeG(catalog_handle, gfal_file_handle fd, void* buff, size_t count, GError**);

int gfal_srm_closeG(catalog_handle, gfal_file_handle fd, GError **);

/**
 * lseek function for the srm  plugin
 */
int gfal_srm_lseekG(catalog_handle ch, gfal_file_handle fd, off_t offset, int whence, GError** err);

