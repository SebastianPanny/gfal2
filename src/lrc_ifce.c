/*
 * Copyright (C) 2003-2004 by CERN
 */

/*
 * @(#)$RCSfile: lrc_ifce.c,v $ $Revision: 1.4 $ $Date: 2004/05/12 08:24:43 $ CERN Jean-Philippe Baud
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

int
getdomainnm (char *name, int namelen)
{
	FILE *fd;
	char line[300];
	char *p;

	if ((fd = fopen ("/etc/resolv.conf", "r")) != NULL) {
		while (fgets (line, sizeof(line), fd) != NULL) {
			if ((strncmp (line, "domain", 6) == 0 ||
			    strncmp (line, "search", 6) == 0) && line[6] == ' ') {
				fclose (fd);
				p = line + 6;
				while (*p == ' ')
					p++;
				if (*p)
					*(p + strlen (p) - 1) = '\0';
				if (strlen (p) > namelen) {
					errno = EINVAL;
					return (-1);
				}
				strcpy (name, p);
				return (0);
			}
		}
		fclose (fd);
	}
	return (-1);
}

char *
guidforpfn (const char *pfn)
{
	int flags;
	struct impl__guidForPfnResponse out;
	char *p;
	int ret;
	int sav_errno;
	struct soap soap;

	soap_init (&soap);
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
	int flags;
	struct impl__guidExistsResponse out;
	int ret;
	struct soap soap;

	soap_init (&soap);
	soap.namespaces = namespaces_lrc;

	if (lrc_endpoint == NULL &&
	    (lrc_endpoint = getenv ("LRC_ENDPOINT")) == NULL &&
	    get_rls_endpoints (&lrc_endpoint, &rmc_endpoint)) {
		errno = EINVAL;
		return (-1);
	}

#ifdef GFAL_SECURE
	if (strncmp (lrc_endpoint, "https", 5) == 0) {
		flags = CGSI_OPT_SSL_COMPATIBLE;
		soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
	}
#endif

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

register_pfn (const char *guid, const char *pfn)
{
	int flags;
	struct impl__addMappingResponse out;
	int ret;
	int sav_errno;
	struct soap soap;

	soap_init (&soap);
	soap.namespaces = namespaces_lrc;

	if (lrc_endpoint == NULL &&
	    (lrc_endpoint = getenv ("LRC_ENDPOINT")) == NULL &&
	    get_rls_endpoints (&lrc_endpoint, &rmc_endpoint)) {
		errno = EINVAL;
		return (-1);
	}

#ifdef GFAL_SECURE
	if (strncmp (lrc_endpoint, "https", 5) == 0) {
		flags = CGSI_OPT_SSL_COMPATIBLE;
		soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
	}
#endif

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

char *
surlfromguid (const char *guid)
{
	char dname[64];
	int first;
	int flags;
	int i;
	struct impl__getPfnsResponse out;
	char *p, *p1, *p2;
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
		/* skip entries not in the form srm: or sfn:
		 * take entry on same domain if it exists else
		 * take the first supported entry
		 */
		first = -1;
		*dname = '\0';
		(void) getdomainnm (dname, sizeof(dname));
		for (i = 0; i < out._getPfnsReturn->__size; i++) {
			p = out._getPfnsReturn->__ptr[i];
			if (strncmp (p, "srm://", 6) && strncmp (p, "sfn://", 6))
				continue;
			if ((p1 = strchr (p + 6, '/')) == NULL) continue;
			*p1 = '\0';
			if ((p2 = strchr (p + 6, ':')))
				*p2 = '\0';
			if ((p = strchr (p + 6, '.')) == NULL) continue;
			if (first < 0) first = i;
			ret = strcmp (p + 1, dname);
			*p1 = '/';
			if (p2) *p2 = ':';
			if (ret == 0) break;	/* domains match ==> local replica */
		}
		if (i == out._getPfnsReturn->__size) {	/* no entry on same domain */
			if (first < 0) {	/* only non suported entries */
				soap_end (&soap);
				soap_done (&soap);
				errno = EPROTONOSUPPORT;
				return (NULL);
			}
			i = first;
		}
		p = strdup (out._getPfnsReturn->__ptr[i]);
		soap_end (&soap);
		soap_done (&soap);
		return (p);
	}
}

char **
surlsfromguid (const char *guid)
{
	int flags;
	int i;
	int j;
	struct impl__getPfnsResponse out;
	int ret;
	int sav_errno;
	struct soap soap;
	char **surlarray;

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

unregister_pfn (const char *guid, const char *pfn)
{
	int flags;
	struct impl__removeMappingResponse out;
	int ret;
	struct soap soap;

	soap_init (&soap);
	soap.namespaces = namespaces_lrc;

	if (lrc_endpoint == NULL &&
	    (lrc_endpoint = getenv ("LRC_ENDPOINT")) == NULL &&
	    get_rls_endpoints (&lrc_endpoint, &rmc_endpoint)) {
		errno = EINVAL;
		return (-1);
	}

#ifdef GFAL_SECURE
	if (strncmp (lrc_endpoint, "https", 5) == 0) {
		flags = CGSI_OPT_SSL_COMPATIBLE;
		soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
	}
#endif

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
