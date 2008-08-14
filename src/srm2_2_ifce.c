/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: srm2_2_ifce.c,v $ $Revision: 1.50 $ $Date: 2008/08/14 22:59:43 $
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include "gfal_api.h"
#undef SOAP_FMAC1
#define SOAP_FMAC1 static
#include "stdsoap2.h"
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
static const char gfal_remote_type[] = "SE";

static int statuscode2errno (int statuscode);
static int filestatus2returncode (int filestatus);
static const char *statuscode2errmsg (int statuscode);

static const char *srmv2_errmsg[] = {
	"SRM_SUCCESS",
	"SRM_FAILURE",
	"SRM_AUTHENTICATION_FAILURE",
	"SRM_AUTHORIZATION_FAILURE",
	"SRM_INVALID_REQUEST",
	"SRM_INVALID_PATH",
	"SRM_FILE_LIFETIME_EXPIRED",
	"SRM_SPACE_LIFETIME_EXPIRED",
	"SRM_EXCEED_ALLOCATION",
	"SRM_NO_USER_SPACE",
	"SRM_NO_FREE_SPACE",
	"SRM_DUPLICATION_ERROR",
	"SRM_NON_EMPTY_DIRECTORY",
	"SRM_TOO_MANY_RESULTS",
	"SRM_INTERNAL_ERROR",
	"SRM_FATAL_INTERNAL_ERROR",
	"SRM_NOT_SUPPORTED",
	"SRM_REQUEST_QUEUED",
	"SRM_REQUEST_INPROGRESS",
	"SRM_REQUEST_SUSPENDED",
	"SRM_ABORTED",
	"SRM_RELEASED",
	"SRM_FILE_PINNED",
	"SRM_FILE_IN_CACHE",
	"SRM_SPACE_AVAILABLE",
	"SRM_LOWER_SPACE_GRANTED",
	"SRM_DONE",
	"SRM_PARTIAL_SUCCESS",
	"SRM_REQUEST_TIMED_OUT",
	"SRM_LAST_COPY",
	"SRM_FILE_BUSY",
	"SRM_FILE_LOST",
	"SRM_FILE_UNAVAILABLE",
	"SRM_CUSTOM_STATUS"
};

