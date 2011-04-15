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
 * @file gfal_voms_if.c
 * @brief header file containing the interface to the voms system
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/04/2011
 * */
#define _GNU_SOURCE
 
#include <glib.h>
#include "../gfal_common.h"
#include "voms_apic.h"
 

char *gfal_get_userdnG (GError** err);



char *gfal_get_voG (GError** err);


GList* gfal_get_fqanG(GError** err);
 