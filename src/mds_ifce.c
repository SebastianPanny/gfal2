/*                                                                -*- mode: c; c-basic-offset: 8;  -*-
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

/*
 * @(#)$RCSfile: mds_ifce.c,v $ $Revision: 1.87 $ $Date: 2009/10/01 09:53:37 $ CERN Jean-Philippe Baud
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <lber.h>
#include <ldap.h>
#include "gfal_api.h"
#include "gfal_internals.h"
#include "ServiceDiscovery.h"


/* get BDII hostname and port number */
/* THIS LOGIC IS IMPLEMENTED IN THE NEW SD LIBRARY */
get_bdii (char *bdii_server, int buflen, int *bdii_port, char *errbuf, int errbufsz)
{
    return (-1);
}

/* get from the BDII the RLS endpoints */
/*NOT IMPLEMENTED*/
get_rls_endpoints (char **lrc_endpoint, char **rmc_endpoint, char *errbuf, int errbufsz)
{
	return (-1);
}


/* get from the BDII the CE Accesspoint for a given SE */
get_ce_ap (const char *host, char **ce_ap, char *errbuf, int errbufsz)
{
	return sd_get_ce_ap(host, ce_ap);
}


/* get from the BDII the endpoint for the LFC */

get_lfc_endpoint (char **lfc_endpoint, char *errbuf, int errbufsz)
{
	return sd_get_lfc_endpoint(lfc_endpoint);
}



get_storage_path (const char *host, const char *spacetokendesc, char **sa_path, char **sa_root, char *errbuf, int errbufsz)
{
	return sd_get_storage_path(host, spacetokendesc, sa_path, sa_root);
}


/* get from the BDII the SE type (disk, srm_v1) */

get_se_types_and_endpoints (const char *host, char ***se_types, char ***se_endpoints, char *errbuf, int errbufsz)
{
	return sd_get_se_types_and_endpoints(host, se_types, se_endpoints);
}

/* get from the BDII the list of supported protocols with their associated
 * port number
 */

get_seap_info (const char *host, char ***access_protocol, int **port, char *errbuf, int errbufsz)
{
	return sd_get_seap_info(host, access_protocol, port);
}
