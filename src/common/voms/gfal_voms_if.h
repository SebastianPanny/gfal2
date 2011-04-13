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
 
#include <glib.h>
#include "voms_apic.h"
 
/**
 * @brief dn of the current registered user
 * @return string of the dn of the current user registered by the voms proxy or null if error
 * need to be free after use
 */ 
char *gfal_get_userdnG (GError** err);


/**
 * @brief vo of the current user
 * @return string of the VO of the current user or null ir error
 * need to be free after use
 */
char *gfal_get_voG (GError** err);

/**
 * @brief get the user attributes
 * @return return an array of string of the compact user Attribute or NULL if error
 * do not free
 */ 
GArray* gfal_get_fqanG(GError** err);
 
