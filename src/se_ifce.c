/*
 * Copyright (C) 2003-2004 by CERN
 */

/*
 * @(#)$RCSfile: se_ifce.c,v $ $Revision: 1.1 $ $Date: 2004/05/26 08:07:48 $ CERN Jean-Philippe Baud
 */

#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include "gfal_api.h"
#include "seH.h"
#include "soapEdgSeWebserviceService.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif

se_deletesurl (const char *surl)
{
	int flags;
	struct impl__deleteResponse out;
	char *sfn;
	struct soap soap;
	char *srm_endpoint;

	if (parsesurl (surl, &srm_endpoint, &sfn) < 0)
		return (-1);

	soap_init (&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	/* issue "delete" request */

	if (soap_call_impl__delete (&soap, srm_endpoint,
	    "delete", (char *)surl + 6, &out)) {
		soap_print_fault (&soap, stderr);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

se_mkdir (const char *dir)
{
	int flags;
	struct impl__mkdirResponse out;
	char *sfn;
	struct soap soap;
	char *srm_endpoint;

	if (parsesurl (dir, &srm_endpoint, &sfn) < 0)
		return (-1);

	soap_init (&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	/* issue "mkdir" request */

	if (soap_call_impl__mkdir (&soap, srm_endpoint,
	    "mkdir", (char *)dir + 6, &out)) {
		soap_print_fault (&soap, stderr);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

char *
se_turlfromsurl (const char *surl, char **protocols, int oflag, char **reqid)
{
	int flags;
	int nbproto = 0;
	struct impl__cacheResponse outg;
	struct impl__createResponse outp;
	struct impl__getTurlResponse outq;
	char *p;
	struct ArrayOf_USCORE_xsd_USCORE_string protoarray;
	int ret;
	int sav_errno;
	char *sfn;
	struct soap soap;
	char *srm_endpoint;
	xsd__long zero = 0;

	if (parsesurl (surl, &srm_endpoint, &sfn) < 0)
		return (NULL);

	while (*protocols[nbproto]) nbproto++;

	soap_init(&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	/* issue "cache" or the "create" request */

	protoarray.__ptr = protocols;
	protoarray.__size = nbproto;
	protoarray.__offset = 0;

	if ((oflag & O_ACCMODE) == 0) {
		if ((ret = soap_call_impl__cache (&soap, srm_endpoint, "cache",
		    (char *)surl + 6, "read", 36000, &outg))) {
			if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				if (strstr (soap.fault->faultstring, "STFN not found"))
					sav_errno = ENOENT;
				else
					sav_errno = ECOMM;
			} else
				sav_errno = ECOMM;
			soap_print_fault (&soap, stderr);
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
			return (NULL);
		}
		if ((*reqid = strdup (outg._cacheReturn)) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			return (NULL);
		}
	} else {
		if (soap_call_impl__create (&soap, srm_endpoint, "create",
		    (char *)surl + 6, zero, "x", 36000, &outp)) {
			soap_print_fault (&soap, stderr);
			soap_end (&soap);
			soap_done (&soap);
			errno = ECOMM;
			return (NULL);
		}
		if ((*reqid = strdup (outp._createReturn)) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			return (NULL);
		}
	}
	if (soap_call_impl__getTurl (&soap, srm_endpoint, "getTurl", *reqid,
	    &protoarray, &outq)) {
		soap_print_fault (&soap, stderr);
		soap_end (&soap);
		soap_done (&soap);
		return (NULL);
	}
	p = strdup (outq._getTurlReturn);
	soap_end (&soap);
	soap_done (&soap);
	return (p);
}

se_getfilemd (const char *surl, struct stat64 *statbuf)
{
	char *dp;
	int flags;
	struct group *gr;
	int i;
	struct impl__getMetadataResponse out;
	char *p;
	struct passwd *pw;
	char *q;
	int ret;
	int sav_errno;
	char *sfn;
	struct soap soap;
	char *srm_endpoint;

	if (parsesurl (surl, &srm_endpoint, &sfn) < 0)
		return (-1);

	soap_init (&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	/* issue "getMetadata" request */

	if ((ret = soap_call_impl__getMetadata (&soap, srm_endpoint,
	    "getMetadata", (char *)surl + 6, &out))) {
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			if (strstr (soap.fault->faultstring, "does not exist"))
				sav_errno = ENOENT;
			else
				sav_errno = ECOMM;
		} else
			sav_errno = ECOMM;
		soap_print_fault (&soap, stderr);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}
	memset (statbuf, 0, sizeof(struct stat64));
	statbuf->st_nlink = 1;
	statbuf->st_uid = 2;
	statbuf->st_gid = 2;
	for (i = 0; i < out._getMetadataReturn->__size; i++) {
		p = out._getMetadataReturn->__ptr[i];
		if ((q = strchr (p, ':'))) {
			*q = '\0';
			if (strcmp (p, "filesize") == 0)
				statbuf->st_size = strtoll (q + 1, &dp, 10);
			else if (strcmp (p, "atime") == 0)
				statbuf->st_atime = strtol (q + 1, &dp, 10);
			else if (strcmp (p, "mtime") == 0)
				statbuf->st_mtime = strtol (q + 1, &dp, 10);
			else if (strcmp (p, "ctime") == 0)
				statbuf->st_ctime = strtol (q + 1, &dp, 10);
		}
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

se_set_xfer_done (char *surl, char *reqid, int oflag)
{
	int flags;
	struct impl__commitResponse out;
	char *sfn;
	struct soap soap;
	char *srm_endpoint;

	if (parsesurl (surl, &srm_endpoint, &sfn) < 0)
		return (-1);

	soap_init (&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif
	if (soap_call_impl__commit (&soap, srm_endpoint,
	    "commit", reqid, &out)) {
		soap_print_fault (&soap, stderr);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

se_set_xfer_running (char *surl, char *reqid)
{
	return (0);
}
