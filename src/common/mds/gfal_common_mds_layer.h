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
 * @file gfal_common_mds_layer.h
 * @brief header file for the external call to the bdii system, mock purpose
 * @author Adrien Devresse
 * @date 10/06/2011
 * */


#include "ServiceDiscoveryIfce.h"
 
struct _gfal_mds_external_call{
	int (*sd_get_se_types_and_endpoints)(const char *host, char ***se_types, char ***se_endpoints,char *errbuf, int errbufsz);	
	
	void (*set_gfal_vo)(char *vo);
	
	void (*set_gfal_fqan)(char **fqan, int fqan_size);	
	
	int (*sd_get_lfc_endpoint)(char **lfc_endpoint,char *errbuf, int errbufsz);


};



extern struct _gfal_mds_external_call gfal_mds_external_call;
