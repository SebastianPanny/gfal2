/*
 * Copyright (C) 2003-2004 by CERN
 */

/*
 * @(#)$RCSfile: srm_ifce.c,v $ $Revision: 1.11 $ $Date: 2004/10/21 07:32:59 $ CERN Jean-Philippe Baud
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

static int
srm_init (struct soap *soap, const char *surl, char *srm_endpoint, int srm_endpointsz)
{
	int flags;
	char *sfn;

	if (parsesurl (surl, srm_endpoint, srm_endpointsz, &sfn) < 0)
		return (-1);

	soap_init (soap);
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif
	return (0);
}

srm_deletesurl (const char *surl)
{
	struct tns__advisoryDeleteResponse out;
	struct soap soap;
	char srm_endpoint[256];
	struct ArrayOfstring surlarray;

	if (srm_init (&soap, surl, srm_endpoint, sizeof(srm_endpoint)) < 0)
		return (-1);

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

srm_get (int nbfiles, char **surls, int nbprotocols, char **protocols, int *reqid, char **token, struct srm_filestatus **filestatuses)
{
	int errflag = 0;
	struct ns11__RequestFileStatus *f;
	struct srm_filestatus *fs;
	int i;
	int n;
	struct tns__getResponse outg;
	char *p;
	struct ArrayOfstring protoarray;
	struct ns11__RequestStatus *reqstatp;
	int sav_errno;
	struct soap soap;
	char srm_endpoint[256];
	struct ArrayOfstring surlarray;

	if (srm_init (&soap, surls[0], srm_endpoint, sizeof(srm_endpoint)) < 0)
		return (-1);

	surlarray.__ptr = (char **)surls;
	surlarray.__size = nbfiles;
	surlarray.__offset = 0;
	protoarray.__ptr = protocols;
	protoarray.__size = nbprotocols;
	protoarray.__offset = 0;

	if (soap_call_tns__get (&soap, srm_endpoint, "get", &surlarray,
	    &protoarray, &outg)) {
		soap_print_fault (&soap, stderr);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	reqstatp = outg._Result;
	if (reqstatp->fileStatuses == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	*reqid = reqstatp->requestId;
	if (token)
		*token = NULL;
	n = reqstatp->fileStatuses->__size;
	if ((*filestatuses = malloc (n * sizeof(struct srm_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}
	f = reqstatp->fileStatuses->__ptr;
	fs = *filestatuses;
	for (i = 0; i < n; i++) {
		if ((f+i)->SURL && (fs->surl = strdup ((f+i)->SURL)) == NULL)
			errflag++;
		if ((f+i)->state) {
			if (strcmp ((f+i)->state, "Pending") == 0 ||
			    strcmp ((f+i)->state, "pending") == 0)
				fs->status = 0;
			else if (strcmp ((f+i)->state, "Failed") == 0 ||
			    strcmp ((f+i)->state, "failed") == 0)
				fs->status = -1;
			else
				fs->status = 1;
		}
		fs->fileid = (f+i)->fileId;
		if ((f+i)->TURL && (fs->turl = strdup ((f+i)->TURL)) == NULL)
			errflag++;
		fs++;
	}
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srm_getstatus (int nbfiles, char **surls, int reqid, char *token, struct srm_filestatus **filestatuses)
{
	int errflag = 0;
	struct ns11__RequestFileStatus *f;
	struct srm_filestatus *fs;
	int i;
	int n;
	struct tns__getRequestStatusResponse outq;
	char *p;
	struct ns11__RequestStatus *reqstatp;
	int sav_errno;
	struct soap soap;
	char srm_endpoint[256];
	struct ArrayOfstring surlarray;

	if (srm_init (&soap, surls[0], srm_endpoint, sizeof(srm_endpoint)) < 0)
		return (-1);

	if (soap_call_tns__getRequestStatus (&soap, srm_endpoint,
	    "getRequestStatus", reqid, &outq)) {
		soap_print_fault (&soap, stderr);
		soap_end (&soap);
		soap_done (&soap);
		return (-1);
	}
	reqstatp = outq._Result;
	n = reqstatp->fileStatuses->__size;
	if ((*filestatuses = malloc (n * sizeof(struct srm_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}
	f = reqstatp->fileStatuses->__ptr;
	fs = *filestatuses;
	for (i = 0; i < n; i++) {
		if ((f+i)->SURL && (fs->surl = strdup ((f+i)->SURL)) == NULL)
			errflag++;
		if ((f+i)->state) {
			if (strcmp ((f+i)->state, "Pending") == 0 ||
			    strcmp ((f+i)->state, "pending") == 0)
				fs->status = 0;
			else if (strcmp ((f+i)->state, "Failed") == 0 ||
			    strcmp ((f+i)->state, "failed") == 0)
				fs->status = -1;
			else
				fs->status = 1;
		}
		fs->fileid = (f+i)->fileId;
		if ((f+i)->TURL && (fs->turl = strdup ((f+i)->TURL)) == NULL)
			errflag++;
		fs++;
	}
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srm_turlsfromsurls (int nbfiles, const char **surls, xsd__long *filesizes, char **protocols, int oflag, int *reqid, int **fileids, char **token, char ***turls)
{
	int *f;
	int i;
	int n;
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
	struct ArrayOflong sizearray;
	struct soap soap;
	struct ArrayOfstring srcarray;
	char srm_endpoint[256];
	struct ArrayOfstring surlarray;
	char **t;

	if (srm_init (&soap, surls[0], srm_endpoint, sizeof(srm_endpoint)) < 0)
		return (-1);

	while (*protocols[nbproto]) nbproto++;

	/* issue "get" or the "put" request */

	surlarray.__ptr = (char **)surls;
	surlarray.__size = nbfiles;
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
			return (-1);
		}
		reqstatp = outg._Result;
	} else {
		srcarray.__ptr = (char **)surls;
		srcarray.__size = nbfiles;
		srcarray.__offset = 0;
		sizearray.__ptr = filesizes;
		sizearray.__size = nbfiles;
		sizearray.__offset = 0;
		if ((permarray.__ptr =
		    soap_malloc (&soap, nbfiles * sizeof(xsd__boolean))) == NULL) {
			soap_end (&soap);
			soap_done (&soap);
			errno = ENOMEM;
			return (-1);
		}
		for (i = 0; i< nbfiles; i++)
			permarray.__ptr[i] = true_;
		permarray.__size = nbfiles;
		permarray.__offset = 0;
		if (soap_call_tns__put (&soap, srm_endpoint, "put", &srcarray,
		    &surlarray, &sizearray, &permarray, &protoarray, &outp)) {
			soap_print_fault (&soap, stderr);
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		reqstatp = outp._Result;
	}
	if (reqstatp->fileStatuses == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	*reqid = reqstatp->requestId;

	/* wait for file "ready" */

	while (strcmp (reqstatp->state, "pending") == 0 ||
	    strcmp (reqstatp->state, "Pending") == 0) {
		sleep ((r++ == 0) ? 1 : (reqstatp->retryDeltaTime > 0) ?
		    reqstatp->retryDeltaTime : DEFPOLLINT);
		if (soap_call_tns__getRequestStatus (&soap, srm_endpoint,
		    "getRequestStatus", *reqid, &outq)) {
			soap_print_fault (&soap, stderr);
			soap_end (&soap);
			soap_done (&soap);
			return (-1);
		}
		reqstatp = outq._Result;
	}
	if (strcmp (reqstatp->state, "failed") == 0 ||
	    strcmp (reqstatp->state, "Failed") == 0) {
		if (reqstatp->errorMessage) {
			if (strstr (reqstatp->errorMessage, "ile exists"))
				sav_errno = EEXIST;
			else if (strstr (reqstatp->errorMessage, "does not exist") ||
			    strstr (reqstatp->errorMessage, "GetStorageInfoFailed"))
				sav_errno = ENOENT;
			else if (strstr (reqstatp->errorMessage, "nvalid arg"))
				sav_errno = EINVAL;
			else if (strstr (reqstatp->errorMessage, "protocol"))
				sav_errno = EPROTONOSUPPORT;
			else
				sav_errno = ECOMM;
		} else
			sav_errno = ECOMM;
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}
	n = reqstatp->fileStatuses->__size;
	if ((f = malloc (n * sizeof(int))) == NULL ||
	    (t = malloc (n * sizeof(char *))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}
	for (i = 0; i < n; i++) {
		f[i] = (reqstatp->fileStatuses->__ptr+i)->fileId;
		if (strcmp ((reqstatp->fileStatuses->__ptr+i)->state, "ready") &&
		    strcmp ((reqstatp->fileStatuses->__ptr+i)->state, "Ready"))
			t[i] = NULL;
		else
			t[i] = strdup ((reqstatp->fileStatuses->__ptr+i)->TURL);
	}
	*fileids = f;
	*token = NULL;
	*turls = t;
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

char *
srm_turlfromsurl (const char *surl, char **protocols, int oflag, int *reqid, int *fileid, char **token)
{
	int *fileids;
	char *p;
	char **turls;
	xsd__long zero = 0;

	if (srm_turlsfromsurls (1, &surl, &zero, protocols, oflag,
	    reqid, &fileids, token, &turls) <= 0)
		return (NULL);
	*fileid = fileids[0];
	p = turls[0];
	free (fileids);
	free (turls);
	return (p);
}

srm_getfilemd (const char *surl, struct stat64 *statbuf)
{
	struct group *gr;
	struct tns__getFileMetaDataResponse out;
	struct passwd *pw;
	int ret;
	int sav_errno;
	struct soap soap;
	char srm_endpoint[256];
	struct ArrayOfstring surlarray;

	if (srm_init (&soap, surl, srm_endpoint, sizeof(srm_endpoint)) < 0)
		return (-1);

	/* issue "getFileMetaData" request */

	surlarray.__ptr = (char **)&surl;
	surlarray.__size = 1;
	surlarray.__offset = 0;

	if ((ret = soap_call_tns__getFileMetaData (&soap, srm_endpoint,
	    "getFileMetaData", &surlarray, &out))) {
		if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			if (strstr (soap.fault->faultstring, "No such file") ||
			    strstr (soap.fault->faultstring, "could not get storage info by path"))
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

srm_set_xfer_done (const char *surl, int reqid, int fileid, char *token, int oflag)
{
	struct tns__setFileStatusResponse out;
	struct soap soap;
	char srm_endpoint[256];

	if (srm_init (&soap, surl, srm_endpoint, sizeof(srm_endpoint)) < 0)
		return (-1);

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

srm_set_xfer_running (const char *surl, int reqid, int fileid, char *token)
{
	struct tns__setFileStatusResponse out;
	struct soap soap;
	char srm_endpoint[256];

	if (srm_init (&soap, surl, srm_endpoint, sizeof(srm_endpoint)) < 0)
		return (-1);

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
