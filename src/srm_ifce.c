/*
 * Copyright (C) 2003-2004 by CERN
 */

/*
 * @(#)$RCSfile: srm_ifce.c,v $ $Revision: 1.4 $ $Date: 2004/04/29 11:31:11 $ CERN Jean-Philippe Baud
 */

#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include "gfal_api.h"
#include "srmH.h"
#include "soapSRMServerV1.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#define DEFPOLLINT 10

deletesurl (const char *surl)
{
	int flags;
	struct tns__advisoryDeleteResponse out;
	char *sfn;
	struct soap soap;
	char *srm_endpoint;
	struct ArrayOfstring surlarray;

	if (parsesurl (surl, &srm_endpoint, &sfn) < 0) {
		perror ("parsesurl");
		exit (1);
	}

	soap_init (&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	/* issue "advisoryDelete" request */

	surlarray.__ptr = (char **)&surl;
	surlarray.__size = 1;
	surlarray.__offset = 0;

	if (soap_call_tns__advisoryDelete (&soap, srm_endpoint,
	    "advisoryDelete", &surlarray, &out)) {
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
turlfromsurl (const char *surl, char **protocols, int oflag, int *reqid, int *fileid)
{
	int flags;
	int nbproto = 0;
	struct tns__getResponse outg;
	struct tns__putResponse outp;
	struct tns__getRequestStatusResponse outq;
	char *p;
	struct ArrayOfboolean permarray;
	struct ArrayOfstring protoarray;
	int r = 0;
	struct ns11__RequestStatus *reqstatp;
	int sav_errno;
	xsd__boolean setperm = true_;
	char *sfn;
	struct ArrayOflong sizearray;
	struct soap soap;
	struct ArrayOfstring srcarray;
	char *srm_endpoint;
	struct ArrayOfstring surlarray;
	xsd__long zero = 0;

	if (parsesurl (surl, &srm_endpoint, &sfn) < 0)
		return (NULL);

	while (*protocols[nbproto]) nbproto++;

	soap_init(&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	/* issue "get" or the "put" request */

	surlarray.__ptr = (char **)&surl;
	surlarray.__size = 1;
	surlarray.__offset = 0;
	protoarray.__ptr = protocols;
	protoarray.__size = nbproto;
	protoarray.__offset = 0;

	if ((oflag & O_ACCMODE) == 0) {
		if (soap_call_tns__get (&soap, srm_endpoint, "get", &surlarray,
		    &protoarray, &outg)) {
			soap_print_fault (&soap, stderr);
			soap_end (&soap);
			soap_done (&soap);
			return (NULL);
		}
		reqstatp = outg._Result;
	} else {
		srcarray.__ptr = (char **)&surl;
		srcarray.__size = 1;
		srcarray.__offset = 0;
		sizearray.__ptr = &zero;
		sizearray.__size = 1;
		sizearray.__offset = 0;
		permarray.__ptr = &setperm;
		permarray.__size = 1;
		permarray.__offset = 0;
		if (soap_call_tns__put (&soap, srm_endpoint, "put", &srcarray,
		    &surlarray, &sizearray, &permarray, &protoarray, &outp)) {
			soap_print_fault (&soap, stderr);
			soap_end (&soap);
			soap_done (&soap);
			return (NULL);
		}
		reqstatp = outp._Result;
	}
	if (reqstatp->fileStatuses == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = EPROTONOSUPPORT;
		return (NULL);
	}
	*reqid = reqstatp->requestId;

	/* wait for file "ready" */

	while ((strcmp (reqstatp->state, "pending") == 0 ||
	    strcmp (reqstatp->state, "Pending") == 0) &&
	    (strcmp (reqstatp->fileStatuses->__ptr->state, "pending") == 0 ||
	    strcmp (reqstatp->fileStatuses->__ptr->state, "Pending") == 0)) {
		sleep ((r++ == 0) ? 1 : (reqstatp->retryDeltaTime > 0) ?
		    reqstatp->retryDeltaTime : DEFPOLLINT);
		if (soap_call_tns__getRequestStatus (&soap, srm_endpoint,
		    "getRequestStatus", *reqid, &outq)) {
			soap_print_fault (&soap, stderr);
			soap_end (&soap);
			soap_done (&soap);
			return (NULL);
		}
		reqstatp = outq._Result;
	}
	if (strcmp (reqstatp->fileStatuses->__ptr->state, "ready") &&
	    strcmp (reqstatp->fileStatuses->__ptr->state, "Ready")) {
		if (strstr (reqstatp->errorMessage, "does not exist") ||
		    strstr (reqstatp->errorMessage, "GetStorageInfoFailed"))
			sav_errno = ENOENT;
		else if (strstr (reqstatp->errorMessage, "nvalid arg"))
			sav_errno = EINVAL;
		else
			sav_errno = ECOMM;
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (NULL);
	}
	*fileid = reqstatp->fileStatuses->__ptr->fileId;
	p = strdup (reqstatp->fileStatuses->__ptr->TURL);
	soap_end (&soap);
	soap_done (&soap);
	return (p);
}

getfilemd (const char *surl, struct stat64 *statbuf)
{
	int flags;
	struct group *gr;
	struct tns__getFileMetaDataResponse out;
	struct passwd *pw;
	int ret;
	int sav_errno;
	char *sfn;
	struct soap soap;
	char *srm_endpoint;
	struct ArrayOfstring surlarray;

	if (parsesurl (surl, &srm_endpoint, &sfn) < 0)
		return (-1);

	soap_init (&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	/* issue "getFileMetaData" request */

	surlarray.__ptr = (char **)&surl;
	surlarray.__size = 1;
	surlarray.__offset = 0;

	if ((ret = soap_call_tns__getFileMetaData (&soap, srm_endpoint,
	    "getFileMetaData", &surlarray, &out))) {
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			if (strstr (soap.fault->faultstring, "No such file"))
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
	if (out._Result->__size == 0 || out._Result->__ptr->SURL == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOENT;
		return (-1);
	}
	memset (statbuf, 0, sizeof(struct stat64));
	statbuf->st_mode = S_IFREG | out._Result->__ptr->permMode;
	statbuf->st_nlink = 1;
	if (out._Result->__ptr->owner && (pw = getpwnam (out._Result->__ptr->owner)))
		statbuf->st_uid = pw->pw_uid;
	else
		statbuf->st_uid = 2;
	if (out._Result->__ptr->group && (gr = getgrnam (out._Result->__ptr->group)))
		statbuf->st_gid = gr->gr_gid;
	else
		statbuf->st_gid = 2;
	statbuf->st_size = out._Result->__ptr->size;
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

set_xfer_done (char *surl, int reqid, int fileid, int oflag)
{
	int flags;
	struct tns__setFileStatusResponse out;
	char *sfn;
	struct soap soap;
	char *srm_endpoint;

	if (parsesurl (surl, &srm_endpoint, &sfn) < 0)
		return (-1);

	soap_init (&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif
	if (soap_call_tns__setFileStatus (&soap, srm_endpoint,
	    "setFileStatus", reqid, fileid, "Done", &out)) {
		soap_print_fault (&soap, stderr);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

set_xfer_running (char *surl, int reqid, int fileid)
{
	int flags;
	struct tns__setFileStatusResponse out;
	char *sfn;
	struct soap soap;
	char *srm_endpoint;

	if (parsesurl (surl, &srm_endpoint, &sfn) < 0)
		return (-1);

	soap_init (&soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif
	if (soap_call_tns__setFileStatus (&soap, srm_endpoint,
	    "setFileStatus", reqid, fileid, "Running", &out)) {
		soap_print_fault (&soap, stderr);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}
