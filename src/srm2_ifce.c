/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: srm2_ifce.c,v $ $Revision: 1.1 $ $Date: 2006/03/31 15:53:11 $ CERN Jean-Philippe Baud
 */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "srm2H.h"
#include "srmSoapBinding+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "srm2C.c"
#include "srm2Client.c"

#include "gfal_api.h"

static int
srm2_init (struct soap *soap, const char *surl, char *srm_endpoint,
          int srm_endpointsz, char *errbuf, int errbufsz)
{
	int flags;
	char *sfn;

	if (parsesurl (surl, srm_endpoint, srm_endpointsz, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	soap_init (soap);
	

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif
	return (0);
}

