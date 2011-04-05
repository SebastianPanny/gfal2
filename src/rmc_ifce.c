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

/*
 * @(#)$RCSfile: rmc_ifce.c,v $ $Revision: 1.19 $ $Date: 2008/12/03 12:38:47 $ CERN Jean-Philippe Baud
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "gfal_api.h"
#define WITH_NOGLOBAL
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "rmcH.h"
#include "edg_replica_metadata_catalogSoapBinding.h"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "rmcC.c"
#include "rmcClient.c"
#include "gfal_internals.h"

extern char *lrc_endpoint;
char *rmc_endpoint;

	static int
rmc_init (struct soap *soap, char *errbuf, int errbufsz)
{
	int flags;

	soap_init (soap);
	soap->namespaces = namespaces_rmc;

	if (rmc_endpoint == NULL && (rmc_endpoint = getenv ("RMC_ENDPOINT")) == NULL) {
		if (!gfal_is_nobdii ()) {
			if (get_rls_endpoints (&lrc_endpoint, &rmc_endpoint, errbuf, errbufsz) != 0) {
				errno = EINVAL;
				return (-1);
			}
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
					"You have to define 'RMC_ENDPOINT' and 'LRC_ENDPOINT' environment variables, when BDII calls are disabled");
			errno = EINVAL;
			return (-1);
		}
	}

#ifdef GFAL_SECURE
	if (strncmp (rmc_endpoint, "https", 5) == 0) {
		flags = CGSI_OPT_SSL_COMPATIBLE;
		soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
	}
#endif
	gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
			"RMC/LRC are obsolete! Please use LFC instead.");
	return (0);
}

	char *
rmc_guidfromlfn (const char *lfn, char *errbuf, int errbufsz)
{
	struct rmc__guidForAliasResponse out;
	char *p;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (rmc_init (&soap, errbuf, errbufsz) < 0)
		return (NULL);

	if ((ret = soap_call_rmc__guidForAlias (&soap, rmc_endpoint, "",
				(char *) lfn, &out))) {
		if (ret == SOAP_FAULT && strstr (soap.fault->faultcode, "NOSUCHALIAS"))
			sav_errno = ENOENT;
		else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
			sav_errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (NULL);
	} else {
		p = strdup (out._guidForAliasReturn);
		soap_end (&soap);
		soap_done (&soap);
		return (p);
	}
}

	char **
rmc_lfnsforguid (const char *guid, char *errbuf, int errbufsz)
{
	int i;
	int j;
	char **lfnarray;
	struct rmc__getAliasesResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (rmc_init (&soap, errbuf, errbufsz) < 0)
		return (NULL);

	if ((ret = soap_call_rmc__getAliases (&soap, rmc_endpoint, "",
				(char *) guid, &out))) {
		if (ret == SOAP_FAULT && strstr (soap.fault->faultcode, "NOSUCHGUID")) {
			sav_errno = ENOENT;
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
			sav_errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (NULL);
	}
	if ((lfnarray = calloc (out._getAliasesReturn->__size + 1, sizeof(char *))) == NULL) 
		return (NULL);
	for (i = 0; i < out._getAliasesReturn->__size; i++) {
		if ((lfnarray[i] = strdup (out._getAliasesReturn->__ptr[i])) == NULL) {
			for (j = 0; j < i; j++)
				free (lfnarray[j]);
			free (lfnarray);
			return (NULL); 
		}
	}
	lfnarray[i] = NULL;
	soap_end (&soap);
	soap_done (&soap);
	return (lfnarray);
}

rmc_register_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
	struct rmc__addAliasResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (rmc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_rmc__addAlias (&soap, rmc_endpoint, "",
				(char *) guid, (char *) lfn, &out))) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "ALIASEXISTS"))
				sav_errno = EEXIST;
			else if (strstr (soap.fault->faultcode, "VALUETOOLONG"))
				sav_errno = ENAMETOOLONG;
			else {
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
				sav_errno = ECOMM;
			}
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
			sav_errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

rmc_unregister_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
	struct rmc__removeAliasResponse out;
	int ret;
	struct soap soap;

	if (rmc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_rmc__removeAlias (&soap, rmc_endpoint, "",
				(char *) guid, (char *) lfn, &out))) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "%s", soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}
