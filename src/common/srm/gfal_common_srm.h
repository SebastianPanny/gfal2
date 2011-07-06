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



#include "../gfal_types.h"
#include "../gfal_common_internal.h"
#include "../gfal_prototypes.h"
#include "../gfal_constants.h"
#include "gfal_common_srm_endpoint.h"



#define GFAL_PREFIX_SRM "srm://"
#define GFAL_ENDPOINT_DEFAULT_PREFIX "httpg://"


struct _gfal_srm_result{
	char turl[GFAL_URL_MAX_LEN+1];
	int err_code;
	char err_str[GFAL_ERRMSG_LEN+1];
};

typedef void* srm_request_handle;


const char* gfal_srm_getName();

gfal_catalog_interface gfal_srm_initG(gfal_handle handle, GError** err);

int gfal_get_asyncG(gfal_handle handle, GList* surls, GError** err);

int gfal_get_async_resultsG(gfal_handle handle, GList** turls,  GError** err);

gboolean gfal_async_request_is_finishedG(gfal_handle handle, GError** err);

int gfal_get_async_results_errcodesG(gfal_handle handle, GList** turl_errcode, GError** err);

int gfal_get_async_results_errstringG(gfal_handle handle, GList** turl_errstring, GError** err);

int gfal_get_async_results_structG(gfal_handle handle, gfal_srm_result** tab_struct, GError** err);

int gfal_wait_async_requestG(gfal_handle handle, long timeout, GError** err);






