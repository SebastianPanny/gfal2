/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: lrc_ifce.c,v $ $Revision: 1.1.1.1 $ $Date: 2003/11/19 12:56:29 $ CERN Jean-Philippe Baud
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "lrcH.h"
#include "soapEdgLocalReplicaCatalogService+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "lrcC.c"
#include "lrcClient.c"
char *lrc_endpoint;
extern char *rmc_endpoint;

char *
surlfromguid (const char *guid)
{
	int flags;
	struct impl__getPfnsResponse out;
	char *p;
	int ret;
	int sav_errno;
	struct soap soap;

	soap_init(&soap);
	soap.namespaces = namespaces_lrc;

	if (lrc_endpoint == NULL &&
	    (lrc_endpoint = getenv ("LRC_ENDPOINT")) == NULL &&
	    get_rls_endpoints (&lrc_endpoint, &rmc_endpoint)) {
		errno = EINVAL;
		return (NULL);
	}

#ifdef GFAL_SECURE
	if (strncmp (lrc_endpoint, "https", 5) == 0) {
		flags = CGSI_OPT_SSL_COMPATIBLE;
		soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
	}
#endif

	if (ret = soap_call_impl__getPfns (&soap, lrc_endpoint, "",
	    (char *) guid, &out)) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "NOSUCHGUID"))
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
		p = strdup (out._getPfnsReturn->__ptr[0]);
		soap_end (&soap);
		soap_done (&soap);
		return (p);
	}
}
