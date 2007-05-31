/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: se_ifce.c,v $ $Revision: 1.11 $ $Date: 2007/05/31 14:02:59 $ CERN Jean-Philippe Baud
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

static int errorstring2errno (const char *);

se_deletesurls (int nbfiles, const char **surls, const char *srm_endpoint,
		struct se_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	int ret;
	int flags;
	struct ns1__deleteResponse out;
	struct soap soap;
	int i;

	if ((*filestatuses = (struct se_filestatus *) calloc (nbfiles, sizeof (struct se_filestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	soap_init (&soap);
	soap.namespaces = namespaces_se;
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;

	/* issue "delete" requests */

	for (i = 0; i < nbfiles; ++i) {
		bzero (*filestatuses + i, sizeof (struct se_filestatus));
		(*filestatuses)[i].surl = strdup (surls[i]);

		if ((ret = soap_call_ns1__delete (&soap, srm_endpoint,
					"delete", (char *)(surls[i] + 6), &out))) {
			char errmsg[ERRMSG_LEN];
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				(*filestatuses)[i].status = ECOMM;
			} else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				(*filestatuses)[i].status = errorstring2errno (soap.fault->faultstring);
			} else {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Unknown error", srm_endpoint);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				(*filestatuses)[i].status = ECOMM;
			}
		}
	}
	soap_end (&soap);
	soap_done (&soap);
	return (nbfiles);
}

se_mkdir (const char *dir, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	struct ns1__mkdirResponse out;
	int ret;
	int sav_errno;
	struct soap soap;

	soap_init (&soap);
	soap.namespaces = namespaces_se;
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;

	/* issue "mkdir" request */
	if ((ret = soap_call_ns1__mkdir (&soap, srm_endpoint,
					"mkdir", (char *)dir + 6, &out))) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			sav_errno = ECOMM;
		} else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			sav_errno = errorstring2errno (soap.fault->faultstring);
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Unknown error", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
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

static int
se_makedirp (const char *surl, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	int c;
	char *lastslash = NULL;
	char *p;
	char *p1;
	char *q;
	char sav_surl[1104];
	struct se_mdfilestatus *mdstatuses;

	if (strlen (surl) >= sizeof(sav_surl)) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: SURL too long", surl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	strcpy (sav_surl, surl);
	p1 = strchr (sav_surl + 6, '/');
	p = strrchr (sav_surl + 6, '/');
	while (p > p1) {
		if (lastslash == NULL) lastslash = p;
		*p = '\0';
		c = se_getfilemd (1, (const char **) &sav_surl, srm_endpoint, &mdstatuses, errbuf, errbufsz, timeout);
		if (mdstatuses[0].surl) free (mdstatuses[0].surl);
		if (mdstatuses[0].status != ENOENT) {
			errno = mdstatuses[0].status;
			free (mdstatuses);
			return (c);
		}
		free (mdstatuses);
		if (c == 0) {
			*p = '/';
			break;
		}
		q = strrchr (sav_surl, '/');
		*p = '/';
		p = q;
	}
	c = 0;
	while (c == 0 && (p = strchr (p + 1, '/')) && p <= lastslash) {
		*p = '\0';
		c = se_mkdir (sav_surl, srm_endpoint, errbuf, errbufsz, timeout);
		*p = '/';
	}
	return (c);
}

se_turlsfromsurls (int nbfiles, const char **surls, const char *srm_endpoint, char **protocols, int oflag,
		struct se_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int nbproto = 0;
	struct ns1__cacheResponse outg;
	struct ns1__createResponse outp;
	struct ns1__getTurlResponse outq;
	char *p;
	struct ArrayOf_USCORExsd_USCOREstring protoarray;
	int ret, i;
	int sav_errno;
	struct soap soap;
	LONG64 zero = 0;

	if ((*filestatuses = (struct se_filestatus *) calloc (nbfiles, sizeof (struct se_filestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	soap_init (&soap);
	soap.namespaces = namespaces_se;
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;

	while (*protocols[nbproto]) nbproto++;

	/* issue "cache" or the "create" request */

	protoarray.__ptr = protocols;
	protoarray.__size = nbproto;

	for (i = 0; i < nbfiles; ++i) {
		bzero (*filestatuses + i, sizeof (struct se_filestatus));
		(*filestatuses)[i].surl = strdup (surls[i]);

		if ((oflag & O_ACCMODE) == 0) {
			if ((ret = soap_call_ns1__cache (&soap, srm_endpoint, "cache",
							(char *)(surls[i] + 6), "read", 36000, &outg))) {
				char errmsg[ERRMSG_LEN];
				if (soap.error == SOAP_EOF) {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
					gfal_errmsg(errbuf, errbufsz, errmsg);
					(*filestatuses)[i].status = ECOMM;
				} else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
					gfal_errmsg(errbuf, errbufsz, errmsg);
					(*filestatuses)[i].status = errorstring2errno (soap.fault->faultstring);
				} else {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: Unknown error", srm_endpoint);
					gfal_errmsg(errbuf, errbufsz, errmsg);
					(*filestatuses)[i].status = ECOMM;
				}
				continue;
			}

			if (((*filestatuses)[i].token = strdup (outg._cacheReturn)) == NULL) {
				(*filestatuses)[i].status = ENOMEM;
				continue;
			}
		} else {
			if (se_makedirp (surls[i], srm_endpoint, errbuf, errbufsz, timeout) == 0) {
				(*filestatuses)[i].status = errno;
				errno = 0;
				continue;
			}
			if ((ret = soap_call_ns1__create (&soap, srm_endpoint, "create",
							(char *)(surls[i] + 6), zero, "x", 36000, &outp))) {
				char errmsg[ERRMSG_LEN];
				if (soap.error == SOAP_EOF) {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
					gfal_errmsg(errbuf, errbufsz, errmsg);
					(*filestatuses)[i].status = ECOMM;
				} else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
					gfal_errmsg(errbuf, errbufsz, errmsg);
					(*filestatuses)[i].status = errorstring2errno (soap.fault->faultstring);
				} else {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: Unknown error", srm_endpoint);
					gfal_errmsg(errbuf, errbufsz, errmsg);
					(*filestatuses)[i].status = ECOMM;
				}
				continue;
			}
			if (((*filestatuses)[i].token = strdup (outp._createReturn)) == NULL) {
				(*filestatuses)[i].status = ENOMEM;
				continue;
			}
		}

		if ((ret = soap_call_ns1__getTurl (&soap, srm_endpoint, "getTurl", (*filestatuses)[i].token,
						&protoarray, &outq))) {
			char errmsg[ERRMSG_LEN];
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				(*filestatuses)[i].status = ECOMM;
			} else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				(*filestatuses)[i].status = errorstring2errno (soap.fault->faultstring);
			} else {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Unknown error", srm_endpoint);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				(*filestatuses)[i].status = ECOMM;
			}
			continue;
		}

		(*filestatuses)[i].turl = strdup (outq._getTurlReturn);
	}

	soap_end (&soap);
	soap_done (&soap);
	return (nbfiles);
}

se_getfilemd (int nbfiles, const char **surls, const char *srm_endpoint,
		struct se_mdfilestatus **mdstatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	char *dp;
	struct group *gr;
	int i, j;
	struct ns1__getMetadataResponse out;
	char *p;
	struct passwd *pw;
	char *q;
	int ret;
	struct soap soap;

	if ((*mdstatuses = (struct se_mdfilestatus *) calloc (nbfiles, sizeof (struct se_mdfilestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	soap_init (&soap);
	soap.namespaces = namespaces_se;
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	/* issue "getMetadata" request */

	for (i = 0; i < nbfiles; ++i) {
		bzero (*mdstatuses + i, sizeof (struct se_mdfilestatus));
		(*mdstatuses)[i].surl = strdup (surls[i]);

		if ((ret = soap_call_ns1__getMetadata (&soap, srm_endpoint,
						"getMetadata", (char *)(surls[i] + 6), &out))) {
			char errmsg[ERRMSG_LEN];
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				(*mdstatuses)[i].status = ECOMM;
			} else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				(*mdstatuses)[i].status = errorstring2errno (soap.fault->faultstring);
			} else {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Unknown error", srm_endpoint);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				(*mdstatuses)[i].status = ECOMM;
			}
			continue;
		}

		(*mdstatuses)[i].stat.st_nlink = 1;
		(*mdstatuses)[i].stat.st_uid = 2;
		(*mdstatuses)[i].stat.st_gid = 2;
		(*mdstatuses)[i].stat.st_mode = 0666;
		for (j = 0; j < out._getMetadataReturn->__size; j++) {
			p = out._getMetadataReturn->__ptr[j];
			if ((q = strchr (p, ':'))) {
				*q = '\0';
				if (strcmp (p, "filesize") == 0)
					(*mdstatuses)[i].stat.st_size = strtoll (q + 1, &dp, 10);
				else if (strcmp (p, "atime") == 0)
					(*mdstatuses)[i].stat.st_atime = strtol (q + 1, &dp, 10);
				else if (strcmp (p, "mtime") == 0)
					(*mdstatuses)[i].stat.st_mtime = strtol (q + 1, &dp, 10);
				else if (strcmp (p, "ctime") == 0)
					(*mdstatuses)[i].stat.st_ctime = strtol (q + 1, &dp, 10);
			}
		}
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

se_set_xfer_done (const char *srm_endpoint, const char *token,
		int oflag, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int ret;
	struct soap soap;
	int sav_errno;

	soap_init (&soap);
	soap.namespaces = namespaces_se;
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK | CGSI_OPT_SSL_COMPATIBLE;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;


	if ((oflag & O_ACCMODE) == 0) {
		/*	not implemented yet 
		struct ns1__abandonResponse outa;

		if (soap_call_ns1__abandon (&soap, srm_endpoint,
					"abandon", token, &outa)) {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		*/
	} else {
		struct ns1__commitResponse outc;

		if ((ret = soap_call_ns1__commit (&soap, srm_endpoint,
					"commit", (char *) token, &outc))) {
			char errmsg[ERRMSG_LEN];
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				sav_errno = ECOMM;
			} else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				sav_errno = errorstring2errno (soap.fault->faultstring);
			} else {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Unknown error", srm_endpoint);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				sav_errno = ECOMM;
			}
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
			return (-1);
		}
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

se_set_xfer_running (const char *srm_endpoint, const char *token,
		char *errbuf, int errbufsz, int timeout)
{
	return (0);
}

static int
errorstring2errno (const char *errstr)
{
	if (!errstr)
		return (ECOMM);
	else if (strstr (errstr, "ile exists"))
		return (EEXIST);
	else if (strstr (errstr, "does not exist") ||
			strstr (errstr, "o such file") ||
			strstr (errstr, "not found") ||
			strstr (errstr, "could not get storage info by path"))
		return (ENOENT);
	else if (strstr (errstr, "ermission denied") ||
			strstr (errstr, "expired"))
		return (EACCES);
	else if (strstr (errstr, "nvalid arg"))
		return (EINVAL);
	else if (strstr (errstr, "rotocol"))
		return (EPROTONOSUPPORT);
	else if (strstr (errstr, "o space left on device"))
		return (ENOSPC);
	else
		return (ECOMM);
}
