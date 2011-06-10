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
 * @file gfal_common_mds_layer.c
 * @brief file for the external call to the bdii system, mock purpose
 * @author Adrien Devresse
 * @date 10/06/2011
 * */


#include <lber.h>
#include <ldap.h>
#include "gfal_common_mds_layer.h"


struct _gfal_mds_external_call gfal_mds_external_call = {
	.sd_get_se_types_and_endpoints = &sd_get_se_types_and_endpoints,
	.set_gfal_vo = &set_gfal_vo,
	.set_gfal_fqan= &set_gfal_fqan,
	.sd_get_lfc_endpoint = &sd_get_lfc_endpoint
};
