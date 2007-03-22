/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: srm_ifce.c,v $ $Revision: 1.27 $ $Date: 2007/03/22 15:36:28 $ CERN Jean-Philippe Baud
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

static int
srm_init (struct soap *soap, const char *surl, char *srm_endpoint,
	  int srm_endpointsz, char *errbuf, int errbufsz)
{
	int flags;
	char *sfn;

	if (parsesurl (surl, srm_endpoint, srm_endpointsz, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	soap_init (soap);
	

#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (soap, client_cgsi_plugin, &flags);
#endif
	return (0);
}

srm_deletesurl (const char *surl, char *errbuf, int errbufsz, int timeout)
{
	char srm_endpoint[256];
	char *sfn;

	if (parsesurl (surl, srm_endpoint, 256, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	return (srm_deletesurle (surl, srm_endpoint, errbuf, errbufsz, timeout));
}

srm_deletesurle (const char *surl, const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	struct ns5__advisoryDeleteResponse out;
	int ret;
	int sav_errno;
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

	surlarray.__ptr = (char **)&surl;
	surlarray.__size = 1;

	if (ret = soap_call_ns5__advisoryDelete (&soap, srm_endpoint,
	    "advisoryDelete", &surlarray, &out)) {
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
			sav_errno = ECOMM;
		}
		else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			sav_errno = errorstring2errno (soap.fault->faultstring);
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

srm_get (int nbfiles, char **surls, int nbprotocols, char **protocols,
	int *reqid, char **token, struct srm_filestatus **filestatuses, int timeout)
{
	char srm_endpoint[256];
	char *sfn;

	if (parsesurl (surls[0], srm_endpoint, 256, &sfn, NULL, 0) < 0)
		return (-1);

	return (srm_getxe (nbfiles, surls, nbprotocols, protocols, reqid, token,
			  filestatuses, srm_endpoint, NULL, 0, timeout));
}

srm_getx (int nbfiles, char **surls, int nbprotocols, char **protocols,
	int *reqid, char **token, struct srm_filestatus **filestatuses,
	char *errbuf, int errbufsz, int timeout)
{
	char srm_endpoint[256];
	char *sfn;

	if (parsesurl (surls[0], srm_endpoint, 256, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	return (srm_getxe (nbfiles, surls, nbprotocols, protocols, reqid, token,
			  filestatuses, srm_endpoint, errbuf, errbufsz, timeout));
}

srm_getxe (int nbfiles, char **surls, int nbprotocols, char **protocols,
	int *reqid, char **token, struct srm_filestatus **filestatuses,
	const char *srm_endpoint, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int errflag = 0;
	struct ns1__RequestFileStatus *f;
	struct srm_filestatus *fs;
	int i;
	int n;
	struct ns5__getResponse outg;
	char *p;
	struct ArrayOfstring protoarray;
	struct ns1__RequestStatus *reqstatp;
	int ret;
	int sav_errno;
	struct soap soap;
	struct ArrayOfstring surlarray;

	soap_init (&soap);
	
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;

	surlarray.__ptr = (char **)surls;
	surlarray.__size = nbfiles;
	protoarray.__ptr = protocols;
	protoarray.__size = nbprotocols;

	if (ret = soap_call_ns5__get (&soap, srm_endpoint, "get", &surlarray,
	    &protoarray, &outg)) {
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
			sav_errno = ECOMM;
		}
		else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			sav_errno = errorstring2errno (soap.fault->faultstring);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
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
	if ((*filestatuses = calloc (n, sizeof(struct srm_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}
	fs = *filestatuses;
	for (i = 0; i < n; i++) {
		f = reqstatp->fileStatuses->__ptr[i];
		if (f->SURL && (fs->surl = strdup (f->SURL)) == NULL)
			errflag++;
		if (f->state) {
			if (strcmp (f->state, "Pending") == 0 ||
			    strcmp (f->state, "pending") == 0)
				fs->status = 0;
			else if (strcmp (f->state, "Failed") == 0 ||
			    strcmp (f->state, "failed") == 0)
				fs->status = -1;
			else
				fs->status = 1;
		}
		fs->fileid = f->fileId;
		if (f->TURL && (fs->turl = strdup (f->TURL)) == NULL)
			errflag++;
		fs++;
	}
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srm_getstatus (int nbfiles, char **surls, int reqid, char *token,
	struct srm_filestatus **filestatuses, int timeout)
{
	char srm_endpoint[256];
	char *sfn;

	if (parsesurl (surls[0], srm_endpoint, 256, &sfn, NULL, 0) < 0)
		return (-1);

	return (srm_getstatusxe (nbfiles, surls, reqid, token, filestatuses,
	    srm_endpoint, NULL, 0, timeout));
}

srm_getstatusx (int nbfiles, char **surls, int reqid, char *token,
	struct srm_filestatus **filestatuses, char *errbuf, int errbufsz, int timeout)
{
	char srm_endpoint[256];
	char *sfn;

	if (parsesurl (surls[0], srm_endpoint, 256, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	return (srm_getstatusxe (nbfiles, surls, reqid, token, filestatuses,
	    srm_endpoint, errbuf, errbufsz, timeout));
}

srm_getstatusxe (int nbfiles, char **surls, int reqid, char *token,
	struct srm_filestatus **filestatuses, const char *srm_endpoint,
	char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int errflag = 0;
	struct ns1__RequestFileStatus *f;
	struct srm_filestatus *fs;
	int i;
	int n;
	struct ns5__getRequestStatusResponse outq;
	char *p;
	struct ns1__RequestStatus *reqstatp;
	int ret;
	int sav_errno;
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
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
			sav_errno = ECOMM;
		}
		else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			sav_errno = errorstring2errno (soap.fault->faultstring);
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}
	reqstatp = outq._Result;
	if ((reqstatp = outq._Result) == NULL || reqstatp->fileStatuses == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = EINVAL;
		return (-1);
	}
	n = reqstatp->fileStatuses->__size;
	if ((*filestatuses = calloc (n, sizeof(struct srm_filestatus))) == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOMEM;
		return (-1);
	}
	fs = *filestatuses;
	for (i = 0; i < n; i++) {
		f = reqstatp->fileStatuses->__ptr[i];
		if (f->SURL && (fs->surl = strdup (f->SURL)) == NULL)
			errflag++;
		if (f->state) {
			if (strcmp (f->state, "Pending") == 0 ||
			    strcmp (f->state, "pending") == 0)
				fs->status = 0;
			else if (strcmp (f->state, "Failed") == 0 ||
			    strcmp (f->state, "failed") == 0)
				fs->status = -1;
			else
				fs->status = 1;
		}
		fs->fileid = f->fileId;
		if (f->TURL && (fs->turl = strdup (f->TURL)) == NULL)
			errflag++;
		fs++;
	}
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

srm_turlsfromsurls (int nbfiles, const char **surls, LONG64 *filesizes, char **protocols, int oflag, int *reqid, 
		    int **fileids, char **token, char ***turls, char *errbuf, int errbufsz, int timeout)
{
	char srm_endpoint[256];
	char *sfn;

	if (parsesurl (surls[0], srm_endpoint, 256, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	return (srm_turlsfromsurlse (nbfiles, surls, srm_endpoint, filesizes, protocols, oflag, reqid,
		    fileids, token, turls, errbuf, errbufsz, timeout));
}

srm_turlsfromsurlse (int nbfiles, const char **surls, const char *srm_endpoint, LONG64 *filesizes, char **protocols,
		    int oflag, int *reqid, int **fileids, char **token, char ***turls, char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int *f;
	int i;
	int n;
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
	int sav_errno;
	struct ArrayOflong sizearray;
	struct soap soap;
	struct ArrayOfstring srcarray;
	struct ArrayOfstring surlarray;
	char **t;
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
			if (soap.error == SOAP_EOF) {
				gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
				sav_errno = ECOMM;
			}
			else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
				sav_errno = errorstring2errno (soap.fault->faultstring);
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
			if (soap.error == SOAP_EOF) {
				gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
				sav_errno = ECOMM;
			}
			else if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
				sav_errno = errorstring2errno (soap.fault->faultstring);
			}
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
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

	if (timeout > 0)
		endtime = time(NULL) + timeout;
	
	while (strcmp (reqstatp->state, "pending") == 0 ||
	    strcmp (reqstatp->state, "Pending") == 0) {
		sleep ((r++ == 0) ? 1 : (reqstatp->retryDeltaTime > 0) ?
		    reqstatp->retryDeltaTime : DEFPOLLINT);
		if (ret = soap_call_ns5__getRequestStatus (&soap, srm_endpoint,
		    "getRequestStatus", *reqid, &outq)) {
			if (soap.error == SOAP_EOF) {
				gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
				sav_errno = ECOMM;
			}
			if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
				gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
				sav_errno = errorstring2errno (soap.fault->faultstring);
			}
			soap_end (&soap);
			soap_done (&soap);
			errno = sav_errno;
			return (-1);
		}
		reqstatp = outq._Result;

		/* check if user-supplied timeout has passed */
		if (timeout > 0 && time(NULL) > endtime) {
			for (i = 0; i < nbfiles-1; i++) {
				struct ns5__setFileStatusResponse out;
				if (ret = soap_call_ns5__setFileStatus (&soap, srm_endpoint,
							"setFileStatus", *reqid, *fileids[i], "Done", &out)) {
					if (soap.error == SOAP_EOF) {
						gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
						sav_errno = ECOMM;
					}
					if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
						gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
						sav_errno = errorstring2errno (soap.fault->faultstring);
					}
					soap_end (&soap);
					soap_done (&soap);
					errno = sav_errno;
					return (-1);
				}
			}
			gfal_errmsg(errbuf, errbufsz, "Waiting for Pending SRM request timed out\n");
			soap_end(&soap);
			soap_done(&soap);
			return (-1);
		}

	}
	if (strcmp (reqstatp->state, "failed") == 0 ||
	    strcmp (reqstatp->state, "Failed") == 0) {
		if (reqstatp->errorMessage) {
			gfal_errmsg(errbuf, errbufsz, reqstatp->errorMessage);
			sav_errno = errorstring2errno (reqstatp->errorMessage);
		} else  {
			gfal_errmsg(errbuf, errbufsz, "SRM request failed, but no errorMessage supplied");
			sav_errno = ECOMM;
		}
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
		f[i] = (reqstatp->fileStatuses->__ptr[i])->fileId;
		if (strcmp ((reqstatp->fileStatuses->__ptr[i])->state, "ready") &&
		    strcmp ((reqstatp->fileStatuses->__ptr[i])->state, "Ready"))
			t[i] = NULL;
		else
			t[i] = strdup ((reqstatp->fileStatuses->__ptr[i])->TURL);
	}
	*fileids = f;
	*token = NULL;
	*turls = t;
	soap_end (&soap);
	soap_done (&soap);
	return (n);
}

char *
srm_turlfromsurl (const char *surl, char **protocols, int oflag, int *reqid, int *fileid, char **token, 
		  char *errbuf, int errbufsz, int timeout)
{
	int *fileids;
	char *p;
	char **turls;
	LONG64 zero = 0;

	if (srm_turlsfromsurls (1, &surl, &zero, protocols, oflag,
	    reqid, &fileids, token, &turls, errbuf, errbufsz, timeout) <= 0)
		return (NULL);
	*fileid = fileids[0];
	p = turls[0];
	free (fileids);
	free (turls);
	return (p);
}

srm_getfilemd (const char *surl, struct stat64 *statbuf, char *errbuf, int errbufsz, int timeout)
{
	char srm_endpoint[256];
	char *sfn;

	if (parsesurl (surl, srm_endpoint, 256, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	return (srm_getfilemde (surl, srm_endpoint, statbuf, errbuf, errbufsz, timeout));
}

srm_getfilemde (const char *surl, const char *srm_endpoint, struct stat64 *statbuf,
		char *errbuf, int errbufsz, int timeout)
{
	int flags;
	struct group *gr;
	struct ns5__getFileMetaDataResponse out;
	struct passwd *pw;
	int ret;
	int sav_errno;
	struct soap soap;
	struct ArrayOfstring surlarray;

	soap_init (&soap);
	
#ifdef GFAL_SECURE
	flags = CGSI_OPT_DISABLE_NAME_CHECK;
	soap_register_plugin_arg (&soap, client_cgsi_plugin, &flags);
#endif

	soap.send_timeout = timeout ;
	soap.recv_timeout = timeout ;


	/* issue "getFileMetaData" request */

	surlarray.__ptr = (char **)&surl;
	surlarray.__size = 1;

	if ((ret = soap_call_ns5__getFileMetaData (&soap, srm_endpoint,
	    "getFileMetaData", &surlarray, &out))) {
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
			sav_errno = ECOMM;
		}
		else if (ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			sav_errno = errorstring2errno (soap.fault->faultstring);
		} else {
			sav_errno = ECOMM;
			gfal_errmsg(errbuf, errbufsz, "Error calling getFileMetaData on SRM, but no error message returned");
		}
		soap_end (&soap);
		soap_done (&soap);
		errno = sav_errno;
		return (-1);
	}
	if (out._Result->__size == 0 || out._Result->__ptr[0]->SURL == NULL) {
		soap_end (&soap);
		soap_done (&soap);
		errno = ENOENT;
		return (-1);
	}
	memset (statbuf, 0, sizeof(struct stat64));
	statbuf->st_mode = out._Result->__ptr[0]->permMode;
	if ((statbuf->st_mode & S_IFMT) == 0)
		statbuf->st_mode |= S_IFREG;
	statbuf->st_nlink = 1;
	if (out._Result->__ptr[0]->owner && (pw = getpwnam (out._Result->__ptr[0]->owner)))
		statbuf->st_uid = pw->pw_uid;
	else
		statbuf->st_uid = 2;
	if (out._Result->__ptr[0]->group && (gr = getgrnam (out._Result->__ptr[0]->group)))
		statbuf->st_gid = gr->gr_gid;
	else
		statbuf->st_gid = 2;
	statbuf->st_size = out._Result->__ptr[0]->size;
	soap_end (&soap);
	soap_done (&soap);
	return (0);
}

srm_set_xfer_done (const char *surl, int reqid, int fileid, char *token, int oflag, char *errbuf, int errbufsz, int timeout)
{
	char srm_endpoint[256];
	char *sfn;

	if (parsesurl (surl, srm_endpoint, 256, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	return (srm_set_xfer_donee (surl, srm_endpoint, reqid, fileid, token, oflag, errbuf, errbufsz, timeout));
}

srm_set_xfer_donee (const char *surl, const char *srm_endpoint, int reqid, int fileid, char *token, int oflag,
		    char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno;
	struct ns5__setFileStatusResponse out;
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
	    "setFileStatus", reqid, fileid, "Done", &out)) {
		if (soap.error == SOAP_EOF) {
	  		gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
			sav_errno = ECOMM;
	       	}
		if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			sav_errno = errorstring2errno (soap.fault->faultstring);
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

srm_set_xfer_running (const char *surl, int reqid, int fileid, char *token, char *errbuf, int errbufsz, int timeout)
{
	char srm_endpoint[256];
	char *sfn;

	if (parsesurl (surl, srm_endpoint, 256, &sfn, errbuf, errbufsz) < 0)
		return (-1);

	return (srm_set_xfer_runninge (surl, srm_endpoint, reqid, fileid, token, errbuf, errbufsz, timeout));
}

srm_set_xfer_runninge (const char *surl, const char *srm_endpoint, int reqid, int fileid, char *token,
			char *errbuf, int errbufsz, int timeout)
{
	int flags;
	int sav_errno;
	struct ns5__setFileStatusResponse out;
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
	    "setFileStatus", reqid, fileid, "Running", &out)) {
		if (soap.error == SOAP_EOF) {
			gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");
			sav_errno = ECOMM;
		}
		if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT) {
			gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);
			sav_errno = errorstring2errno (soap.fault->faultstring);
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

int
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
	else if (strstr (errstr, "nvalid arg"))
		return (EINVAL);
	else if (strstr (errstr, "rotocol"))
		return (EPROTONOSUPPORT);
	else if (strstr (errstr, "o space left on device"))
		return (ENOSPC);
	else
		return (ECOMM);
}
