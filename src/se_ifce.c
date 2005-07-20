/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: se_ifce.c,v $ $Revision: 1.8 $ $Date: 2005/07/20 07:30:38 $ CERN Jean-Philippe Baud
 */

#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include "gfal_api.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "seH.h"
#include "edg_se_webserviceSoapBinding+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "seC.c"
#include "seClient.c"

static int
se_init (struct soap *soap, const char *surl, char *srm_endpoint,
	int srm_endpointsz, char *errbuf, int errbufsz)
{
	int flags;
	char *sfn;

	if (parsesurl (surl, srm_endpoint, srm_endpointsz, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	soap_init (soap);
	soap->namespaces = namespaces_se;
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif
	return (0);
}

se_deletesurl (const char *surl, char *errbuf, int errbufsz, int timeout)
{
	struct ns1__deleteResponse out;
	struct soap soap;
	char srm_endpoint[256];

	if (se_init (&soap, surl, srm_endpoint, sizeof(srm_endpoint),
	    errbuf, errbufsz) < 0)
		return (-1);
        soap.send_timeout = timeout ;
        soap.recv_timeout = timeout ;

	/* issue "delete" request */

	if (soap_call_ns1__delete (&soap, srm_endpoint,
	    "delete", (char *)surl + 6, &out)) {
                if (soap.error == SOAP_EOF) {
                        gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
                        soap_end (&soap);
                        soap_done (&soap);
                        return (-1);
                }
		gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

se_mkdir (const char *dir, char *errbuf, int errbufsz, int timeout)
{
	struct ns1__mkdirResponse out;
	int ret;
	int sav_errno;
	struct soap soap;
	char srm_endpoint[256];

	if (se_init (&soap, dir, srm_endpoint, sizeof(srm_endpoint),
	    errbuf, errbufsz) < 0)
		return (-1);
        soap.send_timeout = timeout ;
        soap.recv_timeout = timeout ;
	
	/* issue "mkdir" request */
	if ((ret = soap_call_ns1__mkdir (&soap, srm_endpoint,
	    "mkdir", (char *)dir + 6, &out))) {
                if (soap.error == SOAP_EOF) {
                        gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
                        soap_end (&soap);
                        soap_done (&soap);
                        return (-1);
                }

		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			if (strstr (soap.fault->faultstring, "does not exist"))
				sav_errno = ENOENT;
			else
				sav_errno = ECOMM;
		} else
			sav_errno = ECOMM;
		if (sav_errno == ECOMM) 
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

int
se_makedirp (const char *surl, char *errbuf, int errbufsz, int timeout)
{
	int c;
	char *lastslash = NULL;
	char *p;
	char *p1;
	char *q;
	char sav_surl[1104];
	struct stat64 statbuf;

	if (strlen (surl) >= sizeof(sav_surl)) {
		gfal_errmsg(errbuf, errbufsz, "Source URL too long.");
		errno = ENAMETOOLONG;
		return (-1);
	}
	strcpy (sav_surl, surl);
	p1 = strchr (sav_surl + 6, '/');
	p = strrchr (sav_surl + 6, '/');
	while (p > p1) {
		if (lastslash == NULL) lastslash = p;
		*p = '\0';
		c = se_getfilemd (sav_surl, &statbuf, errbuf, errbufsz, timeout);
		if (c == 0) {
			*p = '/';
			break;
		}
		if (errno != ENOENT)
			return (c);
		q = strrchr (sav_surl, '/');
		*p = '/';
		p = q;
	}
	c = 0;
	while (c == 0 && (p = strchr (p + 1, '/')) && p <= lastslash) {
		*p = '\0';
		c = se_mkdir (sav_surl, errbuf, errbufsz, timeout);
		*p = '/';
	}
	return (c);
}

char *
se_turlfromsurl (const char *surl, char **protocols, int oflag, int *reqid,
	int *fileid, char **token, char *errbuf, int errbufsz, int timeout)
{
	int nbproto = 0;
	struct ns1__cacheResponse outg;
	struct ns1__createResponse outp;
	struct ns1__getTurlResponse outq;
	char *p;
	struct ArrayOf_USCORExsd_USCOREstring protoarray;
	int ret;
	int sav_errno;
	struct soap soap;
	char srm_endpoint[256];
	LONG64 zero = 0;

	if (se_init (&soap, surl, srm_endpoint, sizeof(srm_endpoint),
	    errbuf, errbufsz) < 0)
		return (NULL);
        soap.send_timeout = timeout ;
        soap.recv_timeout = timeout ;

	while (*protocols[nbproto]) nbproto++;

	/* issue "cache" or the "create" request */

	protoarray.__ptr = protocols;
	protoarray.__size = nbproto;

	if ((oflag & O_ACCMODE) == 0) {
		if ((ret = soap_call_ns1__cache (&soap, srm_endpoint, "cache",
		    (char *)surl + 6, "read", 36000, &outg))) {
	                if (soap.error == SOAP_EOF) {
        	                gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
                	        soap_end (&soap);
                        	soap_done (&soap);
                        	return (NULL);
                	}

			if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				if (strstr (soap.fault->faultstring, "STFN not found"))
					sav_errno = ENOENT;
				else
					sav_errno = ECOMM;
			} else
				sav_errno = ECOMM;
			if (errno == ECOMM)
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
			return (NULL);
		}
		if ((*token = strdup (outg._cacheReturn)) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			return (NULL);
		}
	} else {
retry:
		if ((ret = soap_call_ns1__create (&soap, srm_endpoint, "create",
		    (char *)surl + 6, zero, "x", 36000, &outp))) {
	                if (soap.error == SOAP_EOF) {
        	                gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
                	        soap_end (&soap);
                        	soap_done (&soap);
                        	return (NULL);
                	}
			if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				if (strstr (soap.fault->faultstring, "o such file"))
					sav_errno = ENOENT;
					if (se_makedirp (surl, errbuf, errbufsz, timeout) == 0)
						goto retry;
				else if (strstr (soap.fault->faultstring, "File exists"))
					sav_errno = EEXIST;
				else
					sav_errno = ECOMM;
			} else
				sav_errno = ECOMM;
			if (errno == ECOMM)
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
			return (NULL);
		}
		if ((*token = strdup (outp._createReturn)) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			return (NULL);
		}
	}
	if (soap_call_ns1__getTurl (&soap, srm_endpoint, "getTurl", *token,
	    &protoarray, &outq)) {
                if (soap.error == SOAP_EOF) {
                        gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
                        soap_end (&soap);
                        soap_done (&soap);
                        return (NULL);
                }
		gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		return (NULL);
	}
	*reqid = 0;
	*fileid = 0;
	p = strdup (outq._getTurlReturn);
	soap_end (&soap);
	soap_done (&soap);
	return (p);
}

se_getfilemd (const char *surl, struct stat64 *statbuf, char *errbuf, int errbufsz, int timeout)
{
	char *dp;
	struct group *gr;
	int i;
	struct ns1__getMetadataResponse out;
	char *p;
	struct passwd *pw;
	char *q;
	int ret;
	int sav_errno;
	struct soap soap;
	char srm_endpoint[256];

	if (se_init (&soap, surl, srm_endpoint, sizeof(srm_endpoint),
	    errbuf, errbufsz) < 0)
		return (-1);
        soap.send_timeout = timeout;
        soap.recv_timeout = timeout;

	/* issue "getMetadata" request */

	if ((ret = soap_call_ns1__getMetadata (&soap, srm_endpoint,
	    "getMetadata", (char *)surl + 6, &out))) {
                if (soap.error == SOAP_EOF) {
                        gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
                        soap_end (&soap);
                        soap_done (&soap);
                        return (-1);
                }

		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			if (strstr (soap.fault->faultstring, "does not exist"))
				sav_errno = ENOENT;
			else if (strstr (soap.fault->faultstring, "expired"))
				sav_errno = EACCES;
			else
				sav_errno = ECOMM;
		} else
			sav_errno = ECOMM;
		if (sav_errno == ECOMM)
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
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

se_set_xfer_done (const char *surl, int reqid, int fileid, char *token,
	int oflag, char *errbuf, int errbufsz, int timeout)
{
	struct ns1__abandonResponse outa;
	struct ns1__commitResponse outc;
	struct soap soap;
	char srm_endpoint[256];

	if (se_init (&soap, surl, srm_endpoint, sizeof(srm_endpoint),
	    errbuf, errbufsz) < 0)
		return (-1);
        soap.send_timeout = timeout ;
        soap.recv_timeout = timeout ;


	if ((oflag & O_ACCMODE) == 0) {
/*		not implemented yet 
		if (soap_call_ns1__abandon (&soap, srm_endpoint,
		    "abandon", token, &outa)) {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
*/
	} else {
		if (soap_call_ns1__commit (&soap, srm_endpoint,
		    "commit", token, &outc)) {
                       if (soap.error == SOAP_EOF) {
	                        gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
        	                soap_end (&soap);
                	        soap_done (&soap);
                        	return (-1);
			}
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

se_set_xfer_running (const char *surl, int reqid, int fileid, char *token,
	char *errbuf, int errbufsz)
{
	return (0);
}
