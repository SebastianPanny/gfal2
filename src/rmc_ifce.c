/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: rmc_ifce.c,v $ $Revision: 1.1.1.1 $ $Date: 2003/11/19 12:56:29 $ CERN Jean-Philippe Baud
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "rmcH.h"
#include "soapEdgReplicaMetadataCatalogService+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "rmcC.c"
#include "rmcClient.c"
extern char *lrc_endpoint;
char *rmc_endpoint;

char *
guidfromlfn (const char *lfn)
{
	int flags;
	struct impl__guidForAliasResponse out;
	char *p;
	int ret;
	int sav_errno;
	struct soap soap;

	soap_init(&soap);
	soap.namespaces = namespaces_rmc;

	if (rmc_endpoint == NULL &&
	    (rmc_endpoint = getenv ("RMC_ENDPOINT")) == NULL &&
	    get_rls_endpoints (&lrc_endpoint, &rmc_endpoint)) {
		errno = EINVAL;
		return (NULL);
	}

#ifdef GFAL_SECURE
	if (strncmp (rmc_endpoint, "https", 5) == 0) {
		flags = CGSI_OPT_SSL_COMPATIBLE;
		soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
	}
#endif

	if (ret = soap_call_impl__guidForAlias (&soap, rmc_endpoint, "",
	    (char *) lfn, &out)) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "NOSUCHALIAS"))
				sav_errno = ENOENT;
			else
				sav_errno = ECOMM;
		} else
			sav_errno = ECOMM;
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