srmv2_deletesurls (int nbfiles, const char **surls, const char *srm_endpoint,
		struct srmv2_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int ret;
	struct srm2__srmRmResponse_ rep;
	struct srm2__ArrayOfTSURLReturnStatus *repfs;
	struct srm2__srmRmRequest req;
	struct srm2__TReturnStatus *reqstatp;
	int i, n;
	struct soap soap;
	const char srmfunc[] = "srmRm";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof(req));

	/* NOTE: only one file in the array */
	if ((req.arrayOfSURLs = soap_malloc (&soap, sizeof(struct srm2__ArrayOfAnyURI))) == NULL) {

		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	/* issue "srmRm" request */

	if ((ret = soap_call_srm2__srmRm (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		char errmsg[ERRMSG_LEN];

		if(soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmRmResponse->returnStatus;
	repfs = rep.srmRmResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray) {
		char errmsg[ERRMSG_LEN];

		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>", gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
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
		if (!repfs->statusArray[i])
			continue;
		if (repfs->statusArray[i]->surl)
			(*statuses)[i].surl = strdup (repfs->statusArray[i]->surl);
		if (repfs->statusArray[i]->status) {
			(*statuses)[i].status = statuscode2errno(repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg (repfs->statusArray[i]->status->statusCode));
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srmv2_rmdir (const char *surl, const char *srm_endpoint, int recursive,
		struct srmv2_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int ret;
	struct srm2__srmRmdirResponse_ rep;
	struct srm2__srmRmdirRequest req;
	struct srm2__TReturnStatus *reqstatp;
	enum xsd__boolean trueoption = true_;
	struct soap soap;
	const char srmfunc[] = "srmRmdir";
	char errmsg[ERRMSG_LEN];

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof(req));
	req.SURL = (char *) surl;
	if (recursive)
		req.recursive = &trueoption;

	/* issue "srmRmdir" request */

	if ((ret = soap_call_srm2__srmRmdir (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		if(soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	if (rep.srmRmdirResponse == NULL || (reqstatp = rep.srmRmdirResponse->returnStatus) == NULL) {
		snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>", gfal_remote_type, srmfunc, srm_endpoint);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	if ((*statuses = (struct srmv2_filestatus*) calloc (1, sizeof (struct srmv2_filestatus))) == NULL) {
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	(*statuses)[0].surl = strdup (surl);
	(*statuses)[0].status = statuscode2errno (reqstatp->statusCode);
	if ((*statuses)[0].status) {
		if (reqstatp->explanation != NULL && reqstatp->explanation[0])
			asprintf (&((*statuses)[0].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
		else
			asprintf (&((*statuses)[0].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
					statuscode2errmsg (reqstatp->statusCode));
	}

	soap_end (&soap);
	soap_done (&soap);
	return (1);
}

srmv2_get (int nbfiles, const char **surls, const char *spacetokendesc, int nbprotocols, char **protocols,
		char **reqtoken, struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	char **se_types = NULL;
	char **se_endpoints = NULL;
	char *srm_endpoint = NULL;
	struct srmv2_pinfilestatus *pinfilestatuses = NULL;
	int i, r;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	i = 0;
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

	r = srmv2_gete (nbfiles, surls, srm_endpoint, spacetokendesc, 0, protocols, reqtoken,
			&pinfilestatuses, errbuf, errbufsz, timeout);

	free (srm_endpoint);

	if (r < 0 || pinfilestatuses == NULL)
		return (r);

	if ((*filestatuses = (struct srmv2_filestatus *) calloc (r, sizeof (struct srmv2_filestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < r; ++i) {
		(*filestatuses)[i].surl = pinfilestatuses[i].surl;
		(*filestatuses)[i].turl = pinfilestatuses[i].turl;
		(*filestatuses)[i].status = pinfilestatuses[i].status;
		(*filestatuses)[i].explanation = pinfilestatuses[i].explanation;
	}

	free (pinfilestatuses);
	return (r);
}

srmv2_gete (int nbfiles, const char **surls, const char *srm_endpoint, const char *spacetokendesc,
		int desiredpintime,	char **protocols, char **reqtoken, struct srmv2_pinfilestatus **filestatuses,
		char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int n;
	int ret;
	int nbproto = 0;
	struct srm2__srmPrepareToGetResponse_ rep;
	struct srm2__ArrayOfTGetRequestFileStatus *repfs;
	struct srm2__srmPrepareToGetRequest req;
	struct srm2__TReturnStatus *reqstatp;
	struct soap soap;
	static enum srm2__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	int i = 0;
	char *targetspacetoken;
	const char srmfunc[] = "PrepareToGet";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	/* issue "get" request */

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfFileRequests = 
				soap_malloc (&soap, sizeof(struct srm2__ArrayOfTGetFileRequest))) == NULL || 
			(req.arrayOfFileRequests->requestArray = 
			 soap_malloc (&soap, nbfiles * sizeof(struct srm2__TGetFileRequest *))) == NULL ||
			(req.transferParameters = 
			 soap_malloc (&soap, sizeof(struct srm2__TTransferParameters))) == NULL || 
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
	} else if ((targetspacetoken = srmv2_getbestspacetoken (spacetokendesc, srm_endpoint, 0, errbuf, errbufsz, timeout)) != NULL) {
		req.targetSpaceToken = targetspacetoken;
	} else {
		sav_errno = errno;
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	if (desiredpintime > 0)
		req.desiredPinLifeTime = &desiredpintime;

	req.desiredFileStorageType = &s_types[PERMANENT];
	req.arrayOfFileRequests->__sizerequestArray = nbfiles;

	for (i = 0; i < nbfiles; i++) {
		if ((req.arrayOfFileRequests->requestArray[i] = 	 
					soap_malloc (&soap, sizeof(struct srm2__TGetFileRequest))) == NULL) { 	 
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error"); 	 
			errno = ENOMEM; 	 
			soap_end (&soap); 	 
			soap_done (&soap); 	 
			return (-1); 	 
		}
		memset (req.arrayOfFileRequests->requestArray[i], 0, sizeof(struct srm2__TGetFileRequest));
		req.arrayOfFileRequests->requestArray[i]->sourceSURL = (char *) surls[i];
		req.arrayOfFileRequests->requestArray[i]->dirOption = NULL;
	}

	req.transferParameters->accessPattern = NULL;
	req.transferParameters->connectionType = NULL;
	req.transferParameters->arrayOfClientNetworks = NULL;
	req.transferParameters->arrayOfTransferProtocols = NULL;

	if (protocols) {
		if ((req.transferParameters->arrayOfTransferProtocols =
					soap_malloc (&soap, sizeof(struct srm2__ArrayOfString))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}

		while (protocols[nbproto] && *protocols[nbproto]) nbproto++;
		if (!protocols[nbproto]) protocols[nbproto] = "";

		req.transferParameters->arrayOfTransferProtocols->__sizestringArray = nbproto;
		req.transferParameters->arrayOfTransferProtocols->stringArray = protocols;
	}

	if ((ret = soap_call_srm2__srmPrepareToGet (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	/* return request token */
	if (reqtoken && rep.srmPrepareToGetResponse->requestToken)
		if ((*reqtoken = strdup (rep.srmPrepareToGetResponse->requestToken)) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			errno = ENOMEM;
			return (-1);
		}

	/* return file statuses */
	reqstatp = rep.srmPrepareToGetResponse->returnStatus;
	repfs = rep.srmPrepareToGetResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray) {

		char errmsg[ERRMSG_LEN];

		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>", gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_pinfilestatus *) calloc (n, sizeof(struct srmv2_pinfilestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		if (!repfs->statusArray[i])
			continue;
		memset (*filestatuses + i, 0, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->sourceSURL)
			(*filestatuses)[i].surl = strdup (repfs->statusArray[i]->sourceSURL);
		if (repfs->statusArray[i]->transferURL)
			(*filestatuses)[i].turl = strdup (repfs->statusArray[i]->transferURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = filestatus2returncode (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg(repfs->statusArray[i]->status->statusCode));
		}
		if (repfs->statusArray[i]->remainingPinTime)
			(*filestatuses)[i].pinlifetime = *(repfs->statusArray[i]->remainingPinTime);

	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);	
}

srmv2_getstatus (int nbfiles, const char **surls, const char *reqtoken, struct srmv2_filestatus **filestatuses, 
		char *errbuf, int errbufsz, int timeout)
{
	char **se_types = NULL;
	char **se_endpoints = NULL;
	char *srm_endpoint = NULL;
	struct srmv2_pinfilestatus *pinfilestatuses = NULL;
	int i, r;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	i = 0;
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

	r = srmv2_getstatuse (reqtoken, srm_endpoint, &pinfilestatuses, errbuf, errbufsz, timeout);
	free (srm_endpoint);

	if (r < 0 || pinfilestatuses == NULL)
		return (r);

	if ((*filestatuses = (struct srmv2_filestatus *) calloc (r, sizeof (struct srmv2_filestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < r; ++i) {
		(*filestatuses)[i].surl = pinfilestatuses[i].surl;
		(*filestatuses)[i].turl = pinfilestatuses[i].turl;
		(*filestatuses)[i].status = pinfilestatuses[i].status;
		(*filestatuses)[i].explanation = pinfilestatuses[i].explanation;
	}

	free (pinfilestatuses);
	return (r);
}

srmv2_getstatuse (const char *reqtoken, const char *srm_endpoint,
		struct srmv2_pinfilestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int i = 0;
	int n;
	int ret;
	struct srm2__ArrayOfTGetRequestFileStatus *repfs;
	struct srm2__TReturnStatus *reqstatp;
	struct soap soap;
	struct srm2__srmStatusOfGetRequestResponse_ srep;
	struct srm2__srmStatusOfGetRequestRequest sreq;
	const char srmfunc[] = "StatusOfGetRequest";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&sreq, 0, sizeof(sreq));
	sreq.requestToken = (char *) reqtoken;

	if ((ret = soap_call_srm2__srmStatusOfGetRequest (&soap, srm_endpoint, srmfunc, &sreq, &srep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	/* return file statuses */
	reqstatp = srep.srmStatusOfGetRequestResponse->returnStatus;
	repfs = srep.srmStatusOfGetRequestResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray) {
		char errmsg[ERRMSG_LEN];

		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>", gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_pinfilestatus *) calloc (n, sizeof(struct srmv2_pinfilestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		if (!repfs->statusArray[i])
			continue;
		memset (*filestatuses + i, 0, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->sourceSURL)
			(*filestatuses)[i].surl = strdup (repfs->statusArray[i]->sourceSURL);
		if (repfs->statusArray[i]->transferURL)
			(*filestatuses)[i].turl = strdup (repfs->statusArray[i]->transferURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = filestatus2returncode (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg(repfs->statusArray[i]->status->statusCode));
		}
		if (repfs->statusArray[i]->remainingPinTime)
			(*filestatuses)[i].pinlifetime = *(repfs->statusArray[i]->remainingPinTime);
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

/* returns space tokens associated to the space description */
srmv2_getspacetokens (const char *spacetokendesc, const char *srm_endpoint, int *nbtokens, char ***spacetokens,
		char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int i, ret;
	struct soap soap;
	struct srm2__srmGetSpaceTokensResponse_ tknrep;
	struct srm2__srmGetSpaceTokensRequest tknreq;
	struct srm2__TReturnStatus *tknrepstatp;
	struct srm2__ArrayOfString *tknrepp;
	char errmsg[ERRMSG_LEN];
	const char srmfunc[] = "GetSpaceTokens";

	if (spacetokendesc == NULL || srm_endpoint == NULL || spacetokens == NULL || nbtokens == NULL) {
		gfal_errmsg(errbuf, errbufsz, "srmv2_getspacetokens: Invalid arguments");
		errno = EINVAL;
		return (-1);
	}
	*nbtokens = 0;
	*spacetokens = NULL;

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&tknreq, 0, sizeof(tknreq));

	tknreq.userSpaceTokenDescription = (char *) spacetokendesc;

	if ((ret = soap_call_srm2__srmGetSpaceTokens (&soap, srm_endpoint, srmfunc, &tknreq, &tknrep))) {
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	tknrepstatp = tknrep.srmGetSpaceTokensResponse->returnStatus;

	if (tknrepstatp->statusCode != SRM_USCORESUCCESS) {
		sav_errno = statuscode2errno (tknrepstatp->statusCode);
		if (tknrepstatp->explanation && tknrepstatp->explanation[0])
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, tknrepstatp->explanation);
		else
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint,
					statuscode2errmsg (tknrepstatp->statusCode));

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	tknrepp = tknrep.srmGetSpaceTokensResponse->arrayOfSpaceTokens;

	if (! tknrepp) {
		snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>", gfal_remote_type, srmfunc, srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	*nbtokens = tknrepp->__sizestringArray;
	if (*nbtokens < 1 || !tknrepp->stringArray) {
		snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s: No such space token descriptor",
				gfal_remote_type, srmfunc, srm_endpoint, spacetokendesc);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = EINVAL;
		return (-1);
	}

	if ((*spacetokens = (char **) calloc (*nbtokens + 1, sizeof (char *))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < *nbtokens; ++i)
		(*spacetokens)[i] = strdup(tknrepp->stringArray[i]);

	soap_end (&soap);
	soap_done (&soap);

	return (0);
}

srmv2_getspacemd (int nbtokens, const char **spacetokens, const char *srm_endpoint, gfal_spacemd **spaces,
		char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int i, ret;
	struct soap soap;
	struct srm2__srmGetSpaceMetaDataResponse_ tknrep;
	struct srm2__srmGetSpaceMetaDataRequest tknreq;
	struct srm2__TReturnStatus *tknrepstatp;
	struct srm2__ArrayOfTMetaDataSpace *tknrepp;
	char errmsg[ERRMSG_LEN];
	const char srmfunc[] = "GetSpaceMetaData";

	if (nbtokens < 1 || spacetokens == NULL || srm_endpoint == NULL || spaces == NULL) {
		gfal_errmsg(errbuf, errbufsz, "srmv2_getspacemd: Invalid arguments");
		errno = EINVAL;
		return (-1);
	}

	if (spacetokens[nbtokens] != NULL) {
		gfal_errmsg(errbuf, errbufsz, "srmv2_getspacemd: Invalid space token number");
		errno = EINVAL;
		return (-1);
	}

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&tknreq, 0, sizeof(tknreq));

	if ((tknreq.arrayOfSpaceTokens =
				soap_malloc (&soap, nbtokens * sizeof(struct srm2__ArrayOfString))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	tknreq.arrayOfSpaceTokens->__sizestringArray = nbtokens;
	tknreq.arrayOfSpaceTokens->stringArray = (char **) spacetokens;

	if ((ret = soap_call_srm2__srmGetSpaceMetaData (&soap, srm_endpoint, srmfunc, &tknreq, &tknrep))) {
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
					gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	tknrepstatp = tknrep.srmGetSpaceMetaDataResponse->returnStatus;

	if (tknrepstatp->statusCode != SRM_USCORESUCCESS) {
		sav_errno = statuscode2errno (tknrepstatp->statusCode);
		if (tknrepstatp->explanation && tknrepstatp->explanation[0])
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, tknrepstatp->explanation);
		else
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint,
					statuscode2errmsg (tknrepstatp->statusCode));

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	tknrepp = tknrep.srmGetSpaceMetaDataResponse->arrayOfSpaceDetails;

	if (! tknrepp) {
		snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>", gfal_remote_type, srmfunc, srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}
	if (tknrepp->__sizespaceDataArray < 1 || !tknrepp->spaceDataArray) {
		snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: no valid space tokens", gfal_remote_type, srmfunc, srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = EINVAL;
		return (-1);
	}

	if ((*spaces = (gfal_spacemd *) calloc (nbtokens, sizeof (gfal_spacemd))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < nbtokens; i++) {
		if (!tknrepp->spaceDataArray[i] || !tknrepp->spaceDataArray[i]->spaceToken)
			continue;
		if (tknrepp->spaceDataArray[i]->status &&
				tknrepp->spaceDataArray[i]->status->statusCode != SRM_USCORESUCCESS) {
			int sav_errno = statuscode2errno (tknrepp->spaceDataArray[i]->status->statusCode);
			if (tknrepp->spaceDataArray[i]->status->explanation && tknrepp->spaceDataArray[i]->status->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint, tknrepp->spaceDataArray[i]->status->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", srm_endpoint,
					   statuscode2errmsg (tknrepp->spaceDataArray[i]->status->statusCode));

			gfal_errmsg (errbuf, errbufsz, errmsg);
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
			return (-1);
		}
		(*spaces)[i].spacetoken = strdup (tknrepp->spaceDataArray[i]->spaceToken);
		if (tknrepp->spaceDataArray[i]->owner)
			(*spaces)[i].owner = strdup (tknrepp->spaceDataArray[i]->owner);
		if (tknrepp->spaceDataArray[i]->totalSize)
			(*spaces)[i].totalsize = (GFAL_LONG64) *(tknrepp->spaceDataArray[i]->totalSize);
		if (tknrepp->spaceDataArray[i]->guaranteedSize)
			(*spaces)[i].guaranteedsize = (GFAL_LONG64) *(tknrepp->spaceDataArray[i]->guaranteedSize);
		if (tknrepp->spaceDataArray[i]->unusedSize)
			(*spaces)[i].unusedsize = (GFAL_LONG64) *(tknrepp->spaceDataArray[i]->unusedSize);
		if (tknrepp->spaceDataArray[i]->lifetimeAssigned)
			(*spaces)[i].lifetimeassigned = *(tknrepp->spaceDataArray[i]->lifetimeAssigned);
		if (tknrepp->spaceDataArray[i]->lifetimeLeft)
			(*spaces)[i].lifetimeleft = *(tknrepp->spaceDataArray[i]->lifetimeLeft);
		if (tknrepp->spaceDataArray[i]->retentionPolicyInfo) {
			switch (tknrepp->spaceDataArray[i]->retentionPolicyInfo->retentionPolicy) {
				case REPLICA:
					(*spaces)[i].retentionpolicy = GFAL_POLICY_REPLICA;
					break;
				case OUTPUT:
					(*spaces)[i].retentionpolicy = GFAL_POLICY_OUTPUT;
					break;
				case CUSTODIAL:
					(*spaces)[i].retentionpolicy = GFAL_POLICY_CUSTODIAL;
					break;
				default:
					(*spaces)[i].retentionpolicy = GFAL_POLICY_UNKNOWN;
			}

			if (tknrepp->spaceDataArray[i]->retentionPolicyInfo->accessLatency) {
				switch (*(tknrepp->spaceDataArray[i]->retentionPolicyInfo->accessLatency)) {
					case ONLINE:
						(*spaces)[i].accesslatency = GFAL_LATENCY_ONLINE;
						break;
					case NEARLINE:
						(*spaces)[i].accesslatency = GFAL_LATENCY_NEARLINE;
						break;
					default:
						(*spaces)[i].accesslatency = GFAL_LATENCY_UNKNOWN;
				}
			}
		}
	}

	soap_end (&soap);
	soap_done (&soap);

	return (0);
}

/* returns best space token */
	char *
srmv2_getbestspacetoken (const char *spacetokendesc, const char *srm_endpoint, GFAL_LONG64 neededsize,
		char *errbuf, int errbufsz, int timeout)
{
	int sav_errno = 0;
	int i, ret, nbtokens = -1, numtoken = -1;
	GFAL_LONG64 unusedsize = -1;
	char **spacetokens = NULL;
	gfal_spacemd *spacemd = NULL;
	char *spacetoken = NULL;
	char errmsg[ERRMSG_LEN];

	ret = srmv2_getspacetokens (spacetokendesc, srm_endpoint, &nbtokens, &spacetokens, errbuf, errbufsz, timeout);
	if (ret < 0 || spacetokens == NULL || nbtokens < 1) {
		errno = ret == 0 ? ENOMEM : errno;
		return (NULL);
	}

	ret = srmv2_getspacemd (nbtokens, (const char **) spacetokens, srm_endpoint, &spacemd, errbuf, errbufsz, timeout);
	if (ret < 0 || spacemd == NULL) {
		sav_errno = ret == 0 ? ENOMEM : errno;

		for (i = 0; i < nbtokens; ++i)
			if (spacetokens[i]) free (spacetokens[i]);

		free (spacetokens);
		errno = sav_errno;
		return (NULL);
	}

	/* Get the spacetoken with the least free space, but a bit more than needed */
	for (i = 0; i < nbtokens; ++i) {
		if (spacetokens[i]) free (spacetokens[i]);
		if (spacemd[i].unusedsize < neededsize + GFAL_SIZE_MARGIN)
			continue;
		if (numtoken < 0 || spacemd[i].unusedsize < unusedsize) {
			numtoken = i;
			unusedsize = spacemd[i].unusedsize;
		}
	}

	if (numtoken < 0) {
		/* no suitable space token */
		snprintf (errmsg, ERRMSG_LEN, "%s: no associated space token with enough free space", spacetokendesc);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (NULL);
	}

	spacetoken = spacemd[numtoken].spacetoken;
	spacemd[numtoken].spacetoken = NULL;  // prevent it to be freed
	gfal_spacemd_free (nbtokens, spacemd);
	free (spacetokens);
	return (spacetoken);
}

/* tries to create all directories in 'dest_file' */
	int
srmv2_makedirp (const char *dest_file, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	char file[1024];
	int flags;
	int nbslash = 0;
	int sav_errno = 0;
	char *p;
	struct srm2__srmMkdirResponse_ rep;
	struct srm2__srmMkdirRequest req;
	struct srm2__TReturnStatus *repstatp;
	struct soap soap;
	char errmsg[ERRMSG_LEN];
	int slashes_to_ignore;
	const char srmfunc[] = "Mkdir";

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
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof (struct srm2__srmMkdirRequest));

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

	if (soap_call_srm2__srmMkdir (&soap, srm_endpoint, srmfunc, &req, &rep)) {
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
					gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	repstatp = rep.srmMkdirResponse->returnStatus;

	if (repstatp->statusCode != SRM_USCORESUCCESS) {
		sav_errno = statuscode2errno (repstatp->statusCode);
		if (sav_errno == EEXIST || sav_errno == EACCES) {
			/* EEXIST - dir already exists, nothing to do
			 * EACCES - dir *may* already exists, but no authZ on parent */
			return (0);
		} else if (sav_errno != ENOENT) {
			if (repstatp->explanation && repstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, dest_file, repstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, dest_file,
						statuscode2errmsg(repstatp->statusCode));

			gfal_errmsg(errbuf, errbufsz, errmsg);
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
			return (-1);
		}
	} else {
		/* The directory was successfuly created */
		return (0);
	}

	*p = '/';
	if ((p = strchr (file, '?')) == NULL) {
		p = file;
		slashes_to_ignore = 3;
	} else {
		++p;
		slashes_to_ignore = 2;
	}

	while ((p = strchr (p, '/'))) {
		if (nbslash < slashes_to_ignore) {	/* skip first slashes */
			++nbslash;
			do
				++p;
			while (*p == '/');
			continue;
		}

		*p = '\0';

		/* try to create directory */
		memset (&req, 0, sizeof(req));
		req.SURL = (char *) file;

		if (soap_call_srm2__srmMkdir (&soap, srm_endpoint, srmfunc, &req, &rep)) {
			if (soap.fault != NULL && soap.fault->faultstring != NULL)
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
			else if (soap.error == SOAP_EOF)
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

			gfal_errmsg(errbuf, errbufsz, errmsg);
			soap_end (&soap);
			soap_done (&soap);
			errno = ECOMM;
			return (-1);
		}

		repstatp = rep.srmMkdirResponse->returnStatus;

		if (repstatp->statusCode != SRM_USCORESUCCESS) {
			sav_errno = statuscode2errno(repstatp->statusCode);
			if (sav_errno != EEXIST && sav_errno != EACCES) {
				if (repstatp->explanation && repstatp->explanation[0])
					snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, file, repstatp->explanation);
				else
					snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, file,
							statuscode2errmsg(repstatp->statusCode));

				gfal_errmsg(errbuf, errbufsz, errmsg);
				soap_end (&soap);
				soap_done (&soap);
				errno = sav_errno;
				return (-1);
			}
		}

		*p++ = '/';	/* restore slash */
		while (*p == '/')
			p++;
	}

	soap_end (&soap);
	soap_done (&soap);
	strncpy (lastcreated_dir, dest_file, 1024);
	return (0);
}

srmv2_prestage (int nbfiles, const char **surls, const char *spacetokendesc, int nbprotocols, char **protocols, int desiredpintime,
		char **reqtoken, struct srmv2_pinfilestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	char **se_types = NULL;
	char **se_endpoints = NULL;
	char *srm_endpoint = NULL;
	int i, r;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	i = 0;
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

	r = srmv2_prestagee (nbfiles, surls, srm_endpoint, spacetokendesc, protocols, desiredpintime, reqtoken,
			filestatuses, errbuf, errbufsz, timeout);

	free (srm_endpoint);
	return (r);
}

srmv2_prestagee (int nbfiles, const char **surls, const char *srm_endpoint, const char *spacetokendesc,
		char **protocols, int desiredpintime, char **reqtoken, struct srmv2_pinfilestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int i = 0;
	int n;
	int ret;
	struct srm2__srmBringOnlineResponse_ rep;
	struct srm2__ArrayOfTBringOnlineRequestFileStatus *repfs;
	struct srm2__srmBringOnlineRequest req;
	struct srm2__TReturnStatus *reqstatp;
	struct soap soap;
	static enum srm2__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	char *targetspacetoken;
	int nbproto = 0;
	const char srmfunc[] = "BringOnline";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	/* issue "bringonline" request */

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfFileRequests = 
				soap_malloc (&soap, sizeof(struct srm2__ArrayOfTGetFileRequest))) == NULL || 
			(req.arrayOfFileRequests->requestArray = 
			 soap_malloc (&soap, nbfiles * sizeof(struct srm2__TGetFileRequest *))) == NULL ||
			(req.transferParameters = 
			 soap_malloc (&soap, sizeof(struct srm2__TTransferParameters))) == NULL || 
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
	} else if ((targetspacetoken = srmv2_getbestspacetoken (spacetokendesc, srm_endpoint, 0, errbuf, errbufsz, timeout)) != NULL) {
		req.targetSpaceToken = targetspacetoken;
	} else {
		sav_errno = errno;
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	req.authorizationID = NULL;
	req.userRequestDescription = NULL;
	req.storageSystemInfo = NULL;
	req.desiredFileStorageType = &s_types[PERMANENT];
	req.desiredTotalRequestTime = NULL;
	req.desiredLifeTime = &desiredpintime;
	req.targetFileRetentionPolicyInfo = NULL;
	req.deferredStartTime = NULL;

	req.arrayOfFileRequests->__sizerequestArray = nbfiles;

	for (i = 0; i < nbfiles; i++) {
		if ((req.arrayOfFileRequests->requestArray[i] = 	 
					soap_malloc (&soap, sizeof(struct srm2__TGetFileRequest))) == NULL) { 	 
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error"); 	 
			errno = ENOMEM; 	 
			soap_end (&soap); 	 
			soap_done (&soap); 	 
			return (-1); 	 
		}
		memset (req.arrayOfFileRequests->requestArray[i], 0, sizeof(struct srm2__TGetFileRequest));
		req.arrayOfFileRequests->requestArray[i]->sourceSURL = (char *) surls[i];
		req.arrayOfFileRequests->requestArray[i]->dirOption = NULL;
	}

	req.transferParameters->accessPattern = NULL;
	req.transferParameters->connectionType = NULL;
	req.transferParameters->arrayOfClientNetworks = NULL;
	req.transferParameters->arrayOfTransferProtocols = NULL;

	if (protocols) {
		if ((req.transferParameters->arrayOfTransferProtocols =
					soap_malloc (&soap, sizeof(struct srm2__ArrayOfString))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}

		while (protocols[nbproto] && *protocols[nbproto]) nbproto++;
		if (!protocols[nbproto]) protocols[nbproto] = "";

		req.transferParameters->arrayOfTransferProtocols->__sizestringArray = nbproto;
		req.transferParameters->arrayOfTransferProtocols->stringArray = protocols;
	}

	if ((ret = soap_call_srm2__srmBringOnline (&soap, srm_endpoint, "BringOnline", &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	/* return request token */
	if (reqtoken && rep.srmBringOnlineResponse->requestToken)
		if ((*reqtoken = strdup (rep.srmBringOnlineResponse->requestToken)) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			errno = ENOMEM;
			return (-1);
		}

	/* return file statuses */
	reqstatp = rep.srmBringOnlineResponse->returnStatus;
	repfs = rep.srmBringOnlineResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray) {
		char errmsg[ERRMSG_LEN];

		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>", gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_pinfilestatus *) calloc (n, sizeof(struct srmv2_pinfilestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		if (!repfs->statusArray[i])
			continue;
		memset (*filestatuses + i, 0, sizeof (struct srmv2_pinfilestatus));
		if (repfs->statusArray[i]->sourceSURL)
			(*filestatuses)[i].surl = strdup (repfs->statusArray[i]->sourceSURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = filestatus2returncode (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg (repfs->statusArray[i]->status->statusCode));
		}
		if (repfs->statusArray[i]->remainingPinTime)
			(*filestatuses)[i].pinlifetime = *(repfs->statusArray[i]->remainingPinTime);
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);	
}

srmv2_prestagestatus (int nbfiles, const char **surls, const char *reqtoken, struct srmv2_pinfilestatus **filestatuses, 
		char *errbuf, int errbufsz, int timeout)
{
	char **se_types = NULL;
	char **se_endpoints = NULL;
	char *srm_endpoint = NULL;
	int i, r;

	if (setypesandendpointsfromsurl (surls[0], &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	i = 0;
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

	r = srmv2_prestagestatuse (reqtoken, srm_endpoint, filestatuses, errbuf, errbufsz, timeout);

	free (srm_endpoint);
	return (r);
}

srmv2_prestagestatuse (const char *reqtoken, const char *srm_endpoint, struct srmv2_pinfilestatus **filestatuses, 
		char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int i = 0;
	int n;
	int ret;
	struct srm2__ArrayOfTBringOnlineRequestFileStatus *repfs;
	struct srm2__TReturnStatus *reqstatp;
	struct soap soap;
	struct srm2__srmStatusOfBringOnlineRequestResponse_ srep;
	struct srm2__srmStatusOfBringOnlineRequestRequest sreq;
	const char srmfunc[] = "StatusOfBringOnlineRequest";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&sreq, 0, sizeof(sreq));
	sreq.requestToken = (char *) reqtoken;

	if ((ret = soap_call_srm2__srmStatusOfBringOnlineRequest (&soap, srm_endpoint, srmfunc, &sreq, &srep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
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

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray) {
		char errmsg[ERRMSG_LEN];

		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>", gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_pinfilestatus *) calloc (n, sizeof(struct srmv2_pinfilestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		if (!repfs->statusArray[i])
			continue;
		memset (*filestatuses + i, 0, sizeof (struct srmv2_pinfilestatus));
		if (repfs->statusArray[i]->sourceSURL)
			(*filestatuses)[i].surl = strdup (repfs->statusArray[i]->sourceSURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = filestatus2returncode (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg (repfs->statusArray[i]->status->statusCode));
		}
		if (repfs->statusArray[i]->remainingPinTime)
			(*filestatuses)[i].pinlifetime = *(repfs->statusArray[i]->remainingPinTime);
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);	
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
	int sav_errno = 0;
	int ret;
	struct srm2__srmPutDoneResponse_ rep;
	struct srm2__ArrayOfTSURLReturnStatus *repfs;
	struct srm2__srmPutDoneRequest req;
	struct srm2__TReturnStatus *reqstatp;
	int i, n;
	struct soap soap;
	const char srmfunc[] = "PutDone";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof(req));

	req.requestToken = (char *) reqtoken;

	/* NOTE: only one SURL in the array */
	if ((req.arrayOfSURLs = 
				soap_malloc (&soap, sizeof(struct srm2__ArrayOfAnyURI))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	if ((ret = soap_call_srm2__srmPutDone (&soap, srm_endpoint, srmfunc , &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmPutDoneResponse->returnStatus;
	repfs = rep.srmPutDoneResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray) {
		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>", gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
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
		if (!repfs->statusArray[i])
			continue;
		memset (*statuses + i, 0, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->surl)
			(*statuses)[i].surl = strdup (repfs->statusArray[i]->surl);
		if (repfs->statusArray[i]->status) {
			(*statuses)[i].status = statuscode2errno (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg (repfs->statusArray[i]->status->statusCode));
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);
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
		memset (*filestatuses + i, 0, sizeof (struct srmv2_filestatus));
		(*filestatuses)[i].surl = strdup (surls[i]);
		(*filestatuses)[i].status = 0;
	}

	return (nbfiles);
}

srmv2_turlsfromsurls_get (int nbfiles, const char **surls, const char *srm_endpoint, int desiredpintime, const char *spacetokendesc, 
		char **protocols, char **reqtoken, struct srmv2_pinfilestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	struct srm2__srmAbortRequestRequest abortreq;
	struct srm2__srmAbortRequestResponse_ abortrep;
	time_t endtime = 0;
	int flags;
	int sav_errno = 0;
	int i;
	int n;
	int nbproto = 0;
	int r = 0;
	char *r_token;
	int ret;
	struct srm2__srmPrepareToGetResponse_ rep;
	struct srm2__ArrayOfTGetRequestFileStatus *repfs;
	struct srm2__srmPrepareToGetRequest req;
	struct srm2__TReturnStatus *reqstatp;
	struct soap soap;
	struct srm2__srmStatusOfGetRequestResponse_ srep;
	struct srm2__srmStatusOfGetRequestRequest sreq;
	static enum srm2__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	char *targetspacetoken;
	const char srmfunc[] = "PrepareToGet";
	const char srmfunc_status[] = "StatusOfGetRequest";

retry:
	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	/* issue "get" request */

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfFileRequests = 
				soap_malloc (&soap, sizeof(struct srm2__ArrayOfTGetFileRequest))) == NULL || 
			(req.arrayOfFileRequests->requestArray = 
			 soap_malloc (&soap, nbfiles * sizeof(struct srm2__TGetFileRequest *))) == NULL ||
			(req.transferParameters = 
			 soap_malloc (&soap, sizeof(struct srm2__TTransferParameters))) == NULL || 
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
	} else if ((targetspacetoken = srmv2_getbestspacetoken (spacetokendesc, srm_endpoint, 0, errbuf, errbufsz, timeout)) != NULL) {
		req.targetSpaceToken = targetspacetoken;
	} else {
		sav_errno = errno;
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	if (desiredpintime > 0)
		req.desiredPinLifeTime = &desiredpintime;

	req.desiredFileStorageType = &s_types[PERMANENT];
	req.arrayOfFileRequests->__sizerequestArray = nbfiles;

	for (i = 0; i < nbfiles; i++) {
		if ((req.arrayOfFileRequests->requestArray[i] = 	 
					soap_malloc (&soap, sizeof(struct srm2__TGetFileRequest))) == NULL) { 	 
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error"); 	 
			errno = ENOMEM; 	 
			soap_end (&soap); 	 
			soap_done (&soap); 	 
			return (-1); 	 
		}
		memset (req.arrayOfFileRequests->requestArray[i], 0, sizeof(struct srm2__TGetFileRequest));
		req.arrayOfFileRequests->requestArray[i]->sourceSURL = (char *) surls[i];
		req.arrayOfFileRequests->requestArray[i]->dirOption = NULL;
	}

	req.transferParameters->accessPattern = NULL;
	req.transferParameters->connectionType = NULL;
	req.transferParameters->arrayOfClientNetworks = NULL;
	req.transferParameters->arrayOfTransferProtocols = NULL;

	if (protocols) {
		if ((req.transferParameters->arrayOfTransferProtocols =
					soap_malloc (&soap, sizeof(struct srm2__ArrayOfString))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}

		while (protocols[nbproto] && *protocols[nbproto]) nbproto++;
		if (!protocols[nbproto]) protocols[nbproto] = "";

		req.transferParameters->arrayOfTransferProtocols->__sizestringArray = nbproto;
		req.transferParameters->arrayOfTransferProtocols->stringArray = protocols;
	}

	if ((ret = soap_call_srm2__srmPrepareToGet (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s", gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout", gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg (errbuf, errbufsz, errmsg);
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

	/* INTERNAL_ERROR = transient error => automatic retry */
	while (reqstatp->statusCode == SRM_USCOREINTERNAL_USCOREERROR) {

		if (timeout > 0 && time(NULL) > endtime) {
			char errmsg[ERRMSG_LEN];
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: User timeout over", gfal_remote_type, srmfunc, srm_endpoint);
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

		if ((ret = soap_call_srm2__srmStatusOfGetRequest (&soap, srm_endpoint, srmfunc_status, &sreq, &srep))) {
			char errmsg[ERRMSG_LEN];
			if (soap.fault != NULL && soap.fault->faultstring != NULL)
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc_status, srm_endpoint, soap.fault->faultstring);
			else if (soap.error == SOAP_EOF)
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
						gfal_remote_type, srmfunc_status, srm_endpoint);

			gfal_errmsg (errbuf, errbufsz, errmsg);
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

			if ((ret = soap_call_srm2__srmAbortRequest (&soap, srm_endpoint, "AbortRequest", &abortreq, &abortrep))) {
				if (soap.fault != NULL && soap.fault->faultstring != NULL)
					snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
							gfal_remote_type, srmfunc_status, srm_endpoint, soap.fault->faultstring);
				else if (soap.error == SOAP_EOF)
					snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
							gfal_remote_type, srmfunc_status, srm_endpoint);

				sav_errno = ECOMM;
			} else {
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: User timeout over",
						gfal_remote_type, srmfunc_status, srm_endpoint);
				sav_errno = ETIMEDOUT;
			}

			gfal_errmsg (errbuf, errbufsz, errmsg);
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
			return (-1);
		}
	}

	if (!repfs || repfs->__sizestatusArray < nbfiles || !repfs->statusArray) {
		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc_status, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc_status, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>",
					gfal_remote_type, srmfunc_status, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_pinfilestatus *) calloc (n, sizeof (struct srmv2_pinfilestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		if (!repfs->statusArray[i])
			continue;
		memset (*filestatuses + i, 0, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->sourceSURL)
			(*filestatuses)[i].surl = strdup (repfs->statusArray[i]->sourceSURL);
		if (repfs->statusArray[i]->transferURL)
			(*filestatuses)[i].turl = strdup (repfs->statusArray[i]->transferURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = statuscode2errno (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc_status, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc_status, reqstatp->explanation);
			else
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc_status,
						statuscode2errmsg (repfs->statusArray[i]->status->statusCode));
		}
		if (repfs->statusArray[i]->remainingPinTime)
			(*filestatuses)[i].pinlifetime = *(repfs->statusArray[i]->remainingPinTime);
	}

	if (reqtoken && sreq.requestToken)
		if ((*reqtoken = strdup (sreq.requestToken)) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			errno = ENOMEM;
			return (-1);
		}

	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srmv2_turlsfromsurls_put (int nbfiles, const char **surls, const char *srm_endpoint, GFAL_LONG64 *filesizes,
		int desiredpintime, const char *spacetokendesc, char **protocols, char **reqtoken,
		struct srmv2_pinfilestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	struct srm2__srmAbortRequestRequest abortreq;
	struct srm2__srmAbortRequestResponse_ abortrep;
	time_t endtime = 0;
	int flags;
	int sav_errno = 0;
	int i;
	int n;
	int nbproto = 0;
	int r = 0;
	char *r_token;
	int ret;
	struct srm2__srmPrepareToPutResponse_ rep;
	struct srm2__ArrayOfTPutRequestFileStatus *repfs;
	struct srm2__srmPrepareToPutRequest req;
	struct srm2__TReturnStatus *reqstatp;
	struct soap soap;
	struct srm2__srmStatusOfPutRequestResponse_ srep;
	struct srm2__srmStatusOfPutRequestRequest sreq;
	static enum srm2__TFileStorageType s_types[] = {VOLATILE, DURABLE, PERMANENT};
	char *targetspacetoken;
	GFAL_LONG64 totalsize = 0;
	char errmsg[ERRMSG_LEN];
	const char srmfunc[] = "PrepareToPut";
	const char srmfunc_status[] = "StatusOfPutRequest";

retry:
	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfFileRequests =
				soap_malloc (&soap, sizeof(struct srm2__ArrayOfTPutFileRequest))) == NULL ||
			(req.arrayOfFileRequests->requestArray =
			 soap_malloc (&soap, nbfiles * sizeof(struct srm2__TPutFileRequest *))) == NULL ||
			(req.transferParameters =
			 soap_malloc (&soap, sizeof(struct srm2__TTransferParameters))) == NULL ||
			(req.targetSpaceToken =
			 soap_malloc (&soap, sizeof(char *))) == NULL) {

		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	if (desiredpintime > 0)
		req.desiredPinLifeTime = &desiredpintime;

	req.desiredFileStorageType = &s_types[PERMANENT];
	req.arrayOfFileRequests->__sizerequestArray = nbfiles;

	for (i = 0; i < nbfiles; i++) {
		if ((req.arrayOfFileRequests->requestArray[i] = 	 
					soap_malloc (&soap, sizeof(struct srm2__TPutFileRequest))) == NULL) { 	 
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error"); 	 
			errno = ENOMEM; 	 
			soap_end (&soap); 	 
			soap_done (&soap); 	 
			return (-1); 	 
		}
		memset (req.arrayOfFileRequests->requestArray[i], 0, sizeof(struct srm2__TPutFileRequest));
		req.arrayOfFileRequests->requestArray[i]->targetSURL = (char *) surls[i];

		if ((req.arrayOfFileRequests->requestArray[i]->expectedFileSize = soap_malloc (&soap, sizeof(ULONG64))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		*(req.arrayOfFileRequests->requestArray[i]->expectedFileSize) = filesizes[i];
		totalsize += filesizes[i]; // compute total size to determine best space token
	}

	req.transferParameters->accessPattern = NULL;
	req.transferParameters->connectionType = NULL;
	req.transferParameters->arrayOfClientNetworks = NULL;
	req.transferParameters->arrayOfTransferProtocols = NULL;

	if (protocols) {
		if ((req.transferParameters->arrayOfTransferProtocols =
					soap_malloc (&soap, sizeof(struct srm2__ArrayOfString))) == NULL) {
			gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
			errno = ENOMEM;
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}

		while (protocols[nbproto] && *protocols[nbproto]) nbproto++;
		if (!protocols[nbproto]) protocols[nbproto] = "";

		req.transferParameters->arrayOfTransferProtocols->__sizestringArray = nbproto;
		req.transferParameters->arrayOfTransferProtocols->stringArray = protocols;
	}

	if (!spacetokendesc) {
		req.targetSpaceToken = NULL;
	} else if ((targetspacetoken = srmv2_getbestspacetoken (spacetokendesc, srm_endpoint,
					totalsize, errbuf, errbufsz, timeout)) != NULL) {
		req.targetSpaceToken = targetspacetoken;
	} else {
		sav_errno = errno;
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	/* Create sub-directories of SURLs */
	for (i = 0; i < nbfiles; ++i) {
		if (srmv2_makedirp (surls[i], srm_endpoint, errbuf, errbufsz, timeout) < 0)
			return (-1);
	}

	if ((ret = soap_call_srm2__srmPrepareToPut (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
					gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg (errbuf, errbufsz, errmsg);
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

	/* INTERNAL_ERROR = transient error => automatic retry */
	while (reqstatp->statusCode == SRM_USCOREINTERNAL_USCOREERROR) {

		if (timeout > 0 && time(NULL) > endtime) {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: User timeout over",
					gfal_remote_type, srmfunc, srm_endpoint);
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

		if ((ret = soap_call_srm2__srmStatusOfPutRequest (&soap, srm_endpoint, srmfunc_status, &sreq, &srep))) {
			if (soap.fault != NULL && soap.fault->faultstring != NULL)
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc_status, srm_endpoint, soap.fault->faultstring);
			else if (soap.error == SOAP_EOF)
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
						gfal_remote_type, srmfunc_status, srm_endpoint);

			gfal_errmsg (errbuf, errbufsz, errmsg);
			soap_end (&soap);
			soap_done (&soap);
			errno = ECOMM;
			return (-1);
		}

		reqstatp = srep.srmStatusOfPutRequestResponse->returnStatus;
		repfs = srep.srmStatusOfPutRequestResponse->arrayOfFileStatuses;

		/* if user timeout has passed, abort the request */
		if (timeout > 0 && time(NULL) > endtime) {
			const char srmfunc_abort[] = "AbortRequest";
			abortreq.requestToken = r_token;

			if ((ret = soap_call_srm2__srmAbortRequest (&soap, srm_endpoint, srmfunc_abort, &abortreq, &abortrep))) {
				if (soap.fault != NULL && soap.fault->faultstring != NULL)
					snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
							gfal_remote_type, srmfunc_abort, srm_endpoint, soap.fault->faultstring);
				else if (soap.error == SOAP_EOF)
					snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
							gfal_remote_type, srmfunc_abort, srm_endpoint);

				sav_errno = ECOMM;
			} else {
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: User timeout over",
						gfal_remote_type, srmfunc_status, srm_endpoint);
				sav_errno = ETIMEDOUT;
			}

			gfal_errmsg (errbuf, errbufsz, errmsg);
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
			return (-1);
		}
	}

	if (reqstatp->statusCode == SRM_USCORESPACE_USCORELIFETIME_USCOREEXPIRED) {
		snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Space lifetime expired",
				gfal_remote_type, srmfunc_status, srm_endpoint);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = statuscode2errno(reqstatp->statusCode);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	if (!repfs || repfs->__sizestatusArray < nbfiles || !repfs->statusArray) {
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc_status, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc_status, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>",
					gfal_remote_type, srmfunc_status, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg (errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	n = repfs->__sizestatusArray;

	if ((*filestatuses = (struct srmv2_pinfilestatus *) calloc (n, sizeof (struct srmv2_pinfilestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		if (!repfs->statusArray[i])
			continue;
		memset (*filestatuses + i, 0, sizeof (struct srmv2_filestatus));
		if (repfs->statusArray[i]->SURL)
			(*filestatuses)[i].surl = strdup (repfs->statusArray[i]->SURL);
		if (repfs->statusArray[i]->transferURL)
			(*filestatuses)[i].turl = strdup (repfs->statusArray[i]->transferURL);
		if (repfs->statusArray[i]->status) {
			(*filestatuses)[i].status = statuscode2errno (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc_status, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc_status, reqstatp->explanation);
			else
				asprintf (&((*filestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc_status,
						statuscode2errmsg (repfs->statusArray[i]->status->statusCode));
		}
		if (repfs->statusArray[i]->remainingPinLifetime)
			(*filestatuses)[i].pinlifetime = *(repfs->statusArray[i]->remainingPinLifetime);
	}

	if (reqtoken && sreq.requestToken)
		if ((*reqtoken = strdup (sreq.requestToken)) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			errno = ENOMEM;
			return (-1);
		}

	soap_end (&soap);
	soap_done (&soap);

	return (n);
}

#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
static int
copy_md (struct srm2__TReturnStatus *reqstatp, struct srm2__ArrayOfTMetaDataPathDetail *repfs,
		struct srmv2_mdfilestatus **statuses)
{
	int i, n, r;
	const char srmfunc[] = "Ls";

	if (reqstatp == NULL || repfs == NULL || statuses == NULL) {
		errno = EFAULT;
		return (-1);
	}

	n = repfs->__sizepathDetailArray;

	if ((*statuses = (struct srmv2_mdfilestatus *) calloc (n, sizeof (struct srmv2_mdfilestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; ++i) {
		if (!repfs->pathDetailArray[i])
			continue;
		memset (*statuses + i, 0, sizeof(struct srmv2_mdfilestatus));
		if (repfs->pathDetailArray[i]->path)
			(*statuses)[i].surl = strdup (repfs->pathDetailArray[i]->path);
		if (repfs->pathDetailArray[i]->status)
			(*statuses)[i].status = statuscode2errno(repfs->pathDetailArray[i]->status->statusCode);
		if ((*statuses)[i].status) {
			if (repfs->pathDetailArray[i]->status->explanation && repfs->pathDetailArray[i]->status->explanation[0])
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->pathDetailArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg(repfs->pathDetailArray[i]->status->statusCode));
			continue;
		} 
		if (repfs->pathDetailArray[i]->size)
			(*statuses)[i].stat.st_size = *(repfs->pathDetailArray[i]->size);
		else
			(*statuses)[i].stat.st_size = 0;
		if (repfs->pathDetailArray[i]->fileLocality) {
            switch (*(repfs->pathDetailArray[i]->fileLocality)) {
                case ONLINE_:
                    (*statuses)[i].locality = GFAL_LOCALITY_ONLINE_;
                    break;
                case NEARLINE_:
                    (*statuses)[i].locality = GFAL_LOCALITY_NEARLINE_;
                    break;
                case ONLINE_USCOREAND_USCORENEARLINE:
                    (*statuses)[i].locality = GFAL_LOCALITY_ONLINE_USCOREAND_USCORENEARLINE;
                    break;
                case LOST:
                    (*statuses)[i].locality = GFAL_LOCALITY_LOST;
                    break;
                case NONE_:
                    (*statuses)[i].locality = GFAL_LOCALITY_NONE_;
                    break;
                case UNAVAILABLE:
                    (*statuses)[i].locality = GFAL_LOCALITY_UNAVAILABLE;
                    break;
                default:
                    (*statuses)[i].locality = GFAL_LOCALITY_UNKNOWN;
            }
        }
		(*statuses)[i].stat.st_uid = 2;//TODO: create haseh placeholder for string<->uid/gid mapping
		(*statuses)[i].stat.st_gid = 2;
		(*statuses)[i].stat.st_nlink = 1;
		if (repfs->pathDetailArray[i]->otherPermission)
			(*statuses)[i].stat.st_mode = *(repfs->pathDetailArray[i]->otherPermission);
		if (repfs->pathDetailArray[i]->groupPermission)
			(*statuses)[i].stat.st_mode |= repfs->pathDetailArray[i]->groupPermission->mode << 3;
		if (repfs->pathDetailArray[i]->ownerPermission)
			(*statuses)[i].stat.st_mode |= repfs->pathDetailArray[i]->ownerPermission->mode << 6;
		if (repfs->pathDetailArray[i]->type) {
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

		if (repfs->pathDetailArray[i]->arrayOfSubPaths) {
			r = copy_md (reqstatp, repfs->pathDetailArray[i]->arrayOfSubPaths, &((*statuses)[i].subpaths));

			if (r < 0)
				return (r);

			(*statuses)[i].nbsubpaths = r;
		}
	}

	return (n);
}

srmv2_getfilemd (int nbfiles, const char **surls, const char *srm_endpoint, int numlevels, int *offset, int count,
		struct srmv2_mdfilestatus **statuses, char **reqtoken, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int ret;
	struct soap soap;
	enum xsd__boolean trueoption = true_;
	struct srm2__srmLsRequest req;
	struct srm2__srmLsResponse_ rep;
	struct srm2__TReturnStatus *reqstatp;
	struct srm2__ArrayOfTMetaDataPathDetail *repfs;
	int n;
	const char srmfunc[] = "Ls";


	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfSURLs = soap_malloc (&soap, sizeof(struct srm2__ArrayOfAnyURI))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.fullDetailedList = &trueoption;
	req.numOfLevels = &numlevels;
	if (offset && *offset > 0) req.offset = offset;
	if (count > 0) req.count = &count;
	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	/* issue "srmLs" request */

	if ((ret = soap_call_srm2__srmLs (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if(soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
					gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmLsResponse->returnStatus;
	repfs = rep.srmLsResponse->details;

	if (reqtoken && rep.srmLsResponse->requestToken)
		if ((*reqtoken = strdup (rep.srmLsResponse->requestToken)) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			errno = ENOMEM;
			return (-1);
		}

	if (!repfs || repfs->__sizepathDetailArray <= 0 || !repfs->pathDetailArray) {
		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREDONE &&
				reqstatp->statusCode != SRM_USCORETOO_USCOREMANY_USCORERESULTS) {
			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>",
					gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	n = copy_md (reqstatp, repfs, statuses);
	
	if (n >= 0) {
		if (n == 1 && reqstatp->statusCode == SRM_USCORETOO_USCOREMANY_USCORERESULTS &&
				repfs->pathDetailArray[0] != NULL && repfs->pathDetailArray[0]->arrayOfSubPaths != NULL)
			/* offset is only supported for a single directory listing */
			*offset += repfs->pathDetailArray[0]->arrayOfSubPaths->__sizepathDetailArray;
		else
			*offset = 0;
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);
}
#endif

srmv2_pin (int nbfiles, const char **surls, const char *srm_endpoint, const char *reqtoken, int pintime,
		struct srmv2_pinfilestatus **pinfilestatuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int n;
	int ret;
	struct srm2__ArrayOfTSURLLifetimeReturnStatus *repfs;
	struct srm2__srmExtendFileLifeTimeResponse_ rep;
	struct srm2__srmExtendFileLifeTimeRequest req;
	struct srm2__TReturnStatus *reqstatp;
	struct soap soap;
	int i = 0;
	const char srmfunc[] = "ExtendFileLifeTime";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	/* issue "extendfilelifetime" request */

	memset (&req, 0, sizeof(req));

	if ((req.arrayOfSURLs = soap_malloc (&soap, sizeof(struct srm2__ArrayOfAnyURI))) == NULL) {

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

	if ((ret = soap_call_srm2__srmExtendFileLifeTime (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
					gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	/* return file statuses */
	reqstatp = rep.srmExtendFileLifeTimeResponse->returnStatus;
	repfs = rep.srmExtendFileLifeTimeResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray) {
		char errmsg[ERRMSG_LEN];

		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>",
					gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
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
		if (!repfs->statusArray[i])
			continue;
		memset (*pinfilestatuses + i, 0, sizeof (struct srmv2_pinfilestatus));
		if (repfs->statusArray[i]->surl)
			(*pinfilestatuses)[i].surl = strdup (repfs->statusArray[i]->surl);
		if (repfs->statusArray[i]->pinLifetime)
			(*pinfilestatuses)[i].pinlifetime = *(repfs->statusArray[i]->pinLifetime);
		if (repfs->statusArray[i]->status) {
			(*pinfilestatuses)[i].status = filestatus2returncode (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*pinfilestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*pinfilestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*pinfilestatuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg (repfs->statusArray[i]->status->statusCode));
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);	
}

srmv2_release (int nbfiles, const char **surls, const char *srm_endpoint, const char *reqtoken,
		struct srmv2_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int ret;
	struct srm2__srmReleaseFilesResponse_ rep;
	struct srm2__ArrayOfTSURLReturnStatus *repfs;
	struct srm2__srmReleaseFilesRequest req;
	struct srm2__TReturnStatus *reqstatp;
	int i, n;
	struct soap soap;
	const char srmfunc[] = "ReleaseFiles";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof(req));

	req.requestToken = (char *) reqtoken;

	/* NOTE: only one SURL in the array */
	if ((req.arrayOfSURLs =
				soap_malloc (&soap, sizeof(struct srm2__ArrayOfAnyURI))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	if ((ret = soap_call_srm2__srmReleaseFiles (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
					gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmReleaseFilesResponse->returnStatus;
	repfs = rep.srmReleaseFilesResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray) {
		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>",
					gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
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
		if (!repfs->statusArray[i])
			continue;
		memset (*statuses + i, 0, sizeof (struct srmv2_pinfilestatus));
		if (repfs->statusArray[i]->surl)
			(*statuses)[i].surl = strdup (repfs->statusArray[i]->surl);
		if (repfs->statusArray[i]->status) {
			(*statuses)[i].status = statuscode2errno (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg (repfs->statusArray[i]->status->statusCode));
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srmv2_abortrequest (const char *srm_endpoint, const char *reqtoken, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int ret;
	struct srm2__srmAbortRequestResponse_ rep;
	struct srm2__srmAbortRequestRequest req;
	struct srm2__TReturnStatus *reqstatp;
	struct soap soap;
    char errmsg[ERRMSG_LEN];
	const char srmfunc[] = "AbortRequest";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof(req));

	req.requestToken = (char *) reqtoken;

	if ((ret = soap_call_srm2__srmAbortRequest (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
					gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmAbortRequestResponse->returnStatus;
    ret = 0;

    if (reqstatp->statusCode != SRM_USCORESUCCESS &&
            reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

        sav_errno = statuscode2errno (reqstatp->statusCode);
        if (reqstatp->explanation && reqstatp->explanation[0])
            snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
		else
            snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint,
					statuscode2errmsg (reqstatp->statusCode));

		gfal_errmsg(errbuf, errbufsz, errmsg);
        ret = -1;
    }

    soap_end (&soap);
    soap_done (&soap);
	errno = sav_errno;
    return (ret);
}

srmv2_abortfiles (int nbfiles, const char **surls, const char *srm_endpoint, const char *reqtoken,
		struct srmv2_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int ret;
	struct srm2__srmAbortFilesResponse_ rep;
	struct srm2__ArrayOfTSURLReturnStatus *repfs;
	struct srm2__srmAbortFilesRequest req;
	struct srm2__TReturnStatus *reqstatp;
	int i, n;
	struct soap soap;
	const char srmfunc[] = "AbortFiles";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof(req));

	req.requestToken = (char *) reqtoken;

	/* NOTE: only one SURL in the array */
	if ((req.arrayOfSURLs =
				soap_malloc (&soap, sizeof(struct srm2__ArrayOfAnyURI))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	if ((ret = soap_call_srm2__srmAbortFiles (&soap, srm_endpoint, "AbortFiles", &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (soap.fault != NULL && soap.fault->faultstring != NULL)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
					gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmAbortFilesResponse->returnStatus;
	repfs = rep.srmAbortFilesResponse->arrayOfFileStatuses;

	if (!repfs || repfs->__sizestatusArray < 1 || !repfs->statusArray) {
		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation && reqstatp->explanation[0])
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>",
					gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
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
		if (!repfs->statusArray[i])
			continue;
		memset (*statuses + i, 0, sizeof (struct srmv2_pinfilestatus));
		if (repfs->statusArray[i]->surl)
			(*statuses)[i].surl = strdup (repfs->statusArray[i]->surl);
		if (repfs->statusArray[i]->status) {
			(*statuses)[i].status = statuscode2errno (repfs->statusArray[i]->status->statusCode);
			if (repfs->statusArray[i]->status->explanation && repfs->statusArray[i]->status->explanation[0])
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->statusArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg (repfs->statusArray[i]->status->statusCode));
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);
}


srmv2_access (int nbfiles, const char **surls, const char *srm_endpoint, int amode,
		struct srmv2_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno = 0;
	int ret;
	struct srm2__srmCheckPermissionResponse_ rep;
	struct srm2__ArrayOfTSURLPermissionReturn *repfs;
	struct srm2__srmCheckPermissionRequest req;
	struct srm2__TReturnStatus *reqstatp;
	int i, n;
	struct soap soap;
	const char srmfunc[] = "CheckPermission";

	soap_init (&soap);
	soap.namespaces = namespaces_srmv2;

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout;
	soap.recv_timeout = timeout;
	soap.connect_timeout = timeout;

	memset (&req, 0, sizeof(req));

	/* NOTE: only one SURL in the array */
	if ((req.arrayOfSURLs =
				soap_malloc (&soap, sizeof(struct srm2__ArrayOfAnyURI))) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "soap_malloc error");
		errno = ENOMEM;
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}

	req.arrayOfSURLs->__sizeurlArray = nbfiles;
	req.arrayOfSURLs->urlArray = (char **) surls;

	if ((ret = soap_call_srm2__srmCheckPermission (&soap, srm_endpoint, srmfunc, &req, &rep))) {
		char errmsg[ERRMSG_LEN];
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
					gfal_remote_type, srmfunc, srm_endpoint, soap.fault->faultstring);
		else if (soap.error == SOAP_EOF)
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: Connection fails or timeout",
					gfal_remote_type, srmfunc, srm_endpoint);

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = ECOMM;
		return (-1);
	}

	reqstatp = rep.srmCheckPermissionResponse->returnStatus;
	repfs = rep.srmCheckPermissionResponse->arrayOfPermissions;
	n = repfs->__sizesurlPermissionArray;

	if (!repfs || n < 1 || !repfs->surlPermissionArray) {
		char errmsg[ERRMSG_LEN];
		if (reqstatp->statusCode != SRM_USCORESUCCESS &&
				reqstatp->statusCode != SRM_USCOREPARTIAL_USCORESUCCESS) {

			sav_errno = statuscode2errno (reqstatp->statusCode);
			if (reqstatp->explanation)
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint, reqstatp->explanation);
			else
				snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: %s",
						gfal_remote_type, srmfunc, srm_endpoint,
						statuscode2errmsg (reqstatp->statusCode));
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "[%s][%s] %s: <empty response>",
					gfal_remote_type, srmfunc, srm_endpoint);
			sav_errno = ECOMM;
		}

		gfal_errmsg(errbuf, errbufsz, errmsg);
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}

	if ((*statuses = (struct srmv2_filestatus *) calloc (n, sizeof(struct srmv2_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < n; i++) {
		if (!repfs->surlPermissionArray[i])
			continue;
		memset (*statuses + i, 0, sizeof (struct srmv2_pinfilestatus));
		if (repfs->surlPermissionArray[i]->surl)
			(*statuses)[i].surl = strdup (repfs->surlPermissionArray[i]->surl);
		if (repfs->surlPermissionArray[i]->status) {
			(*statuses)[i].status = statuscode2errno (repfs->surlPermissionArray[i]->status->statusCode);
			if (repfs->surlPermissionArray[i]->status->explanation && repfs->surlPermissionArray[i]->status->explanation[0])
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, repfs->surlPermissionArray[i]->status->explanation);
			else if (reqstatp->explanation != NULL && reqstatp->explanation[0] && strncasecmp (reqstatp->explanation, "failed for all", 14))
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc, reqstatp->explanation);
			else
				asprintf (&((*statuses)[i].explanation), "[%s][%s] %s", gfal_remote_type, srmfunc,
						statuscode2errmsg (repfs->surlPermissionArray[i]->status->statusCode));
		} else
			(*statuses)[i].status = ENOMEM;
        if ((*statuses)[i].status == 0) {
			enum srm2__TPermissionMode perm = *(repfs->surlPermissionArray[i]->permission);

			if ((amode == R_OK && (perm == NONE || perm == X || perm == W || perm == WX)) ||
					(amode == W_OK && (perm == NONE || perm == X || perm == R || perm == RX)) ||
					(amode == X_OK && (perm == NONE || perm == W || perm == R || perm == RW)) ||
					(amode == (R_OK|W_OK) && perm != RW && perm != RWX) ||
					(amode == (R_OK|X_OK) && perm != RX && perm != RWX) ||
					(amode == (W_OK|X_OK) && perm != WX && perm != RWX) ||
					(amode == (R_OK|W_OK|X_OK) && perm != RWX))
				(*statuses)[i].status = EACCES;
		}
	}

	soap_end (&soap);
	soap_done (&soap);
	return (n);

}

static int
statuscode2errno (int statuscode)
{
	switch (statuscode) {
		case SRM_USCOREINVALID_USCOREPATH:
			return (ENOENT);
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

static int
filestatus2returncode (int filestatus)
{
	switch (filestatus) {
		case SRM_USCOREREQUEST_USCOREQUEUED:
		case SRM_USCOREREQUEST_USCOREINPROGRESS:
			return (0);
		case SRM_USCORESUCCESS:
		case SRM_USCOREFILE_USCOREPINNED:
			return (1);
		default:
			return (-1);
	}
}

static const char *
statuscode2errmsg (int statuscode)
{
	return (srmv2_errmsg[statuscode]);
}
