/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: srm2_2_ifce.c,v $ $Revision: 1.15 $ $Date: 2007/06/11 15:12:59 $ CERN Jean-Philippe Baud
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
#include "srmv2H.h"
#include "srmSoapBinding+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#define DEFPOLLINT 10
#include "srmv2C.c"
#include "srmv2Client.c"

static char lastcreated_dir[1024] = "";

srmv2_deletesurls (int nbfiles, const char **surls, const char *srm_endpoint,
		struct srmv2_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int ret;
	struct ns1__srmRmResponse_ rep;
	struct ns1__ArrayOfTSURLReturnStatus *repfs;
	struct ns1__srmRmRequest req;
	struct ns1__TReturnStatus *reqstatp;
	int s, i, n;
	struct soap soap;

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;

	memset (&req, 0, sizeof(req));

	/* NOTE: only one file in the array */
	if ((req.arrayOfSURLs = soap_malloc (&soap, sizeof(struct ns1__ArrayOfAnyURI))) == NULL) {

		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	/* issue "srmRm" request */

	if (ret = soap_call_ns1__srmRm (&soap, srm_endpoint, "srmRm", &req, &rep)) {
		char errmsg[ERRMSG_LEN];

		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		} else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmRmResponse->returnStatus;
	repfs = rep.srmRmResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray ||
			!repfs->statusArray[0] || !repfs->statusArray[0]->status) {

		char errmsg[ERRMSG_LEN];

		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			errno = statuscode2errno(reqstatp->statusCode);
			if (reqstatp->explanation) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, reqstatp->explanation);
				gfal_errmsg(errbuf, errbufsz, errmsg);
			}
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ECOMM;
		}

		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*statuses = (struct srmv2_filestatus*) calloc (n, sizeof (struct srmv2_filestatus))) == NULL) {
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	for (i = 0; i < n; ++i) {
		if (repfs->statusArray[0]->surl)
			(*statuses)[i].surl = strdup (repfs->statusArray[i]->surl);
		if (repfs->statusArray[0]->status) {
			(*statuses)[i].status = statuscode2errno(repfs->statusArray[0]->status->statusCode);
			if (repfs->statusArray[0]->status->explanation)
				(*statuses)[i].explanation = strdup (repfs->statusArray[0]->status->explanation);
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (i);
}

srmv2_get (int nbfiles, const char **surls, const char *spacetokendesc, int nbprotocols, char **protocols,
		char **reqtoken, struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	char **se_types;
	char **se_endpoints;
	char *srm_endpoint = NULL;
	int i, r;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "srm_v2")) == 0)
			srm_endpoint = se_endpoints[i];
		i++;
	}

	free (se_types);
	free (se_endpoints);

	if (! srm_endpoint) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: No matching SRMv2.2-compliant SE", surls[0]);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}

	r = srmv2_gete (nbfiles, surls, srm_endpoint, spacetokendesc, nbprotocols, protocols, reqtoken,
			filestatuses, errbuf, errbufsz, timeout);

	free (srm_endpoint);
	return (r);
}

srmv2_gete (int nbfiles, const char **surls, const char *srm_endpoint, const char *spacetokendesc, int nbprotocols,
		char **protocols, char **reqtoken, struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int n;
	int rc;
	int ret;
	struct ns1__srmPrepareToGetResponse_ rep;
	struct ns1__ArrayOfTGetRequestFileStatus *repfs;
	struct ns1__srmPrepareToGetRequest req;
	struct ns1__TGetFileRequest *reqfilep;
	struct ns1__TReturnStatus *reqstatp;
	struct soap soap;
	static enum ns1__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	int i = 0;
	char *targetspacetoken;

retry:

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	/* issue "get" request */

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfFileRequests = 
				soap_malloc (&soap, sizeof(struct ns1__ArrayOfTGetFileRequest))) == NULL || 
			(req.arrayOfFileRequests->requestArray = 
			 soap_malloc (&soap, nbfiles * sizeof(struct ns1__TGetFileRequest *))) == NULL ||
			(req.transferParameters = 
			 soap_malloc (&soap, sizeof(struct ns1__TTransferParameters))) == NULL || 
			(req.targetSpaceToken = 
			 soap_malloc (&soap, sizeof(char *))) == NULL) { 

		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	/* get first space token from user space token description */
	if (!spacetokendesc) {
		req.targetSpaceToken = NULL;
	} else if ((targetspacetoken = srmv2_getspacetoken (spacetokendesc, srm_endpoint, errbuf, errbufsz, timeout)) == NULL) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid space token description", spacetokendesc);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	} else {
		req.targetSpaceToken = strdup(targetspacetoken);
		free (targetspacetoken);
	}

	req.authorizationID = NULL;
	req.userRequestDescription = NULL;
	req.storageSystemInfo = NULL;
	req.desiredFileStorageType = &s_types[PERMANENT];
	req.desiredTotalRequestTime = NULL;
	req.desiredPinLifeTime = NULL;
	req.targetFileRetentionPolicyInfo = NULL;

	for (i = 0; i < nbfiles; i++) {
		if ((req.arrayOfFileRequests->requestArray[i] = 
					soap_malloc (&soap, sizeof(struct ns1__TGetFileRequest))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
	}

	req.arrayOfFileRequests->__sizerequestArray = nbfiles;

	for (i = 0; i < nbfiles; i++) {
		reqfilep = req.arrayOfFileRequests->requestArray[i];
		memset (reqfilep, 0, sizeof(*reqfilep));
		reqfilep->sourceSURL = (char *) surls[i];
		reqfilep->dirOption = NULL;
	}

	req.transferParameters->accessPattern = NULL;
	req.transferParameters->connectionType = NULL;
	req.transferParameters->arrayOfClientNetworks = NULL;

	if ((req.transferParameters->arrayOfTransferProtocols =
				soap_malloc (&soap, nbprotocols * sizeof(struct ns1__ArrayOfString))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.transferParameters->arrayOfTransferProtocols->__sizestringArray = nbprotocols;
	req.transferParameters->arrayOfTransferProtocols->stringArray = protocols;

	if (ret = soap_call_ns1__srmPrepareToGet (&soap, srm_endpoint, "PrepareToGet", &req, &rep)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	/* return request token */
	*reqtoken = strdup(rep.srmPrepareToGetResponse->requestToken);

	/* return file statuses */
	reqstatp = rep.srmPrepareToGetResponse->returnStatus;
	repfs = rep.srmPrepareToGetResponse->arrayOfFileStatuses;

	if (! repfs) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		filestatuses = NULL;
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_filestatus *) calloc (n, sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		bzero (*filestatuses + i, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[0]->sourceSURL)
			(*filestatuses)[i].surl = strdup ((repfs->statusArray[i])->sourceSURL);
		if (repfs->statusArray[i]->transferURL)
			(*filestatuses)[i].turl = strdup ((repfs->statusArray[i])->transferURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			if (repfs->statusArray[i]->status->explanation)
				(*filestatuses)[i].explanation = strdup ((repfs->statusArray[i])->status->explanation);
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);	
}

srmv2_getstatus (int nbfiles, const char **surls, const char *reqtoken, struct srmv2_filestatus **filestatuses, 
		char *errbuf, int errbufsz, int timeout)
{
	char **se_types;
	char **se_endpoints;
	char *srm_endpoint;
	int i, r;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "srm_v2")) == 0) {
			srm_endpoint = se_endpoints[i];
		}
		i++;
	}

	free (se_types);
	free (se_endpoints);

	if (! srm_endpoint) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: No matching SRMv2.2-compliant SE", surls[0]);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}

	r = srmv2_getstatuse (reqtoken, srm_endpoint, filestatuses, errbuf, errbufsz, timeout);

	free (srm_endpoint);
	return (r);
}

