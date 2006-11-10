/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: srm2_2_ifce.c,v $ $Revision: 1.4 $ $Date: 2006/11/10 16:14:53 $ CERN Jean-Philippe Baud
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

static int
srmv2_init (struct soap *soap, const char *surl, char *srm_endpoint,
          int srm_endpointsz, char *errbuf, int errbufsz)
{
	int flags;
	char *sfn;

	if (parsesurl (surl, srm_endpoint, srm_endpointsz, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	soap_init (soap);
	soap->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif
	return (0);
}

srmv2_deletesurl (const char *surl, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	char *e;
	int flags;
	int ret;
        struct ns1__srmRmResponse_ rep;
	struct ns1__ArrayOfTSURLReturnStatus *repfs;
	struct ns1__srmRmRequest req;
	struct ns1__TReturnStatus *reqstatp;
	int s;
        struct soap soap;

	soap_init (&soap);
	(&soap)->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

        soap.send_timeout = timeout ;
        soap.recv_timeout = timeout ;

	memset (&req, 0, sizeof(req));

	/* NOTE: only one file in the array */
	if ((req.arrayOfSURLs = soap_malloc (&soap, sizeof(struct ns1__ArrayOfAnyURI))) == NULL || 
	    (req.arrayOfSURLs->urlArray = soap_malloc (&soap, sizeof(char *))) == NULL) {

		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = 1;
	req.arrayOfSURLs->urlArray[0] = (char *) surl;

        /* issue "srmRm" request */

        if (ret = soap_call_ns1__srmRm (&soap, srm_endpoint, "srmRm", &req, &rep)) {
                if (soap.error == SOAP_EOF) {
                        gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
                        soap_end (&soap);
                        soap_done (&soap);
                        return (-1);
                }
                if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
                        gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
                soap_end (&soap);
                soap_done (&soap);
                return (-1);
        }

	reqstatp = rep.srmRmResponse->returnStatus;
	repfs = rep.srmRmResponse->arrayOfFileStatuses;

	if (reqstatp->statusCode != SRM_USCORESUCCESS &&
		reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

		errno = statuscode2errno(reqstatp->statusCode);
		if (reqstatp->explanation) {
			gfal_errmsg(errbuf, errbufsz, reqstatp->explanation);
		} else if (reqstatp->statusCode == SRM_USCOREINVALID_USCOREPATH) {
                        gfal_errmsg(errbuf, errbufsz, "Invalid file or directory");
			errno = EINVAL;
                }

		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	if (! repfs) {
		gfal_errmsg(errbuf, errbufsz, "Empty ArrayOfTSURLReturnStatus");
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

        soap_end (&soap);
        soap_done (&soap);
        return (0);
}

srmv2_get (int nbfiles, char **surls, char *spacetokendesc, int nbprotocols, char **protocols, char **reqtoken, 
	struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	struct srmv2_filestatus *fs;
	int i = 0;
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
	char **se_types;
	char **se_endpoints;
	char *srm_endpoint;
	int srm_v2;
	char *targetspacetoken;
	
retry:

	soap_init (&soap);
	(&soap)->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

        while (se_types[i]) {
                if ((strcmp (se_types[i], "srm_v2")) == 0) {
                        srm_v2 = 1;
			srm_endpoint = se_endpoints[i];
		}
                i++;
        }

	free (se_types);
	free (se_endpoints);

	if (! srm_v2) {
		gfal_errmsg (errbuf, errbufsz, "Storage Element doesn't publish SRM v2.2");
		return (-1);
	}

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
		gfal_errmsg(errbuf, errbufsz, "Invalid space token description");
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
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	/* return request token */
        *reqtoken = strdup(rep.srmPrepareToGetResponse->requestToken);

	/* return file statuses */
	reqstatp = rep.srmPrepareToGetResponse->returnStatus;
	repfs = rep.srmPrepareToGetResponse->arrayOfFileStatuses;

	if (! repfs) {
		filestatuses = NULL;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = malloc (n * sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	fs = *filestatuses;

	for (i = 0; i < n; i++) {
		if (repfs->statusArray[i]->transferURL) {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);
			fs->turl = strdup ((repfs->statusArray[i])->transferURL);
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = NULL;
		} else if (repfs->statusArray[i]->status->explanation) {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);	
			fs->turl = NULL;
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = strdup ((repfs->statusArray[i])->status->explanation);
		} else {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);
			fs->turl = NULL;
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = NULL;
		}
		fs++;
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);	
}

srmv2_getstatus (int nbfiles, char **surls, char *reqtoken, struct srmv2_filestatus **filestatuses, 
	char *errbuf, int errbufsz, int timeout)
{
	int flags;
	struct srmv2_filestatus *fs;
	int i = 0;
	int n;
	int rc;
	int ret;
	struct ns1__ArrayOfTGetRequestFileStatus *repfs;
	struct ns1__TReturnStatus *reqstatp;
	char **se_types;
	char **se_endpoints;
	struct soap soap;
	struct ns1__srmStatusOfGetRequestResponse_ srep;
	struct ns1__srmStatusOfGetRequestRequest sreq;
	char *srm_endpoint;
	int srm_v2;
	char *targetspacetoken;

retry:

	soap_init (&soap);
	(&soap)->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

        while (se_types[i]) {
                if ((strcmp (se_types[i], "srm_v2")) == 0) {
                        srm_v2 = 1;
			srm_endpoint = se_endpoints[i];
		}
                i++;
        }

	free (se_types);
	free (se_endpoints);

	if (! srm_v2) {
		gfal_errmsg (errbuf, errbufsz, "Storage Element doesn't publish SRM v2.2");
		return (-1);
	}

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	memset (&sreq, 0, sizeof(sreq));
	sreq.requestToken = reqtoken;

	if (ret = soap_call_ns1__srmStatusOfGetRequest (&soap, srm_endpoint, "StatusOfGetRequest", &sreq, &srep)) {
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	/* return file statuses */
	reqstatp = srep.srmStatusOfGetRequestResponse->returnStatus;
	repfs = srep.srmStatusOfGetRequestResponse->arrayOfFileStatuses;

	if (! repfs) {
		filestatuses = NULL;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = malloc (n * sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	fs = *filestatuses;

        for (i = 0; i < n; i++) {
		if (repfs->statusArray[i]->transferURL) {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);
			fs->turl = strdup ((repfs->statusArray[i])->transferURL);
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = NULL;
		} else if (repfs->statusArray[i]->status->explanation) {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);
			fs->turl = NULL;
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = strdup ((repfs->statusArray[i])->status->explanation);
		} else {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);
			fs->turl = NULL;
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = NULL;
		 }
		fs++;
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);
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
	(&soap)->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	memset (&tknreq, 0, sizeof(tknreq));

	tknreq.userSpaceTokenDescription = (char *) spacetokendesc;

	if (ret = soap_call_ns1__srmGetSpaceTokens (&soap, srm_endpoint, "GetSpaceToken", &tknreq, &tknrep)) {
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
			soap_end (&soap);
			soap_done (&soap);
			return (NULL);
		}
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		return (NULL);
	}

	tknrepstatp = tknrep.srmGetSpaceTokensResponse->returnStatus;

	if (tknrepstatp->statusCode != SRM_USCORESUCCESS) {
		errno = statuscode2errno(tknrepstatp->statusCode);
		if (tknrepstatp->explanation) {
			gfal_errmsg(errbuf, errbufsz, tknrepstatp->explanation);
		}
		soap_end (&soap);
		soap_done (&soap);
		return (NULL);
	}

	tknrepp = tknrep.srmGetSpaceTokensResponse->arrayOfSpaceTokens;

	if (! tknrepp) {
		gfal_errmsg (errbuf, errbufsz, "Empty arrayOfSpaceTokens");
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
srmv2_makedirpt (const char *dest_file, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	int c = 0;
	char *file;
	int flags;
	int nbslash = 0;
	char *p;
	struct ns1__srmMkdirResponse_ rep;
	struct ns1__srmMkdirRequest req;
	struct ns1__TReturnStatus *repstatp;
	int ret;
	struct soap soap;

	file = strdup (dest_file);
	p = file;

	while (p = strchr (p, '/')) {
		*p = '\0';
		if (nbslash < 2) {	/* skip first 3 slashes */
			nbslash++;
			*p++ = '/';
			while (*p == '/')
				p++;
			continue;
		}
 
		/* try to create directory */
		soap_init (&soap);
		(&soap)->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
		flags = CGSI_OPT_DISABLE_NAME_CHECK;
		soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

		soap.send_timeout = timeout;
		soap.recv_timeout = timeout;

		memset (&req, 0, sizeof(req));

		req.directoryPath = (char *) file;

		if (ret = soap_call_ns1__srmMkdir (&soap, srm_endpoint, "Mkdir", &req, &rep)) {
        		if (soap.error == SOAP_EOF) {
                		gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
				free (file);
				soap_end (&soap);
				soap_done (&soap);
				return (0);
			}
			if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			free (file);
			soap_end (&soap);
			soap_done (&soap);
			return (0);
		}

		repstatp = rep.srmMkdirResponse->returnStatus;

		if (repstatp->statusCode != SRM_USCORESUCCESS) {
			errno = statuscode2errno(repstatp->statusCode);
			if (errno == EEXIST) {
				goto ret;
			} 
		}

ret:
		*p++ = '/';	/* restore slash */
		while (*p == '/')
			p++;

		soap_end (&soap);
		soap_done (&soap);
	}

	free (file);
	return (0);
}

srmv2_prestage (int nbfiles, char **surls, char *spacetokendesc, int nbprotocols, char **protocols, char **reqtoken, 
	struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	struct srmv2_filestatus *fs;
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
	char **se_types;
	char **se_endpoints;
	char *srm_endpoint;
	int srm_v2;
	static enum ns1__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	char *targetspacetoken;

retry:

	soap_init (&soap);
	(&soap)->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

        while (se_types[i]) {
                if ((strcmp (se_types[i], "srm_v2")) == 0) {
                        srm_v2 = 1;
			srm_endpoint = se_endpoints[i];
		}
                i++;
        }

	free (se_types);
	free (se_endpoints);

	if (!srm_v2) {
		gfal_errmsg (errbuf, errbufsz, "Storage Element doesn't publish SRM v2.2");
		return (-1);
	}

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
		gfal_errmsg(errbuf, errbufsz, "Invalid space token description");
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
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	/* return request token */
        *reqtoken = strdup(rep.srmBringOnlineResponse->requestToken);

	/* return file statuses */
	reqstatp = rep.srmBringOnlineResponse->returnStatus;
	repfs = rep.srmBringOnlineResponse->arrayOfFileStatuses;

	if (! repfs) {
		filestatuses = NULL;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = malloc (n * sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	fs = *filestatuses;

	for (i = 0; i < n; i++) {
		if (repfs->statusArray[i]->status->explanation) {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);
			fs->turl = NULL;
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = strdup ((repfs->statusArray[i])->status->explanation);
		} else {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);
			fs->turl = NULL;
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = NULL;
		}
		fs++;
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);	
}

srmv2_prestagestatus (int nbfiles, char **surls, char *reqtoken, struct srmv2_filestatus **filestatuses, 
	char *errbuf, int errbufsz, int timeout)
{
	int flags;
	struct srmv2_filestatus *fs;
	int i = 0;
	int n;
	int r = 0;
	int rc;
	int ret;
	struct ns1__ArrayOfTBringOnlineRequestFileStatus *repfs;
	struct ns1__TReturnStatus *reqstatp;
	char **se_types;
	char **se_endpoints;
	struct soap soap;
	struct ns1__srmStatusOfBringOnlineRequestResponse_ srep;
	struct ns1__srmStatusOfBringOnlineRequestRequest sreq;
	char *srm_endpoint;
	int srm_v2;
	char *targetspacetoken;

retry:

	soap_init (&soap);
	(&soap)->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

		if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

        while (se_types[i]) {
                if ((strcmp (se_types[i], "srm_v2")) == 0) {
                        srm_v2 = 1;
			srm_endpoint = se_endpoints[i];
		}
                i++;
        }

	free (se_types);
	free (se_endpoints);

	if (! srm_v2) {
		gfal_errmsg (errbuf, errbufsz, "Storage Element doesn't publish SRM v2.2");
		return (-1);
	}

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	memset (&sreq, 0, sizeof(sreq));
	sreq.requestToken = reqtoken;

	if (ret = soap_call_ns1__srmStatusOfBringOnlineRequest (&soap, srm_endpoint, "StatusOfBringOnlineRequest", &sreq, &srep)) {
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	reqstatp = srep.srmStatusOfBringOnlineRequestResponse->returnStatus;
	repfs = srep.srmStatusOfBringOnlineRequestResponse->arrayOfFileStatuses;

	/* return file statuses */
	reqstatp = srep.srmStatusOfBringOnlineRequestResponse->returnStatus;
	repfs = srep.srmStatusOfBringOnlineRequestResponse->arrayOfFileStatuses;

	if (! repfs) {
		filestatuses = NULL;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = malloc (n * sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	fs = *filestatuses;

	for (i = 0; i < n; i++) {
		if (repfs->statusArray[i]->status->explanation) {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);
			fs->turl = NULL;
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = strdup ((repfs->statusArray[i])->status->explanation);
		} else {
			fs->surl = strdup ((repfs->statusArray[i])->sourceSURL);
			fs->turl = NULL;
			fs->status = filestatus2returncode ((repfs->statusArray[i])->status->statusCode);
			fs->explanation = NULL;
		}
		fs++;
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);	
}

srmv2_set_xfer_done_get (const char *surl, char *reqtoken, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	char *e;
	int flags;
	int ret;
	struct ns1__srmReleaseFilesResponse_ rep;
	struct ns1__ArrayOfTSURLReturnStatus *repfs;
	struct ns1__srmReleaseFilesRequest req;
	struct ns1__TSURL *reqfilep;
	struct ns1__TReturnStatus *reqstatp;
	int s;
	struct soap soap;

	soap_init (&soap);
	(&soap)->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

        soap.send_timeout = timeout;
        soap.recv_timeout = timeout;

        memset (&req, 0, sizeof(req));

        req.requestToken = reqtoken;

        /* NOTE: only one SURL in the array */
        if ((req.arrayOfSURLs =
                        soap_malloc (&soap, sizeof(struct ns1__ArrayOfAnyURI))) == NULL ||
            (req.arrayOfSURLs->urlArray =
                        soap_malloc (&soap, 1 * sizeof(char *))) == NULL) {
                gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
                errno = ENOMEM;
                soap_end (&soap);
                soap_done (&soap);
                return (-1);
        }

        req.arrayOfSURLs->__sizeurlArray = 1;
        req.arrayOfSURLs->urlArray[0] = (char *) surl;

        if (ret = soap_call_ns1__srmReleaseFiles (&soap, srm_endpoint, "ReleaseFiles", &req, &rep)) {
                if (soap.error == SOAP_EOF) {
                        gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
                        soap_end (&soap);
                        soap_done (&soap);
                        return (-1);
                }
                if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
                        gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
                soap_end (&soap);
                soap_done (&soap);
                return (-1);
        }

        reqstatp = rep.srmReleaseFilesResponse->returnStatus;
        repfs = rep.srmReleaseFilesResponse->arrayOfFileStatuses;

        if (reqstatp->statusCode != SRM_USCORESUCCESS &&
            reqstatp->statusCode != SRM_USCOREDONE) {
                errno = statuscode2errno(reqstatp->statusCode);
                if (reqstatp->explanation)
                        gfal_errmsg(errbuf, errbufsz, reqstatp->explanation);
        }

        if (! repfs) {
                gfal_errmsg(errbuf, errbufsz, "Empty arrayOfFileStatuses");
                soap_end (&soap);
                soap_done (&soap);
                return (-1);
        }

        soap_end (&soap);
        soap_done (&soap);
        return (0);

}

srmv2_set_xfer_done_put (const char *surl, char *reqtoken, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	char *e;
	int flags;
	int ret;
	struct ns1__srmPutDoneResponse_ rep;
	struct ns1__ArrayOfTSURLReturnStatus *repfs;
	struct ns1__srmPutDoneRequest req;
	struct ns1__TSURL *reqfilep;
	struct ns1__TReturnStatus *reqstatp;
	int s;
	struct soap soap;

	soap_init (&soap);
	(&soap)->namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;

	memset (&req, 0, sizeof(req));

	req.requestToken = reqtoken;

	/* NOTE: only one SURL in the array */
	if ((req.arrayOfSURLs = 
			soap_malloc (&soap, sizeof(struct ns1__ArrayOfAnyURI))) == NULL || 
	    (req.arrayOfSURLs->urlArray = 
			soap_malloc (&soap, 1 * sizeof(char *))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = 1;
	req.arrayOfSURLs->urlArray[0] = (char *) surl;

	if (ret = soap_call_ns1__srmPutDone (&soap, srm_endpoint, "PutDone", &req, &rep)) {
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	reqstatp = rep.srmPutDoneResponse->returnStatus;
	repfs = rep.srmPutDoneResponse->arrayOfFileStatuses;

	if (reqstatp->statusCode != SRM_USCORESUCCESS &&
	    reqstatp->statusCode != SRM_USCOREDONE) {
		errno = statuscode2errno(reqstatp->statusCode);
		if (reqstatp->explanation) 
			gfal_errmsg(errbuf, errbufsz, reqstatp->explanation);
	}

	if (! repfs) {
		gfal_errmsg(errbuf, errbufsz, "Empty arrayOfFileStatuses");
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

srmv2_set_xfer_running (const char *surl, char *reqtoken, char *errbuf, int errbufsz, int timeout)
{
	return (0);
}

srmv2_turlsfromsurls_get (int nbfiles, const char **surls, const char *srm_endpoint, GFAL_LONG64 *filesizes, const char *spacetokendesc, 
	char **protocols, char **reqtoken, char ***sourcesurls, char ***turls, int **statuses, char ***explanations, char *errbuf, 
	int errbufsz, int timeout)
{
	struct ns1__srmAbortRequestRequest abortreq;
	struct ns1__srmAbortRequestResponse_ abortrep;
	char **e;
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
	int *s;
	struct soap soap;
	struct ns1__srmStatusOfGetRequestResponse_ srep;
	struct ns1__srmStatusOfGetRequestRequest sreq;
	static enum ns1__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	char *targetspacetoken;
	char **t;
	char **u;

retry:

	soap_init (&soap);
	(&soap)->namespaces = namespaces_srmv2;

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
		gfal_errmsg(errbuf, errbufsz, "Invalid space token description");
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
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
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
			gfal_errmsg(errbuf, errbufsz, "User timeout over");
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
			if (soap.error == SOAP_EOF) {
				gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
				soap_end (&soap);
				soap_done (&soap);
				return (-1);
			}
			if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}

		reqstatp = srep.srmStatusOfGetRequestResponse->returnStatus;
		repfs = srep.srmStatusOfGetRequestResponse->arrayOfFileStatuses;

		/* if user timeout has passed, abort the request */
		if (timeout > 0 && time(NULL) > endtime) {

			abortreq.requestToken = r_token;

			if (ret = soap_call_ns1__srmAbortRequest (&soap, srm_endpoint, "AbortRequest", &abortreq, &abortrep)) {
				if (soap.error == SOAP_EOF) {
					gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
					soap_end (&soap);
					soap_done (&soap);
					return (-1);
				}
				if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
					gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
				soap_end (&soap);
				soap_done (&soap);
				return (-1);
			}

			gfal_errmsg(errbuf, errbufsz, "User timeout over");
			errno = ETIMEDOUT;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
	}

	if (reqstatp->statusCode != SRM_USCORESUCCESS &&
		reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

		errno = statuscode2errno(reqstatp->statusCode); 
		if (reqstatp->explanation) {
			gfal_errmsg(errbuf, errbufsz, reqstatp->explanation);
		} else if (reqstatp->statusCode == SRM_USCOREINVALID_USCOREPATH) {
			gfal_errmsg(errbuf, errbufsz, "Invalid file or directory");
			errno = EINVAL;
		}
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	if (! repfs) {
		gfal_errmsg(errbuf, errbufsz, "Empty arrayOfFileStatuses");
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((u = malloc (n * sizeof (char *))) == NULL ||
	    (t = malloc (n * sizeof (char *))) == NULL ||
	    (e = malloc (n * sizeof (char *))) == NULL || 
	    (s = malloc (n * sizeof (int))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		if (repfs->statusArray[i]->transferURL) {
			u[i] = strdup ((repfs->statusArray[i])->sourceSURL);
			t[i] = strdup ((repfs->statusArray[i])->transferURL);
			e[i] = NULL;
			s[i] = ((repfs->statusArray[i])->status->statusCode);
		} else if (repfs->statusArray[i]->status->explanation) {
			u[i] = strdup ((repfs->statusArray[i])->sourceSURL);
			t[i] = NULL;
			e[i] = strdup ((repfs->statusArray[i])->status->explanation);
			s[i] = ((repfs->statusArray[i])->status->statusCode);
		} else {
			u[i] = strdup ((repfs->statusArray[i])->sourceSURL);
			t[i] = NULL;
			e[i] = NULL;
			s[i] = ((repfs->statusArray[i])->status->statusCode);
		}
	}

	if ((*reqtoken = strdup (sreq.requestToken)) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	*sourcesurls = u;
	*turls = t;
	*explanations = e;
	*statuses = s;
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srmv2_turlsfromsurls_put (int nbfiles, const char **surls, const char *srm_endpoint, GFAL_LONG64 *filesizes, const char *spacetokendesc, 
	char **protocols, char **reqtoken, char ***sourcesurls, char ***turls, int **statuses, char ***explanations, char *errbuf, 
	int errbufsz, int timeout)
{
	struct ns1__srmAbortRequestRequest abortreq;
	struct ns1__srmAbortRequestResponse_ abortrep;
	time_t endtime;
	char **e;
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
	int *s;
	struct soap soap;
	struct ns1__srmStatusOfPutRequestResponse_ srep;
	struct ns1__srmStatusOfPutRequestRequest sreq;
	static enum ns1__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	char *targetspacetoken;
	char **t;
	char **u;

retry:

	soap_init (&soap);
	(&soap)->namespaces = namespaces_srmv2;

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
		gfal_errmsg(errbuf, errbufsz, "Invalid space token description");
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
			perror ("malloc");
			soap_end (&soap);
			exit (1);
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

	if (ret = soap_call_ns1__srmPrepareToPut (&soap, srm_endpoint, "PrepareToPut", &req, &rep)) {
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
		soap_end (&soap);
		soap_done (&soap);
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
			gfal_errmsg(errbuf, errbufsz, "User timeout over");
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
				gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
				soap_end (&soap);
				soap_done (&soap);
				return (-1);
			}
			if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}

		reqstatp = srep.srmStatusOfPutRequestResponse->returnStatus;
		repfs = srep.srmStatusOfPutRequestResponse->arrayOfFileStatuses;

		/* if user timeout has passed, abort the request */
		if (timeout > 0 && time(NULL) > endtime) {

			abortreq.requestToken = r_token;

			if (ret = soap_call_ns1__srmAbortRequest (&soap, srm_endpoint, "AbortRequest", &abortreq, &abortrep)) {
				if (soap.error == SOAP_EOF) {
					gfal_errmsg(errbuf, errbufsz, "Connection fails or timeout");
					soap_end (&soap);
					soap_done (&soap);
					return (-1);
				}
				if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
					gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
				soap_end (&soap);
				soap_done (&soap);
				return (-1);
			}

			gfal_errmsg(errbuf, errbufsz, "User timeout over");
			errno = ETIMEDOUT;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
	}

	if (reqstatp->statusCode != SRM_USCORESUCCESS &&
		reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

		errno = statuscode2errno(reqstatp->statusCode);
		if (reqstatp->explanation) {
			gfal_errmsg(errbuf, errbufsz, reqstatp->explanation);
		} else if (reqstatp->statusCode == SRM_USCOREINVALID_USCOREPATH) {
			/* try to create the possible missing directories */
			soap_end (&soap);
			soap_done (&soap);
			for (i = 0; i < nbfiles; i++) {
				srmv2_makedirpt (surls[i], srm_endpoint, errbuf, errbufsz, timeout);
			}
			if (nbretry == 0) {
				nbretry++;
				goto retry;
			}
			gfal_errmsg(errbuf, errbufsz, "Invalid file or directory");
		} else if (reqstatp->statusCode == SRM_USCORESPACE_USCORELIFETIME_USCOREEXPIRED) {
			gfal_errmsg(errbuf, errbufsz, "Space lifetime expired");
		}
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	if (! repfs) {
		gfal_errmsg(errbuf, errbufsz, "Empty arrayOfFileStatuses");
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((u = malloc (n * sizeof (char *))) == NULL ||
	    (t = malloc (n * sizeof (char *))) == NULL ||
	    (e = malloc (n * sizeof (char *))) == NULL ||
	    (s = malloc (n * sizeof (int))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		if (repfs->statusArray[i]->transferURL) {
			u[i] = strdup ((repfs->statusArray[i])->SURL);
			t[i] = strdup ((repfs->statusArray[i])->transferURL);
			e[i] = NULL;
			s[i] = ((repfs->statusArray[i])->status->statusCode);
		} else if (repfs->statusArray[i]->status->explanation) {
			u[i] = strdup ((repfs->statusArray[i])->SURL);
			t[i] = NULL;
			e[i] = strdup ((repfs->statusArray[i])->status->explanation);
			s[i] = ((repfs->statusArray[i])->status->statusCode);
		} else {
			u[i] = strdup ((repfs->statusArray[i])->SURL);
			t[i] = NULL;
			e[i] = NULL;
			s[i] = ((repfs->statusArray[i])->status->statusCode);
		}
	}

	if ((*reqtoken = strdup (sreq.requestToken)) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	*sourcesurls = u;
	*turls = t;
	*explanations = e;
	*statuses = s;
	soap_end (&soap);
	soap_done (&soap);

	return (n);
}

char *
srmv2_turlfromsurl (const char *surl, const char *srm_endpoint, const char *spacetokendesc, char **protocols, int oflag, char **reqtoken, 
		char *errbuf, int errbufsz, int timeout)
{
	char **explanations;
	char *e;
	int *statuses;
	int s;
	char **sourcesurls;
	char **turls;
	char *t;
	GFAL_LONG64 zero = 0;

	if ((oflag & O_ACCMODE) == 0) {
		if (srmv2_turlsfromsurls_get (1, &surl, srm_endpoint, &zero, spacetokendesc, protocols,
			reqtoken, &sourcesurls, &turls, &statuses, &explanations, errbuf, errbufsz, timeout) <= 0) {	
			free (sourcesurls);
			free (turls);
			free (statuses);
			free (explanations);
			return NULL;
		}
	} else {
		if ((srmv2_turlsfromsurls_put (1, &surl, srm_endpoint, &zero, spacetokendesc, protocols,
			reqtoken, &sourcesurls, &turls, &statuses, &explanations, errbuf, errbufsz, timeout)) <= 0) {
			free (sourcesurls);
			free (turls);
			free (statuses);
			free (explanations);
			return NULL;
		}
	}

        t = turls[0];

	free (sourcesurls);
        free (turls);
	free (statuses);
	free (explanations);
        return (t);
}

statuscode2errno (int statuscode)
{
        switch (statuscode) {
		case SRM_USCOREINVALID_USCOREPATH:
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
