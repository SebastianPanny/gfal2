/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: srm_ifce.c,v $ $Revision: 1.35 $ $Date: 2008/04/03 07:03:41 $ CERN Jean-Philippe Baud
 */

#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include "gfal_api.h"
#include "srmH.h"
#include "ISRM.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#define DEFPOLLINT 10

static int srm_set_xfer_status (const char *, const char *, int, int, char *, int, int);
static int errorstring2errno (const char *);


srm_deletesurls (int nbfiles, const char **surls, const char *srm_endpoint,
		struct srm_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	struct ns5__advisoryDeleteResponse out;
	int i, ret;
	int sav_errno = 0;
	int flags;
	struct soap soap;
	struct ArrayOfstring surlarray;

	soap_init (&soap);

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;


	/* issue "advisoryDelete" request */

	surlarray.__size = nbfiles;
	surlarray.__ptr = (char **) surls;

	if (ret = soap_call_ns5__advisoryDelete (&soap, srm_endpoint,
				"advisoryDelete", &surlarray, &out)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			sav_errno = ECOMM;
		} else if(soap.fault != NULL && soap.fault->faultstring != NULL) {
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

	if ((*filestatuses = (struct srm_filestatus *) calloc (nbfiles, sizeof (struct srm_filestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	for (i = 0; i < nbfiles; ++i) {
		memset (*filestatuses + i, 0, sizeof (struct srm_filestatus));
		(*filestatuses)[i].surl = strdup (surls[i]);
	}
	return (nbfiles);
}

srm_get (int nbfiles, const char **surls, int nbprotocols, char **protocols,
		int *reqid, char **token, struct srm_filestatus **filestatuses, int timeout)
{
	return (srm_getx (nbfiles, surls, nbprotocols, protocols, reqid,
				filestatuses, NULL, 0, timeout));
}

srm_getx (int nbfiles, const char **surls, int nbprotocols, char **protocols,
		int *reqid, struct srm_filestatus **filestatuses,
		char *errbuf, int errbufsz, int timeout)
{
	int i = 0;
	char **se_types;
	char **se_endpoints;
	char *srm_endpoint = NULL;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "srm_v1")) == 0) 
			srm_endpoint = se_endpoints[i];
		i++;
	}

	free (se_types);
	free (se_endpoints);

	if (! srm_endpoint) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: No matching SRMv1-compliant SE", surls[0]);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}

	return (srm_getxe (nbfiles, surls, srm_endpoint, protocols, reqid,
				filestatuses, errbuf, errbufsz, timeout));
}

srm_getxe (int nbfiles, const char **surls, const char *srm_endpoint,
		char **protocols, int *reqid, struct srm_filestatus **filestatuses,
		char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int errflag = 0;
	struct ns1__RequestFileStatus *f;
	int i, n;
	int nbproto = 0;
	struct ns5__getResponse outg;
	char *p;
	struct ArrayOfstring protoarray;
	struct ns1__RequestStatus *reqstatp;
	int ret;
	int sav_errno = 0;
	struct soap soap;
	struct ArrayOfstring surlarray;

	soap_init (&soap);

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;

	while (protocols[nbproto] && *protocols[nbproto]) nbproto++;
	if (!protocols[nbproto]) protocols[nbproto] = "";

	surlarray.__ptr = (char **)surls;
	surlarray.__size = nbfiles;
	protoarray.__ptr = protocols;
	protoarray.__size = nbproto;

	if (ret = soap_call_ns5__get (&soap, srm_endpoint, "get", &surlarray,
				&protoarray, &outg)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			sav_errno = ECOMM;
		} else if(soap.fault != NULL && soap.fault->faultstring != NULL) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg (errbuf, errbufsz, errmsg);
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

	if ((reqstatp = outg._Result) == NULL || reqstatp->fileStatuses == NULL) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}
	*reqid = reqstatp->requestId;

	n = reqstatp->fileStatuses->__size;

	if ((*filestatuses = calloc (n, sizeof(struct srm_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		f = reqstatp->fileStatuses->__ptr[i];
		if (f->SURL)
			(*filestatuses)[i].surl = strdup (f->SURL);
		if (f->state) {
			if (strcasecmp (f->state, "pending") == 0)
				(*filestatuses)[i].status = 0;
			else if (strcasecmp (f->state, "failed") == 0)
				(*filestatuses)[i].status = -1;
			else
				(*filestatuses)[i].status = 1;
		}
		(*filestatuses)[i].fileid = f->fileId;
		if (f->TURL)
			(*filestatuses)[i].turl = strdup (f->TURL);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srm_getstatus (int nbfiles, const char **surls, int reqid, char *token,
		struct srm_filestatus **filestatuses, int timeout)
{
	return (srm_getstatusx (nbfiles, surls, reqid, filestatuses, NULL, 0, timeout));
}

srm_getstatusx (int nbfiles, const char **surls, int reqid, struct srm_filestatus **filestatuses,
		char *errbuf, int errbufsz, int timeout)
{
	int i = 0;
	char **se_types;
	char **se_endpoints;
	char *srm_endpoint = NULL;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "srm_v1")) == 0) 
			srm_endpoint = se_endpoints[i];
		i++;
	}

	free (se_types);
	free (se_endpoints);

	if (! srm_endpoint) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: No matching SRMv1-compliant SE", surls[0]);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}

	return (srm_getstatusxe (reqid, srm_endpoint, filestatuses, errbuf, errbufsz, timeout));
}

srm_getstatusxe (int reqid, const char *srm_endpoint, struct srm_filestatus **filestatuses,
		char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int errflag = 0;
	struct ns1__RequestFileStatus *f;
	int i;
	int n;
	struct ns5__getRequestStatusResponse outq;
	char *p;
	struct ns1__RequestStatus *reqstatp;
	int ret;
	int sav_errno = 0;
	struct soap soap;
	struct ArrayOfstring surlarray;

	soap_init (&soap);

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;


	if (ret = soap_call_ns5__getRequestStatus (&soap, srm_endpoint,
				"getRequestStatus", reqid, &outq)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			sav_errno = ECOMM;
		} else if(soap.fault != NULL && soap.fault->faultstring != NULL) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg (errbuf, errbufsz, errmsg);
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

	if ((reqstatp = outq._Result) == NULL || reqstatp->fileStatuses == NULL) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	n = reqstatp->fileStatuses->__size;

	if ((*filestatuses = (struct srm_filestatus *) calloc (n, sizeof(struct srm_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		f = reqstatp->fileStatuses->__ptr[i];
		if (f->SURL)
			(*filestatuses)[i].surl = strdup (f->SURL);
		if (f->state) {
			if (strcasecmp (f->state, "pending") == 0)
				(*filestatuses)[i].status = 0;
			else if (strcasecmp (f->state, "failed") == 0)
				(*filestatuses)[i].status = -1;
			else
				(*filestatuses)[i].status = 1;
		}
		(*filestatuses)[i].fileid = f->fileId;
		if (f->TURL)
			(*filestatuses)[i].turl = strdup (f->TURL);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srm_turlsfromsurls (int nbfiles, const char **surls, const char *srm_endpoint, GFAL_LONG64 *filesizes, char **protocols,
		int oflag, int *reqid, struct srm_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int i, n;
	int nbproto = 0;
	struct ns5__getResponse outg;
	struct ns5__putResponse outp;
	struct ns5__getRequestStatusResponse outq;
	char *p;
	struct ArrayOfboolean permarray;
	struct ArrayOfstring protoarray;
	int r = 0;
	struct ns1__RequestStatus *reqstatp;
	int ret;
	int sav_errno = 0;
	struct ArrayOflong sizearray;
	struct soap soap;
	struct ArrayOfstring srcarray;
	struct ArrayOfstring surlarray;
	time_t endtime;

	soap_init (&soap);

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;


	while (protocols[nbproto] && *protocols[nbproto]) nbproto++;
	if (!protocols[nbproto]) protocols[nbproto] = "";

	/* issue "get" or the "put" request */

	surlarray.__ptr = (char **)surls;
	surlarray.__size = nbfiles;
	protoarray.__ptr = protocols;
	protoarray.__size = nbproto;

	if ((oflag & O_ACCMODE) == 0) {
		if (ret = soap_call_ns5__get (&soap, srm_endpoint, "get", &surlarray,
					&protoarray, &outg)) {
			char errmsg[ERRMSG_LEN];
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg (errbuf, errbufsz, errmsg);
				sav_errno = ECOMM;
			} else if(soap.fault != NULL && soap.fault->faultstring != NULL) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg (errbuf, errbufsz, errmsg);
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
		reqstatp = outg._Result;
	} else {
		srcarray.__ptr = (char **)surls;
		srcarray.__size = nbfiles;
		sizearray.__ptr = filesizes;
		sizearray.__size = nbfiles;
		if ((permarray.__ptr =
					soap_malloc (&soap, nbfiles * sizeof(enum xsd__boolean))) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			errno = ENOMEM;
			return (-1);
		}
		for (i = 0; i< nbfiles; i++)
			permarray.__ptr[i] = true_;
		permarray.__size = nbfiles;
		if (ret = soap_call_ns5__put (&soap, srm_endpoint, "put", &srcarray,
					&surlarray, &sizearray, &permarray, &protoarray, &outp)) {
			char errmsg[ERRMSG_LEN];
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg (errbuf, errbufsz, errmsg);
				sav_errno = ECOMM;
			} else if(soap.fault != NULL && soap.fault->faultstring != NULL) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg (errbuf, errbufsz, errmsg);
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
		reqstatp = outp._Result;
	}
	if (reqstatp == NULL || reqstatp->fileStatuses == NULL) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}
	*reqid = reqstatp->requestId;

	/* wait for file "ready" */

	if (timeout > 0)
		endtime = time(NULL) + timeout;

	while (reqstatp && strcmp (reqstatp->state, "pending") == 0 ||
			strcmp (reqstatp->state, "Pending") == 0) {
		sleep ((r++ == 0) ? 1 : (reqstatp->retryDeltaTime > 0) ?
				reqstatp->retryDeltaTime : DEFPOLLINT);
		if (ret = soap_call_ns5__getRequestStatus (&soap, srm_endpoint,
					"getRequestStatus", *reqid, &outq)) {
			char errmsg[ERRMSG_LEN];
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg (errbuf, errbufsz, errmsg);
				sav_errno = ECOMM;
			} else if(soap.fault != NULL && soap.fault->faultstring != NULL) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg (errbuf, errbufsz, errmsg);
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
		reqstatp = outq._Result;

		/* check if user-supplied timeout has passed */
		if (timeout > 0 && time(NULL) > endtime) {
			char errmsg[ERRMSG_LEN];

			soap_end(&soap);
			soap_done(&soap);

			if (reqstatp == NULL || reqstatp->fileStatuses != NULL && reqstatp->fileStatuses->__ptr != NULL)
				for (i = 0; i < reqstatp->fileStatuses->__size; ++i)
					srm_set_xfer_done (srm_endpoint, *reqid, reqstatp->fileStatuses->__ptr[i]->fileId,
							errbuf, errbufsz, timeout);

			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Waiting for Pending SRM request timed out", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			return (-1);
		}

	}
	if (reqstatp == NULL || reqstatp->state == NULL || strcasecmp (reqstatp->state, "failed") == 0) {
		char errmsg[ERRMSG_LEN];
		if (reqstatp->errorMessage) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, reqstatp->errorMessage);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			sav_errno = errorstring2errno (reqstatp->errorMessage);
		} else  {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: SRM request failed, but no errorMessage supplied", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			sav_errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	n = reqstatp->fileStatuses->__size;

	if ((*statuses = (struct srm_filestatus *) calloc (n, sizeof(struct srm_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		memset (*statuses + i, 0, sizeof (struct srm_filestatus));
		if (reqstatp->fileStatuses->__ptr[i]->SURL)
			(*statuses)[i].surl = strdup (reqstatp->fileStatuses->__ptr[i]->SURL);
		(*statuses)[i].fileid = reqstatp->fileStatuses->__ptr[i]->fileId;
		if (!strcasecmp (reqstatp->fileStatuses->__ptr[i]->state, "ready") &&
				reqstatp->fileStatuses->__ptr[i]->TURL) {
			(*statuses)[i].turl = strdup (reqstatp->fileStatuses->__ptr[i]->TURL);
			(*statuses)[i].status = 0;
		} else {
			(*statuses)[i].status = ENOENT;
		}
	}
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
srm_getfilemd (int nbfiles, const char **surls, const char *srm_endpoint,
		struct srm_mdfilestatus **mdstatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	struct group *gr;
	struct ns5__getFileMetaDataResponse out;
	struct passwd *pw;
	int ret;
	int sav_errno = 0;
	struct soap soap;
	struct ArrayOfstring surlarray;
	int i, n;

	soap_init (&soap);

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;


	/* issue "getFileMetaData" request */

	surlarray.__size = nbfiles;
	surlarray.__ptr = (char **) surls;

	if ((ret = soap_call_ns5__getFileMetaData (&soap, srm_endpoint,
					"getFileMetaData", &surlarray, &out))) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			sav_errno = ECOMM;
		} else if (soap.fault != NULL && soap.fault->faultstring != NULL) {
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
	if (out._Result->__size == 0 || out._Result->__ptr[0]->SURL == NULL) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	n = out._Result->__size;

	if ((*mdstatuses = (struct srm_mdfilestatus *) calloc (n, sizeof (struct srm_mdfilestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; ++i) {
		if (out._Result->__ptr[i]->SURL)
			(*mdstatuses)[i].surl = strdup (out._Result->__ptr[i]->SURL);
		(*mdstatuses)[i].stat.st_mode = out._Result->__ptr[i]->permMode;
		if (((*mdstatuses)[i].stat.st_mode & S_IFMT) == 0)
			(*mdstatuses)[i].stat.st_mode |= S_IFREG;
		if (out._Result->__ptr[i]->owner && (pw = getpwnam (out._Result->__ptr[i]->owner)))
			(*mdstatuses)[i].stat.st_uid = pw->pw_uid;
		else
			(*mdstatuses)[i].stat.st_uid = 2;
		if (out._Result->__ptr[i]->group && (gr = getgrnam (out._Result->__ptr[i]->group)))
			(*mdstatuses)[i].stat.st_gid = gr->gr_gid;
		else
			(*mdstatuses)[i].stat.st_gid = 2;
		(*mdstatuses)[i].stat.st_size = out._Result->__ptr[i]->size;
		(*mdstatuses)[i].stat.st_nlink = 1;
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);
}
#endif

srm_set_xfer_done (const char *srm_endpoint, int reqid, int fileid,	char *errbuf, int errbufsz, int timeout)
{
	return srm_set_xfer_status ("Done", srm_endpoint, reqid, fileid, errbuf, errbufsz, timeout);
}

srm_set_xfer_running (const char *srm_endpoint, int reqid, int fileid, char *errbuf, int errbufsz, int timeout)
{
	return srm_set_xfer_status ("Running", srm_endpoint, reqid, fileid, errbuf, errbufsz, timeout);
}

	static int
srm_set_xfer_status (const char *status, const char *srm_endpoint, int reqid, int fileid,
		char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	struct ns5__setFileStatusResponse out;
	struct ns1__RequestStatus *reqstat;
	int ret;
	struct soap soap;

	soap_init (&soap);

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;


	if (ret = soap_call_ns5__setFileStatus (&soap, srm_endpoint,
				"setFileStatus", reqid, fileid, (char *) status, &out)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			sav_errno = ECOMM;
		} else if(soap.fault != NULL && soap.fault->faultstring != NULL) {
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

	if ((reqstat = out._Result) == NULL || reqstat->state == NULL || strcasecmp (reqstat->state, "failed") == 0) {
		char errmsg[ERRMSG_LEN];
		if (reqstat->errorMessage) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, reqstat->errorMessage);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			sav_errno = errorstring2errno (reqstat->errorMessage);
		} else  {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: SRM request failed, but no errorMessage supplied", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
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
errorstring2errno (const char *errstr)
{
	if (!errstr)
		return (ECOMM);
	else if (strstr (errstr, "ile exists"))
		return (EEXIST);
	else if (strstr (errstr, "does not exist") ||
			strstr (errstr, "o such file or directory") ||
			strstr (errstr, "could not get storage info by path"))
		return (ENOENT);
	else if (strstr (errstr, "ermission denied"))
		return (EACCES);
	else if (strstr (errstr, "nvalid arg") ||
			strstr (errstr, "nknown"))
		return (EINVAL);
	else if (strstr (errstr, "rotocol"))
		return (EPROTONOSUPPORT);
	else if (strstr (errstr, "o space left on device"))
		return (ENOSPC);
	else
		return (ECOMM);
}