srmv2_getstatuse (const char *reqtoken, const char *srm_endpoint,
		struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int i = 0;
	int n;
	int rc;
	int ret;
	struct ns1__ArrayOfTGetRequestFileStatus *repfs;
	struct ns1__TReturnStatus *reqstatp;
	struct soap soap;
	struct ns1__srmStatusOfGetRequestResponse_ srep;
	struct ns1__srmStatusOfGetRequestRequest sreq;

retry:

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	memset (&sreq, 0, sizeof(sreq));
	sreq.requestToken = (char *) reqtoken;

	if (ret = soap_call_ns1__srmStatusOfGetRequest (&soap, srm_endpoint, "StatusOfGetRequest", &sreq, &srep)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	/* return file statuses */
	reqstatp = srep.srmStatusOfGetRequestResponse->returnStatus;
	repfs = srep.srmStatusOfGetRequestResponse->arrayOfFileStatuses;

	if (! repfs) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		filestatuses = NULL;
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_filestatus *) calloc (n, sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		bzero (*filestatuses + i, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[0]->sourceSURL)
			(*filestatuses)[i].surl = strdup ((repfs->statusArray[i])->sourceSURL);
		if (repfs->statusArray[i]->transferURL)
			(*filestatuses)[i].turl = strdup ((repfs->statusArray[i])->transferURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			if (repfs->statusArray[i]->status->explanation)
				(*filestatuses)[i].explanation = strdup ((repfs->statusArray[i])->status->explanation);
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

/* returns first (!) space token in ArrayOfSpaceTokens */
	char *
srmv2_getspacetoken (const char *spacetokendesc, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int ret;
	struct soap soap;
	char *spacetoken;
	struct ns1__srmGetSpaceTokensResponse_ tknrep;
	struct ns1__srmGetSpaceTokensRequest tknreq;
	struct ns1__TReturnStatus *tknrepstatp;
	struct ns1__ArrayOfString *tknrepp;

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	memset (&tknreq, 0, sizeof(tknreq));

	tknreq.userSpaceTokenDescription = (char *) spacetokendesc;

	if (ret = soap_call_ns1__srmGetSpaceTokens (&soap, srm_endpoint, "GetSpaceTokens", &tknreq, &tknrep)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		return (NULL);
	}

	tknrepstatp = tknrep.srmGetSpaceTokensResponse->returnStatus;

	if (tknrepstatp->statusCode != SRM_USCORESUCCESS) {
		errno = statuscode2errno(tknrepstatp->statusCode);
		if (tknrepstatp->explanation) {
			char errmsg[ERRMSG_LEN];
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, tknrepstatp->explanation);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}

		soap_end (&soap);
		soap_done (&soap);
		return (NULL);
	}

	tknrepp = tknrep.srmGetSpaceTokensResponse->arrayOfSpaceTokens;

	if (! tknrepp) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		return (NULL);
	}

	spacetoken = strdup(tknrepp->stringArray[0]);

	soap_end (&soap);
	soap_done (&soap);

	return (spacetoken);
}

/* tries to create all directories in 'dest_file' */
	static int
srmv2_makedirp (const char *dest_file, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	int c = 0;
	char file[1024];
	int flags;
	int nbslash = 0;
	int sav_errno = 0;
	char *p;
	struct ns1__srmMkdirResponse_ rep;
	struct ns1__srmMkdirRequest req;
	struct ns1__TReturnStatus *repstatp;
	int ret = 0;
	struct soap soap;
	char errmsg[ERRMSG_LEN];

	if (!strncmp (lastcreated_dir, dest_file, 1024))
		/* this is exactly the same directory as the previous one, so nothing to do */
		return (0);

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	strncpy (file, dest_file, 1023);

	/* First of all, check if there is nothing to do (if last sub-directory exists or not) */
	if ((p = strrchr (file, '/')) == NULL) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid SURL", dest_file);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = EINVAL;
		return (-1);
	}
	*p = 0;
	req.SURL = file;

	if ((ret = soap_call_ns1__srmMkdir (&soap, srm_endpoint, "Mkdir", &req, &rep))) {
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		errno = ECOMM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	repstatp = rep.srmMkdirResponse->returnStatus;

	if (repstatp->statusCode != SRM_USCORESUCCESS) {
		sav_errno = statuscode2errno(repstatp->statusCode);
		if (sav_errno == EEXIST) {
			/* The directory already exists, notinhg to do */
			return (0);
		} else if (sav_errno != ENOENT) {
			if (repstatp->explanation)
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", dest_file, repstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", dest_file, strerror (sav_errno));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = sav_errno;
			ret = -1;
		}
	} else {
		/* The directory was successfuly created */
		return (0);
	}

	*p = '/';
	p = file;

	while (!ret && (p = strchr (p, '/'))) {
		*p = '\0';
		if (nbslash < 2) {	/* skip first 2 slashes */
			nbslash++;
			*p++ = '/';
			while (*p == '/')
				p++;
			continue;
		}

		/* try to create directory */
		memset (&req, 0, sizeof(req));
		req.SURL = (char *) file;

		if (ret = soap_call_ns1__srmMkdir (&soap, srm_endpoint, "Mkdir", &req, &rep)) {
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg(errbuf, errbufsz, errmsg);
			} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg(errbuf, errbufsz, errmsg);
			}
			errno = ECOMM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}

		repstatp = rep.srmMkdirResponse->returnStatus;
		ret = sav_errno = 0;

		if (repstatp->statusCode != SRM_USCORESUCCESS) {
			sav_errno = statuscode2errno(repstatp->statusCode);
			if (sav_errno != EEXIST && sav_errno != EACCES) {
				if (repstatp->explanation)
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", file, repstatp->explanation);
				else
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", file, strerror (sav_errno));
				gfal_errmsg(errbuf, errbufsz, errmsg);
				errno = sav_errno;
				ret = -1;
			}
		}

		*p++ = '/';	/* restore slash */
		while (!ret && *p == '/')
			p++;
	}

	if  (!sav_errno) {
		if (repstatp->explanation)
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", dest_file, repstatp->explanation);
		else
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", dest_file, strerror (sav_errno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = sav_errno;
		ret = -1;
	}

	soap_end (&soap);
	soap_done (&soap);
	strncpy (lastcreated_dir, dest_file, 1024);
	return (ret);
}

