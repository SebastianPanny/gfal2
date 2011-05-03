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
  * @brief  header file for the bdii request part of gfal
  * @author : Devresse Adrien
  * @version 2.0.0
  * @date 18/04/2011
  * */
#define _GNU_SOURCE
#include "../gfal_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <lber.h>
#include <ldap.h>
#include "ServiceDiscovery.h"

int gfal_mds_get_se_types_and_endpoints(const char *host, char ***se_types, char ***se_endpoints, GError** err);



char * gfal_get_lfchost_bdii(gfal_handle handle, GError** err);


void gfal_set_nobdiiG(gfal_handle handle, gboolean no_bdii_chk);

gboolean gfal_get_nobdiiG(gfal_handle handle);
 
 
 
