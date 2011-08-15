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
 * @file gfal_common_interface.h
 * @brief The main Interface file for the SRM interface
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/04/2011
 * */

#include "gfal_prototypes.h"
#include "gfal_types.h"


char* gfal_last_error_string(gfal_handle handle);

int gfal_has_error(gfal_handle handle);


gfal_handle gfal_handle_new();

void gfal_handle_free(gfal_handle handle);

int gfal_get_async(gfal_handle handle, char** surls);

int gfal_async_request_is_finished(gfal_handle handle);

int gfal_get_async_results(gfal_handle handle, char*** turls);

int gfal_get_async_get_results_errcodes(gfal_handle handle, int** turl_errcode);

int gfal_get_async_get_results_errstring(gfal_handle handle, char*** turl_errstring);

int gfal_wait_async_request(gfal_handle handle, long timeout);
  
void gfal_set_nobdii_srm(gfal_handle handle, gboolean no_bdii_chk);



