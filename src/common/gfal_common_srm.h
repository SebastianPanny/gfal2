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
 * @file gfal_common_srm.h
 * @brief the header file with the main srm funcs of the common API
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/04/2011
 * */


 
#include "gfal_common.h"
#include <regex.h>
#include <time.h>

#define GFAL_PREFIX_SRM "srm://"
#define GFAL_ENDPOINT_DEFAULT_PREFIX "httpg://"


gfal_handle gfal_initG(GError** err);

void gfal_handle_freeG(gfal_handle handle);

int gfal_get_asyncG(gfal_handle handle, GList* surls, GError** err);

/**
 * get the result to the last get_async request
 * @return return the number of response in turls or negative value if error
 * @param handle : handle of the current context
 * @param GList** turls : GList<char*> turls with the full list of answer, an answer with error is a NULL pointer
 * @warning turls need to be free manually 
 */
int gfal_get_async_resultsG(gfal_handle handle, GList** turls,  GError** err);



gboolean gfal_async_request_is_finishedG(gfal_handle handle, GError** err);


int gfal_get_async_get_request_errcodesG(gfal_handle handle, GList** turl_errcode, GError** err);

int gfal_get_async_get_request_errstringG(gfal_handle handle, GList** turl_errstring, GError** err);

int gfal_wait_async_requestG(gfal_handle handle, long timeout, GError** err);
  
void gfal_set_nobdiiG(gfal_handle handle, gboolean no_bdii_chk);


