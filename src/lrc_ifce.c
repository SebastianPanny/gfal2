/*
 * Copyright (C) 2003-2004 by CERN
 */

/*
 * @(#)$RCSfile: lrc_ifce.c,v $ $Revision: 1.7 $ $Date: 2004/10/24 10:50:19 $ CERN Jean-Philippe Baud
 */

#include <errno.h>
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

#include "gfal_api.h"

static int
lrc_init (struct soap *soap)
{
	int flags;

	soap_init (soap);
	soap->namespaces = namespaces_lrc;

	if (lrc_endpoint == NULL &&
	    (lrc_endpoint = getenv ("LRC_ENDPOINT")) == NULL &&
	    get_rls_endpoints (&lrc_endpoint, &rmc_endpoint)) {
		errno = EINVAL;
		return (-1);
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
lrc_guidforpfn (const char *pfn)
{
	struct impl__guidForPfnResponse out;
	char *p;
	int ret;
	int sav_errno;
	struct soap soap;

	if (lrc_init (&soap) < 0)
		return (NULL);

	if (ret = soap_call_impl__guidForPfn (&soap, lrc_endpoint, "",
	    (char *) pfn, &out)) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "NOSUCHPFN"))
				sav_errno = ENOENT;
			else
				sav_errno = ECOMM;
		} else
			sav_errno = ECOMM;
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

lrc_guid_exists (const char *guid)
{
	struct impl__guidExistsResponse out;
	int ret;
	struct soap soap;

	if (lrc_init (&soap) < 0)
		return (-1);

	if (ret = soap_call_impl__guidExists (&soap, lrc_endpoint, "",
	    (char *) guid, &out)) {
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

lrc_register_pfn (const char *guid, const char *pfn)
{
	struct impl__addMappingResponse out;
	int ret;
	int sav_errno;
	struct soap soap;

	if (lrc_init (&soap) < 0)
		return (-1);

	if (ret = soap_call_impl__addMapping (&soap, lrc_endpoint, "",
	    (char *) guid, (char *) pfn, &out)) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "PFNEXISTS"))
				sav_errno = EEXIST;
			else if (strstr (soap.fault->faultcode, "VALUETOOLONG"))
				sav_errno = ENAMETOOLONG;
			else
				sav_errno = ECOMM;
		} else
			sav_errno = ECOMM;
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

lrc_setfilesize (const char *pfn, long long filesize)
{
	struct impl__setStringPfnAttributeResponse out;
	int ret;
	int sav_errno;
	struct soap soap;
	char tmpbuf[21];

	if (lrc_init (&soap) < 0)
		return (-1);

	sprintf (tmpbuf, "%lld", filesize);
	if (ret = soap_call_impl__setStringPfnAttribute (&soap, lrc_endpoint,
	    "", (char *) pfn, "size", tmpbuf, &out)) {
		if (ret == SOAP_FAULT) {
			if (strstr (soap.fault->faultcode, "NOSUCHPFN"))
				sav_errno = ENOENT;
			else
				sav_errno = ECOMM;
		} else
			sav_errno = ECOMM;
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
lrc_surlfromguid (const char *guid)
{
	struct impl__getPfnsResponse out;
	char *p, *result;
	int ret;
	int sav_errno;
	struct soap soap;
	char **surls;

	if (lrc_init (&soap) < 0)
		return (NULL);

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
	  result = getbestfile (out._getPfnsReturn->__ptr, 
				out._getPfnsReturn->__size);
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
lrc_surlsfromguid (const char *guid)
{
	int i;
	int j;
	struct impl__getPfnsResponse out;
	int ret;
	int sav_errno;
	struct soap soap;
	char **surlarray;

	if (lrc_init (&soap) < 0)
		return (NULL);

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
	}
	if ((surlarray = calloc (out._getPfnsReturn->__size + 1, sizeof(char *))) == NULL)
		return (NULL);
	for (i = 0; i < out._getPfnsReturn->__size; i++) {
		if ((surlarray[i] = strdup (out._getPfnsReturn->__ptr[i])) == NULL) {
			for (j = 0; j < i; j++)
				free (surlarray[j]);
			free (surlarray);
			return (NULL);
		}
	}
	soap_end (&soap);
	soap_done (&soap);
	return (surlarray);
}

lrc_unregister_pfn (const char *guid, const char *pfn)
{
	struct impl__removeMappingResponse out;
	int ret;
	struct soap soap;

	if (lrc_init (&soap) < 0)
		return (-1);

	if (ret = soap_call_impl__removeMapping (&soap, lrc_endpoint, "",
	    (char *) guid, (char *) pfn, &out)) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}
