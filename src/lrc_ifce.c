/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: lrc_ifce.c,v $ $Revision: 1.20 $ $Date: 2008/05/08 13:16:36 $ CERN Jean-Philippe Baud
 */

#include <errno.h>
#include <string.h>
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "lrcH.h"
#include "edg_local_replica_catalogSoapBinding+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "lrcC.c"
#include "lrcClient.c"
char *lrc_endpoint;
extern char *rmc_endpoint;

#include "gfal_api.h"

	static int
lrc_init (struct soap *soap, char *errbuf, int errbufsz)
{
	int flags;

	soap_init (soap);
	soap->namespaces = namespaces_lrc;

	if (lrc_endpoint == NULL && (lrc_endpoint = getenv ("LRC_ENDPOINT")) == NULL) {
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
	if (strncmp (lrc_endpoint, "https", 5) == 0) {
		flags = CGSI_OPT_SSL_COMPATIBLE;
		soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
	}
#endif
	return (0);
}


char *
lrc_get_catalog_endpoint(char *errbuf, int errbufsz) {
	struct soap soap;
	if(lrc_init(&soap, errbuf, errbufsz) < 0)
		return (NULL);
	return lrc_endpoint;
}

	int
lrc_replica_exists (const char *guid, char *errbuf, int errbufsz)
{
	struct lrc__getPfnsResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_lrc__getPfns (&soap, lrc_endpoint, "",
					(char *) guid, &out))) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "NOSUCHGUID"))
				sav_errno = 0;
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
		// return 'false' if guid does not exist in LRC
		if(sav_errno == 0)
			return (0);

		errno = sav_errno;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (out._getPfnsReturn->__size==0?0:1);
}

	char *
lrc_guidforpfn (const char *pfn, char *errbuf, int errbufsz)
{
	struct lrc__guidForPfnResponse out;
	char *p;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (NULL);

	if ((ret = soap_call_lrc__guidForPfn (&soap, lrc_endpoint, "",
				(char *) pfn, &out))) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "NOSUCHPFN"))
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
	p = strdup (out._guidForPfnReturn);
	soap_end (&soap);
	soap_done (&soap);
	return (p);
}

lrc_guid_exists (const char *guid, char *errbuf, int errbufsz)
{
	struct lrc__guidExistsResponse out;
	int ret;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_lrc__guidExists (&soap, lrc_endpoint, "",
				(char *) guid, &out))) {
		gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);	
		errno = ECOMM;
		return (-1);
	}
	ret = out._guidExistsReturn ? 1 : 0;
	soap_end (&soap);
	soap_done (&soap);
	return (ret);
}

lrc_register_pfn (const char *guid, const char *pfn, char *errbuf, int errbufsz)
{
	struct lrc__addMappingResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_lrc__addMapping (&soap, lrc_endpoint, "",
				(char *) guid, (char *) pfn, &out))) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "PFNEXISTS"))
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

lrc_setfilesize (const char *pfn, GFAL_LONG64 filesize, char *errbuf, int errbufsz)
{
	struct lrc__setStringPfnAttributeResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;
	char tmpbuf[21];

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	sprintf (tmpbuf, GFAL_LONG64_FORMAT, filesize);
	if ((ret = soap_call_lrc__setStringPfnAttribute (&soap, lrc_endpoint,
				"", (char *) pfn, "size", tmpbuf, &out))) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "NOSUCHPFN"))
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
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

	char *
lrc_surlfromguid (const char *guid, char *errbuf, int errbufsz)
{
	struct lrc__getPfnsResponse out;
	char *p, *result;
	int ret;
	int sav_errno = 0;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (NULL);

	if ((ret = soap_call_lrc__getPfns (&soap, lrc_endpoint, "",
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
	} else {
		result = getbestfile (out._getPfnsReturn->__ptr, 
				out._getPfnsReturn->__size, errbuf, errbufsz);
		if(result == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			errno = EPROTONOSUPPORT;
			return (NULL);
		}
		p = strdup (result);
		soap_end (&soap);
		soap_done (&soap);
		return (p);
	}

}

	char **
lrc_surlsfromguid (const char *guid, char *errbuf, int errbufsz)
{
	int i;
	int j;
	struct lrc__getPfnsResponse out;
	int ret;
	int sav_errno = 0;
	struct soap soap;
	char **surlarray;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (NULL);

	if ((ret = soap_call_lrc__getPfns (&soap, lrc_endpoint, "",
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
	if ((surlarray = calloc (out._getPfnsReturn->__size + 1, sizeof(char *))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (NULL);
	}
	for (i = 0; i < out._getPfnsReturn->__size; i++) {
		if ((surlarray[i] = strdup (out._getPfnsReturn->__ptr[i])) == NULL) {
			for (j = 0; j < i; j++)
				free (surlarray[j]);
			free (surlarray);
			soap_end (&soap);
			soap_done (&soap);
			errno = ENOMEM;
			return (NULL);
		}
	}
	soap_end (&soap);
	soap_done (&soap);
	return (surlarray);
}

lrc_unregister_pfn (const char *guid, const char *pfn, char *errbuf, int errbufsz)
{
	struct lrc__removeMappingResponse out;
	int ret;
	struct soap soap;

	if (lrc_init (&soap, errbuf, errbufsz) < 0)
		return (-1);

	if ((ret = soap_call_lrc__removeMapping (&soap, lrc_endpoint, "",
				(char *) guid, (char *) pfn, &out))) {
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
