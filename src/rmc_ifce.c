/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: rmc_ifce.c,v $ $Revision: 1.16 $ $Date: 2008/08/23 07:00:30 $ CERN Jean-Philippe Baud
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
#include "edg_replica_metadata_catalogSoapBinding+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "rmcC.c"
#include "rmcClient.c"
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
			gfal_errmsg(errbuf, errbufsz, "You have to define 'RMC_ENDPOINT' and 'LRC_ENDPOINT' environment variables, when BDII calls are disabled");
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
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "NOSUCHALIAS"))
				sav_errno = ENOENT;
			else {
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
				sav_errno = ECOMM;
			}
		} else {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
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
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "NOSUCHGUID"))
				sav_errno = ENOENT;
			else {
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
				sav_errno = ECOMM;
			}
		} else {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
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
	soap_end (&soap);
	soap_done (&soap);
	return (lfnarray);
}

rmc_create_alias(const char *guid, const char* lfn, char *errbuf, int errbufsz)
{
	return (rmc_register_alias (guid, lfn, errbuf, errbufsz));
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
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
				sav_errno = ECOMM;
			}
		} else {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
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
		gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}