srmv2_prestage (int nbfiles, const char **surls, const char *spacetokendesc, int nbprotocols, char **protocols, char **reqtoken, 
		struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	char **se_types;
	char **se_endpoints;
	char *srm_endpoint;
	int i, r;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "srm_v2")) == 0)
			srm_endpoint = se_endpoints[i];
		i++;
	}

	free (se_types);
	free (se_endpoints);

	if (!srm_endpoint) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: No matching SRMv2.2-compliant SE", surls[0]);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}

	r = srmv2_prestagee (nbfiles, surls, spacetokendesc, srm_endpoint, nbprotocols, protocols, reqtoken,
			filestatuses, errbuf, errbufsz, timeout);

	free (srm_endpoint);
	return (r);
}

srmv2_prestagee (int nbfiles, const char **surls, const char *spacetokendesc, const char *srm_endpoint, int nbprotocols,
		char **protocols, char **reqtoken, struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int i = 0;
	int n;
	int rc;
	int ret;
	struct ns1__srmBringOnlineResponse_ rep;
	struct ns1__ArrayOfTBringOnlineRequestFileStatus *repfs;
	struct ns1__srmBringOnlineRequest req;
	struct ns1__TGetFileRequest *reqfilep;
	struct ns1__TReturnStatus *reqstatp;
	struct soap soap;
	static enum ns1__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	char *targetspacetoken;

retry:

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	/* issue "bringonline" request */

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfFileRequests = 
				soap_malloc (&soap, sizeof(struct ns1__ArrayOfTGetFileRequest))) == NULL || 
			(req.arrayOfFileRequests->requestArray = 
			 soap_malloc (&soap, nbfiles * sizeof(struct ns1__TGetFileRequest *))) == NULL ||
			(req.transferParameters = 
			 soap_malloc (&soap, sizeof(struct ns1__TTransferParameters))) == NULL || 
			(req.targetSpaceToken = 
			 soap_malloc (&soap, sizeof(char *))) == NULL) { 

		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	/* get first space token from user space token description */
	if (!spacetokendesc) {
		req.targetSpaceToken = NULL;
	} else if ((targetspacetoken = srmv2_getspacetoken (spacetokendesc, srm_endpoint, errbuf, errbufsz, timeout)) == NULL) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid space token description", spacetokendesc);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	} else {
		req.targetSpaceToken = strdup(targetspacetoken);
		free (targetspacetoken);
	}

	req.authorizationID = NULL;
	req.userRequestDescription = NULL;
	req.storageSystemInfo = NULL;
	req.desiredFileStorageType = &s_types[PERMANENT];
	req.desiredTotalRequestTime = NULL;
	req.desiredLifeTime = NULL;
	req.targetFileRetentionPolicyInfo = NULL;
	req.deferredStartTime = NULL;

	for (i = 0; i < nbfiles; i++) {
		if ((req.arrayOfFileRequests->requestArray[i] = 
					soap_malloc (&soap, sizeof(struct ns1__TGetFileRequest))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
	}

	req.arrayOfFileRequests->__sizerequestArray = nbfiles;

	for (i = 0; i < nbfiles; i++) {
		reqfilep = req.arrayOfFileRequests->requestArray[i];
		memset (reqfilep, 0, sizeof(*reqfilep));
		reqfilep->sourceSURL = (char *) surls[i];
		reqfilep->dirOption = NULL;
	}

	req.transferParameters->accessPattern = NULL;
	req.transferParameters->connectionType = NULL;
	req.transferParameters->arrayOfClientNetworks = NULL;

	if ((req.transferParameters->arrayOfTransferProtocols =
				soap_malloc (&soap, nbprotocols * sizeof(struct ns1__ArrayOfString))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.transferParameters->arrayOfTransferProtocols->__sizestringArray = nbprotocols;
	req.transferParameters->arrayOfTransferProtocols->stringArray = protocols;

	if (ret = soap_call_ns1__srmBringOnline (&soap, srm_endpoint, "BringOnline", &req, &rep)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	/* return request token */
	*reqtoken = strdup(rep.srmBringOnlineResponse->requestToken);

	/* return file statuses */
	reqstatp = rep.srmBringOnlineResponse->returnStatus;
	repfs = rep.srmBringOnlineResponse->arrayOfFileStatuses;

	if (! repfs) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		filestatuses = NULL;
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_filestatus *) calloc (n, sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		bzero (*filestatuses + i, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->sourceSURL)
			(*filestatuses)[i].surl = strdup ((repfs->statusArray[i])->sourceSURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			if (repfs->statusArray[i]->status->explanation)
				(*filestatuses)[i].explanation = strdup ((repfs->statusArray[i])->status->explanation);
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);	
}

srmv2_prestagestatus (int nbfiles, const char **surls, const char *reqtoken, struct srmv2_filestatus **filestatuses, 
		char *errbuf, int errbufsz, int timeout)
{
	char **se_types;
	char **se_endpoints;
	char *srm_endpoint;
	int i, r;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "srm_v2")) == 0)
			srm_endpoint = se_endpoints[i];
		i++;
	}

	free (se_types);
	free (se_endpoints);

	if (! srm_endpoint) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: No matching SRMv2.2-compliant SE", surls[0]);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}

	r = srmv2_prestagestatuse (srm_endpoint, reqtoken, filestatuses, errbuf, errbufsz, timeout);

	free (srm_endpoint);
	return (r);
}

