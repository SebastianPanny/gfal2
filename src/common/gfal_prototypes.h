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
 * @file contain all the prototypes needed by the gfal common files
 * @author Devresse Adrien
 * @version 0.0.1
 * @date 04/05/2011
 * 
 * */
 #include <glib.h>
 
 // opts SRM
typedef struct _gfal_request_state gfal_request_state; 
typedef struct _gfal_srmv2_opt gfal_srmv2_opt; 


 // catalog 
typedef struct _catalog_opts catalog_opts;
typedef struct _gfal_catalog_interface gfal_catalog_interface;
typedef enum _catalog_mode catalog_mode;
typedef gpointer catalog_handle;

// main 
typedef struct gfal_handle_* gfal_handle;