srmv2_prestagestatuse (const char *srm_endpoint, const char *reqtoken, struct srmv2_filestatus **filestatuses, 
		char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int i = 0;
	int n;
	int r = 0;
	int rc;
	int ret;
	struct ns1__ArrayOfTBringOnlineRequestFileStatus *repfs;
	struct ns1__TReturnStatus *reqstatp;
	struct soap soap;
	struct ns1__srmStatusOfBringOnlineRequestResponse_ srep;
	struct ns1__srmStatusOfBringOnlineRequestRequest sreq;
	char *targetspacetoken;

retry:

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	memset (&sreq, 0, sizeof(sreq));
	sreq.requestToken = (char *) reqtoken;

	if (ret = soap_call_ns1__srmStatusOfBringOnlineRequest (&soap, srm_endpoint, "StatusOfBringOnlineRequest", &sreq, &srep)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = srep.srmStatusOfBringOnlineRequestResponse->returnStatus;
	repfs = srep.srmStatusOfBringOnlineRequestResponse->arrayOfFileStatuses;

	/* return file statuses */
	reqstatp = srep.srmStatusOfBringOnlineRequestResponse->returnStatus;
	repfs = srep.srmStatusOfBringOnlineRequestResponse->arrayOfFileStatuses;

	if (! repfs) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		filestatuses = NULL;
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_filestatus *) calloc (n, sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		bzero (*filestatuses + i, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->sourceSURL)
			(*filestatuses)[i].surl = strdup ((repfs->statusArray[i])->sourceSURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			if (repfs->statusArray[i]->status->explanation)
				(*filestatuses)[i].explanation = strdup ((repfs->statusArray[i])->status->explanation);
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);	
}

srmv2_set_xfer_done_get (int nbfiles, const char **surls, const char *srm_endpoint, const char *reqtoken,
		struct srmv2_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	return srmv2_release (nbfiles, surls, srm_endpoint, reqtoken, statuses, errbuf, errbufsz, timeout);
}

srmv2_set_xfer_done_put (int nbfiles, const char **surls, const char *srm_endpoint, const char *reqtoken,
		struct srmv2_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int ret;
	struct ns1__srmPutDoneResponse_ rep;
	struct ns1__ArrayOfTSURLReturnStatus *repfs;
	struct ns1__srmPutDoneRequest req;
	struct ns1__TSURL *reqfilep;
	struct ns1__TReturnStatus *reqstatp;
	int s, i, n;
	struct soap soap;

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	memset (&req, 0, sizeof(req));

	req.requestToken = (char *) reqtoken;

	/* NOTE: only one SURL in the array */
	if ((req.arrayOfSURLs = 
				soap_malloc (&soap, sizeof(struct ns1__ArrayOfAnyURI))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	if (ret = soap_call_ns1__srmPutDone (&soap, srm_endpoint, "PutDone", &req, &rep)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmPutDoneResponse->returnStatus;
	repfs = rep.srmPutDoneResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray ||
			!repfs->statusArray[0] || !repfs->statusArray[0]->status) {

		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			errno = statuscode2errno(reqstatp->statusCode);
			if (reqstatp->explanation) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, reqstatp->explanation);
				gfal_errmsg (errbuf, errbufsz, errmsg);
			}
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ECOMM;
		}

		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*statuses = (struct srmv2_filestatus *) calloc (n, sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		bzero (*statuses + i, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->surl)
			(*statuses)[i].surl = strdup (repfs->statusArray[i]->surl);
		if (repfs->statusArray[i]->status) {
			(*statuses)[i].status = filestatus2returncode (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation)
				(*statuses)[i].explanation = strdup (repfs->statusArray[i]->status->explanation);
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

srmv2_set_xfer_running (int nbfiles, const char **surls, const char *srm_endpoint, const char *reqtoken, struct srmv2_filestatus **filestatuses,
		char *errbuf, int errbufsz, int timeout)
{
	int i;

	if ((*filestatuses = (struct srmv2_filestatus *) calloc (nbfiles, sizeof (struct srmv2_filestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < nbfiles; ++i) {
		bzero (*filestatuses + i, sizeof (struct srmv2_filestatus));
		(*filestatuses)[i].surl = strdup (surls[i]);
		(*filestatuses)[i].status = 1;
	}

	return (nbfiles);
}

srmv2_turlsfromsurls_get (int nbfiles, const char **surls, const char *srm_endpoint, GFAL_LONG64 *filesizes, const char *spacetokendesc, 
		char **protocols, char **reqtoken, struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	struct ns1__srmAbortRequestRequest abortreq;
	struct ns1__srmAbortRequestResponse_ abortrep;
	time_t endtime;
	int flags;
	int i;
	int n;
	int nbproto = 0;
	int r = 0;
	char *r_token;
	int ret;
	struct ns1__srmPrepareToGetResponse_ rep;
	struct ns1__ArrayOfTGetRequestFileStatus *repfs;
	struct ns1__srmPrepareToGetRequest req;
	struct ns1__TGetFileRequest *reqfilep;
	struct ns1__TReturnStatus *reqstatp;
	struct soap soap;
	struct ns1__srmStatusOfGetRequestResponse_ srep;
	struct ns1__srmStatusOfGetRequestRequest sreq;
	static enum ns1__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	char *targetspacetoken;

retry:

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	while (*protocols[nbproto]) nbproto++;

	/* issue "get" request */

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfFileRequests = 
				soap_malloc (&soap, sizeof(struct ns1__ArrayOfTGetFileRequest))) == NULL || 
			(req.arrayOfFileRequests->requestArray = 
			 soap_malloc (&soap, nbfiles * sizeof(struct ns1__TGetFileRequest *))) == NULL ||
			(req.transferParameters = 
			 soap_malloc (&soap, sizeof(struct ns1__TTransferParameters))) == NULL || 
			(req.targetSpaceToken = 
			 soap_malloc (&soap, sizeof(char *))) == NULL) { 

		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	/* get first space token from user space token description */
	if (!spacetokendesc) {
		req.targetSpaceToken = NULL;
	} else if ((targetspacetoken = srmv2_getspacetoken (spacetokendesc, srm_endpoint, errbuf, errbufsz, timeout)) == NULL) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid space token description", spacetokendesc);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	} else {
		req.targetSpaceToken = strdup(targetspacetoken);
		free (targetspacetoken);
	}

	req.authorizationID = NULL;
	req.userRequestDescription = NULL;
	req.storageSystemInfo = NULL;
	req.desiredFileStorageType = &s_types[PERMANENT];
	req.desiredTotalRequestTime = NULL;
	req.desiredPinLifeTime = NULL;
	req.targetFileRetentionPolicyInfo = NULL;

	for (i = 0; i < nbfiles; i++) {
		if ((req.arrayOfFileRequests->requestArray[i] = 
					soap_malloc (&soap, sizeof(struct ns1__TGetFileRequest))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
	}

	req.arrayOfFileRequests->__sizerequestArray = nbfiles;

	for (i = 0; i < nbfiles; i++) {
		reqfilep = req.arrayOfFileRequests->requestArray[i];
		memset (reqfilep, 0, sizeof(*reqfilep));
		reqfilep->sourceSURL = (char *) surls[i];
		reqfilep->dirOption = NULL;
	}

	req.transferParameters->accessPattern = NULL;
	req.transferParameters->connectionType = NULL;
	req.transferParameters->arrayOfClientNetworks = NULL;

	if ((req.transferParameters->arrayOfTransferProtocols =
				soap_malloc (&soap, nbproto * sizeof(struct ns1__ArrayOfString))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.transferParameters->arrayOfTransferProtocols->__sizestringArray = nbproto;
	req.transferParameters->arrayOfTransferProtocols->stringArray = protocols;

	if (ret = soap_call_ns1__srmPrepareToGet (&soap, srm_endpoint, "PrepareToGet", &req, &rep)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmPrepareToGetResponse->returnStatus;
	repfs = rep.srmPrepareToGetResponse->arrayOfFileStatuses;

	r_token = rep.srmPrepareToGetResponse->requestToken;

	memset (&sreq, 0, sizeof(sreq));
	sreq.requestToken = rep.srmPrepareToGetResponse->requestToken;

	if (timeout > 0)
		endtime = (time(NULL) + timeout);

	/* automatic retry if DB access failed */

	while (reqstatp->statusCode == SRM_USCOREFAILURE) {

		if (timeout > 0 && time(NULL) > endtime) {
			char errmsg[ERRMSG_LEN];
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: User timeout over", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}

		soap_end (&soap);
		soap_done (&soap);
		goto retry;
	}

	/* wait for files ready */

	while (reqstatp->statusCode == SRM_USCOREREQUEST_USCOREQUEUED || 
			reqstatp->statusCode == SRM_USCOREREQUEST_USCOREINPROGRESS) {

		sleep ((r++ == 0) ? 1 : DEFPOLLINT);

		if (ret = soap_call_ns1__srmStatusOfGetRequest (&soap, srm_endpoint, "StatusOfGetRequest", &sreq, &srep)) {
			char errmsg[ERRMSG_LEN];
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg (errbuf, errbufsz, errmsg);
			} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg (errbuf, errbufsz, errmsg);
			}
			soap_end (&soap);
			soap_done (&soap);
			errno = ECOMM;
			return (-1);
		}

		reqstatp = srep.srmStatusOfGetRequestResponse->returnStatus;
		repfs = srep.srmStatusOfGetRequestResponse->arrayOfFileStatuses;

		/* if user timeout has passed, abort the request */
		if (timeout > 0 && time(NULL) > endtime) {
			char errmsg[ERRMSG_LEN];
			abortreq.requestToken = r_token;

			if (ret = soap_call_ns1__srmAbortRequest (&soap, srm_endpoint, "AbortRequest", &abortreq, &abortrep)) {
				if (soap.error == SOAP_EOF) {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
					gfal_errmsg (errbuf, errbufsz, errmsg);
				} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
					gfal_errmsg (errbuf, errbufsz, errmsg);
				}
				soap_end (&soap);
				soap_done (&soap);
				errno = ECOMM;
				return (-1);
			}

			snprintf (errmsg, ERRMSG_LEN - 1, "%s: User timeout over", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
	}

	if (!repfs || repfs->__sizestatusArray < nbfiles || !repfs->statusArray) {

		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			errno = statuscode2errno(reqstatp->statusCode);
			if (reqstatp->explanation) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, reqstatp->explanation);
				gfal_errmsg (errbuf, errbufsz, errmsg);
			}
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = ECOMM;
		}

		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_filestatus *) calloc (n, sizeof (struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		bzero (*filestatuses + i, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->sourceSURL)
			(*filestatuses)[i].surl = strdup ((repfs->statusArray[i])->sourceSURL);
		if (repfs->statusArray[i]->transferURL)
			(*filestatuses)[i].turl = strdup ((repfs->statusArray[i])->transferURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = statuscode2errno ((repfs->statusArray[i])->status->statusCode);
			if (repfs->statusArray[i]->status->explanation)
				(*filestatuses)[i].explanation = strdup ((repfs->statusArray[i])->status->explanation);
		}
	}

	if ((*reqtoken = strdup (sreq.requestToken)) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

srmv2_turlsfromsurls_put (int nbfiles, const char **surls, const char *srm_endpoint, GFAL_LONG64 *filesizes, const char *spacetokendesc, 
		char **protocols, char **reqtoken, struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	struct ns1__srmAbortRequestRequest abortreq;
	struct ns1__srmAbortRequestResponse_ abortrep;
	time_t endtime;
	int flags;
	int i;
	int n;
	int nbproto = 0;
	int nbretry = 0;
	int r = 0;
	char *r_token;
	int ret;
	struct ns1__srmPrepareToPutResponse_ rep;
	struct ns1__ArrayOfTPutRequestFileStatus *repfs;
	struct ns1__srmPrepareToPutRequest req;
	struct ns1__TPutFileRequest *reqfilep;
	struct ns1__TReturnStatus *reqstatp;
	struct soap soap;
	struct ns1__srmStatusOfPutRequestResponse_ srep;
	struct ns1__srmStatusOfPutRequestRequest sreq;
	static enum ns1__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	char *targetspacetoken;
	char errmsg[ERRMSG_LEN];

	/* Create sub-directories of SURLs before doing anything else */
	for (i = 0; i < nbfiles; ++i) {
		if (srmv2_makedirp (surls[i], srm_endpoint, errbuf, errbufsz, timeout) < 0) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Fail to create sub-directories", surls[i]);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			return (-1);
		}
	}

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	while (*protocols[nbproto]) nbproto++;

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfFileRequests =
				soap_malloc (&soap, sizeof(struct ns1__ArrayOfTPutFileRequest))) == NULL ||
			(req.arrayOfFileRequests->requestArray =
			 soap_malloc (&soap, nbfiles * sizeof(struct ns1__TPutFileRequest *))) == NULL ||
			(req.transferParameters =
			 soap_malloc (&soap, sizeof(struct ns1__TTransferParameters))) == NULL ||
			(req.targetSpaceToken =
			 soap_malloc (&soap, sizeof(char *))) == NULL) {

		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	if (!spacetokendesc) {
		req.targetSpaceToken = NULL;
	} else if ((targetspacetoken = srmv2_getspacetoken (spacetokendesc, srm_endpoint, errbuf, errbufsz, timeout)) == NULL) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid space token description", spacetokendesc);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	} else {
		req.targetSpaceToken = strdup(targetspacetoken);
		free (targetspacetoken);
	}

	req.authorizationID = NULL;
	req.userRequestDescription = NULL;
	req.overwriteOption = NULL;
	req.storageSystemInfo = NULL;
	req.desiredFileStorageType = &s_types[PERMANENT];
	req.desiredTotalRequestTime = NULL;
	req.desiredPinLifeTime = NULL;
	req.desiredFileLifeTime = NULL;
	req.targetFileRetentionPolicyInfo = NULL;

	for (i = 0; i < nbfiles; i++) {
		if ((req.arrayOfFileRequests->requestArray[i] =
					soap_malloc (&soap, sizeof(struct ns1__TPutFileRequest))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
	}

	req.arrayOfFileRequests->__sizerequestArray = nbfiles;

	for (i = 0; i < nbfiles; i++) {
		reqfilep = req.arrayOfFileRequests->requestArray[i];
		memset (reqfilep, 0, sizeof(*reqfilep));
		reqfilep->targetSURL = (char *) surls[i];

		if ((reqfilep->expectedFileSize = soap_malloc (&soap, sizeof(ULONG64))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		*reqfilep->expectedFileSize = filesizes[i];
	}

	req.transferParameters->accessPattern = NULL;
	req.transferParameters->connectionType = NULL;
	req.transferParameters->arrayOfClientNetworks = NULL;

	if ((req.transferParameters->arrayOfTransferProtocols =
				soap_malloc (&soap, nbproto * sizeof(struct ns1__ArrayOfString))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.transferParameters->arrayOfTransferProtocols->__sizestringArray = nbproto;
	req.transferParameters->arrayOfTransferProtocols->stringArray = protocols;

retry:

	if (ret = soap_call_ns1__srmPrepareToPut (&soap, srm_endpoint, "PrepareToPut", &req, &rep)) {
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmPrepareToPutResponse->returnStatus;
	repfs = rep.srmPrepareToPutResponse->arrayOfFileStatuses;

	r_token = rep.srmPrepareToPutResponse->requestToken;

	memset (&sreq, 0, sizeof(sreq));
	sreq.requestToken = rep.srmPrepareToPutResponse->requestToken;

	if (timeout > 0)
		endtime = time(NULL) + timeout;

	/* automatic retry if DB access failed */

	while (reqstatp->statusCode == SRM_USCOREFAILURE) {

		if (timeout > 0 && time(NULL) > endtime) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: User timeout over", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}

		soap_end (&soap);
		soap_done (&soap);
		goto retry;
	} 

	/* wait for files ready */

	while (reqstatp->statusCode == SRM_USCOREREQUEST_USCOREQUEUED ||
			reqstatp->statusCode == SRM_USCOREREQUEST_USCOREINPROGRESS) {

		sleep ((r++ == 0) ? 1 : DEFPOLLINT);

		if (ret = soap_call_ns1__srmStatusOfPutRequest (&soap, srm_endpoint, "StatusOfPutRequest", &sreq, &srep)) {
			if (soap.error == SOAP_EOF) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
				gfal_errmsg (errbuf, errbufsz, errmsg);
			} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
				gfal_errmsg (errbuf, errbufsz, errmsg);
			}
			soap_end (&soap);
			soap_done (&soap);
			errno = ECOMM;
			return (-1);
		}

		reqstatp = srep.srmStatusOfPutRequestResponse->returnStatus;
		repfs = srep.srmStatusOfPutRequestResponse->arrayOfFileStatuses;

		/* if user timeout has passed, abort the request */
		if (timeout > 0 && time(NULL) > endtime) {
			abortreq.requestToken = r_token;

			if (ret = soap_call_ns1__srmAbortRequest (&soap, srm_endpoint, "AbortRequest", &abortreq, &abortrep)) {
				if (soap.error == SOAP_EOF) {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
					gfal_errmsg (errbuf, errbufsz, errmsg);
				} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
					gfal_errmsg (errbuf, errbufsz, errmsg);
				}
				soap_end (&soap);
				soap_done (&soap);
				errno = ECOMM;
				return (-1);
			}

			snprintf (errmsg, ERRMSG_LEN - 1, "%s: User timeout over", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = ETIMEDOUT;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
	}

	if (reqstatp->statusCode == SRM_USCORESPACE_USCORELIFETIME_USCOREEXPIRED) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Space lifetime expired", srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = statuscode2errno(reqstatp->statusCode);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	if (!repfs || repfs->__sizestatusArray < nbfiles || !repfs->statusArray) {
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			errno = statuscode2errno(reqstatp->statusCode);
			if (reqstatp->explanation) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, reqstatp->explanation);
				gfal_errmsg (errbuf, errbufsz, errmsg);
			}
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = ECOMM;
		}

		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_filestatus *) calloc (n, sizeof (struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		bzero (*filestatuses + i, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->SURL)
			(*filestatuses)[i].surl = strdup (repfs->statusArray[i]->SURL);
		if (repfs->statusArray[i]->transferURL)
			(*filestatuses)[i].turl = strdup (repfs->statusArray[i]->transferURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = statuscode2errno (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation)
				(*filestatuses)[i].explanation = strdup (repfs->statusArray[i]->status->explanation);
		}
	}

	if ((*reqtoken = strdup (sreq.requestToken)) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	soap_end (&soap);
	soap_done (&soap);

	return (0);
}

#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
srmv2_getfilemd (int nbfiles, const char **surls, const char *srm_endpoint, struct srmv2_mdfilestatus **statuses,
		char **reqtoken, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int ret;
	int sav_errno;
	struct soap soap;
	enum xsd__boolean trueoption = true_;
	int numlevels = 0;
	struct ns1__srmLsRequest req;
	struct ns1__srmLsResponse_ rep;
	struct ns1__ArrayOfTMetaDataPathDetail *repfs;
	struct ns1__TSURLInfo *reqfilep;
	struct ns1__TReturnStatus *reqstatp;
	int i, n;


	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;

	memset (&req, 0, sizeof(req));

	/* NOTE: only one file in the array */
	if ((req.arrayOfSURLs = soap_malloc (&soap, sizeof(struct ns1__ArrayOfAnyURI))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.fullDetailedList = &trueoption;
	req.numOfLevels = &numlevels;
	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	/* issue "srmLs" request */

	if ((ret = soap_call_ns1__srmLs (&soap, srm_endpoint, "Ls", &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		}
		if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (0);
	}

	reqstatp = rep.srmLsResponse->returnStatus;
	repfs = rep.srmLsResponse->details;

	if (reqtoken && rep.srmLsResponse->requestToken)
		*reqtoken = strdup(rep.srmLsResponse->requestToken);

	if (!repfs || repfs->__sizepathDetailArray <= 0 || !repfs->pathDetailArray) {
		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREDONE) {
			errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, reqstatp->explanation);
				gfal_errmsg(errbuf, errbufsz, errmsg);
			}
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ECOMM;
		}
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizepathDetailArray;

	if ((*statuses = (struct srmv2_mdfilestatus *) calloc (n, sizeof (struct srmv2_mdfilestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; ++i) {
		bzero (*statuses + i, sizeof(struct srmv2_mdfilestatus));
		if (repfs->pathDetailArray[i]->path)
			(*statuses)[i].surl = strdup (repfs->pathDetailArray[i]->path);
		if (repfs->pathDetailArray[i]->status)
			(*statuses)[i].status = statuscode2errno(repfs->pathDetailArray[i]->status->statusCode);
		if ((*statuses)[i].status) {
			if (repfs->pathDetailArray[i]->status->explanation)
				(*statuses)[i].explanation = strdup (repfs->pathDetailArray[i]->status->explanation);
			continue;
		}

		(*statuses)[i].stat.st_size = *(repfs->pathDetailArray[i]->size);
		(*statuses)[i].stat.st_uid = 2;
		(*statuses)[i].stat.st_gid = 2;
		(*statuses)[i].stat.st_nlink = 1;
		(*statuses)[i].stat.st_mode = *(repfs->pathDetailArray[i]->otherPermission);
		(*statuses)[i].stat.st_mode |= repfs->pathDetailArray[i]->groupPermission->mode << 3;
		(*statuses)[i].stat.st_mode |= repfs->pathDetailArray[i]->ownerPermission->mode << 6;
		switch (*(repfs->pathDetailArray[i]->type)) {
			case FILE_:
				(*statuses)[i].stat.st_mode |= S_IFREG;
				break;
			case DIRECTORY:
				(*statuses)[i].stat.st_mode |= S_IFDIR;
				break;
			case LINK:
				(*statuses)[i].stat.st_mode |= S_IFLNK;
				break;
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);
}
#endif

srmv2_pin (int nbfiles, const char **surls, const char *srm_endpoint, const char *reqtoken, int pintime,
		struct srmv2_pinfilestatus **pinfilestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int n;
	int rc;
	int ret;
	struct ns1__ArrayOfTSURLLifetimeReturnStatus *repfs;
	struct ns1__srmExtendFileLifeTimeResponse_ rep;
	struct ns1__srmExtendFileLifeTimeRequest req;
	struct ns1__TReturnStatus *reqstatp;
	struct soap soap;
	int i = 0;
	char *targetspacetoken;

retry:

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	/* issue "extendfilelifetime" request */

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfSURLs = soap_malloc (&soap, sizeof(struct ns1__ArrayOfAnyURI))) == NULL) {

		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.authorizationID = NULL;
	req.requestToken = (char *) reqtoken;
	req.newFileLifeTime = NULL;
	req.newPinLifeTime = &pintime;
	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	if (ret = soap_call_ns1__srmExtendFileLifeTime (&soap, srm_endpoint, "ExtendFileLifeTime", &req, &rep)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	/* return file statuses */
	reqstatp = rep.srmExtendFileLifeTimeResponse->returnStatus;
	repfs = rep.srmExtendFileLifeTimeResponse->arrayOfFileStatuses;

	if (! repfs) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		pinfilestatuses = NULL;
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*pinfilestatuses = (struct srmv2_pinfilestatus *) calloc (n, sizeof(struct srmv2_pinfilestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		bzero (*pinfilestatuses + i, sizeof (struct srmv2_pinfilestatus));
		if (repfs->statusArray[i]->surl)
			(*pinfilestatuses)[i].surl = strdup (repfs->statusArray[i]->surl);
		(*pinfilestatuses)[i].pinlifetime = *(repfs->statusArray[i]->pinLifetime);
		if (repfs->statusArray[i]->status) {
			(*pinfilestatuses)[i].status = filestatus2returncode (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation)
				(*pinfilestatuses)[i].explanation = strdup (repfs->statusArray[i]->status->explanation);
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);	
}

srmv2_release (int nbfiles, const char **surls, const char *srm_endpoint, const char *reqtoken,
		struct srmv2_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int ret;
	struct ns1__srmReleaseFilesResponse_ rep;
	struct ns1__ArrayOfTSURLReturnStatus *repfs;
	struct ns1__srmReleaseFilesRequest req;
	struct ns1__TSURL *reqfilep;
	struct ns1__TReturnStatus *reqstatp;
	int s, i, n;
	struct soap soap;

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	memset (&req, 0, sizeof(req));

	req.requestToken = (char *) reqtoken;

	/* NOTE: only one SURL in the array */
	if ((req.arrayOfSURLs =
				soap_malloc (&soap, sizeof(struct ns1__ArrayOfAnyURI))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	if (ret = soap_call_ns1__srmReleaseFiles (&soap, srm_endpoint, "ReleaseFiles", &req, &rep)) {
		char errmsg[ERRMSG_LEN];
		if (soap.error == SOAP_EOF) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Connection fails or timeout", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		} else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmReleaseFilesResponse->returnStatus;
	repfs = rep.srmReleaseFilesResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray ||
			!repfs->statusArray[0] || !repfs->statusArray[0]->status) {

		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			errno = statuscode2errno(reqstatp->statusCode);
			if (reqstatp->explanation) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, reqstatp->explanation);
				gfal_errmsg(errbuf, errbufsz, errmsg);
			}
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: <empty response>", srm_endpoint);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = ECOMM;
		}

		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*statuses = (struct srmv2_filestatus *) calloc (n, sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		bzero (*statuses + i, sizeof (struct srmv2_pinfilestatus));
		if (repfs->statusArray[i]->surl)
			(*statuses)[i].surl = strdup (repfs->statusArray[i]->surl);
		if (repfs->statusArray[i]->status) {
			(*statuses)[i].status = filestatus2returncode (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation)
				(*statuses)[i].explanation = strdup (repfs->statusArray[i]->status->explanation);
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);

}

statuscode2errno (int statuscode)
{
	switch (statuscode) {
		case SRM_USCOREINVALID_USCOREPATH:
			return (ENOENT);
		case SRM_USCOREINVALID_USCOREREQUEST:
		case SRM_USCORESPACE_USCORELIFETIME_USCOREEXPIRED:
			return (EINVAL);
		case SRM_USCOREAUTHORIZATION_USCOREFAILURE:
			return (EACCES);
		case SRM_USCOREDUPLICATION_USCOREERROR:
			return (EEXIST);
		case SRM_USCORENO_USCOREFREE_USCORESPACE:
			return (ENOSPC);
		case SRM_USCOREINTERNAL_USCOREERROR:
			return (ECOMM);
		case SRM_USCORESUCCESS:
		case SRM_USCOREFILE_USCOREPINNED:
		case SRM_USCORESPACE_USCOREAVAILABLE:
			return (0);
		default:
			return (EINVAL);
	}
}

filestatus2returncode (int filestatus)
{
	switch (filestatus) {
		case SRM_USCOREREQUEST_USCOREQUEUED:
		case SRM_USCOREREQUEST_USCOREINPROGRESS:
			return (0);
		case SRM_USCOREABORTED:
		case SRM_USCORERELEASED:
		case SRM_USCOREFILE_USCORELOST:
		case SRM_USCOREFILE_USCOREBUSY:
		case SRM_USCOREFILE_USCOREUNAVAILABLE:
		case SRM_USCOREINVALID_USCOREPATH:
		case SRM_USCOREAUTHORIZATION_USCOREFAILURE:
		case SRM_USCOREFILE_USCORELIFETIME_USCOREEXPIRED:
		case SRM_USCOREFAILURE:
			return (-1);
		case SRM_USCORESUCCESS:
		case SRM_USCOREFILE_USCOREPINNED:
			return (1);
	}
}
