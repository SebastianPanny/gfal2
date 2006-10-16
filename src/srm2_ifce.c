/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: srm2_ifce.c,v $ $Revision: 1.2 $ $Date: 2006/10/16 07:27:24 $ CERN Jean-Philippe Baud
 */

/*tests conducted with memory checker usually*/

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "srm2H.h"

//#include "srm2Stub.h"
//#include "srm2H.h"
#if _DUMA
#include "duma.h"
#endif



#include "srmSoapBinding+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "srm2C.c"
#include "srm2Client.c"

//#include "gfal_api.h"
#include "srm2_soap_conversion.h"


static int
srm2_init (struct soap *soap, const char *surl, char *srm_endpoint,
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



#define INTERNAL_SOAP_BODY_DECL	\
	int ret;\
	struct soap soap;\
	char srm_endpoint[256];
	
#define DEF_INOUT(_typeName)						\
	struct srm2__##_typeName##Response_ _res;	\
	struct srm2__##_typeName##Request* _req;
	
	
#define STANDARD_SURL_CHECK_INIT(_surl)	\
		if (srm2_init (&soap, _surl, srm_endpoint, sizeof(srm_endpoint),errbuf, errbufsz) < 0)	\
			return (-1);	\
	        soap.send_timeout = timeout ;	\
	        soap.recv_timeout = timeout ;	

	
#define STANDARD_CLEANUP_RETURN(_n)	\
	soap_end (&soap);	\
	soap_done (&soap);	\
	return (_n);
	
#define HANDLE_SOAP_ERROR	\
	if (soap.error == SOAP_EOF) {	\
	    gfal_errmsg(errbuf, errbufsz, "connection fails or timeout");	\
		STANDARD_CLEANUP_RETURN(-1);	\
    }	\
    if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)	\
            gfal_errmsg(errbuf, errbufsz, soap.fault->faultstring);	\
	STANDARD_CLEANUP_RETURN(-1);
            
			
/* 
 * in:
 * TRequestToken requestToken 
 * TUserID	authId
 * TSURL[] sitesurls
 * 
 * out:
 * TReturnStatus returnStatus
 * TSURLReturnStatus[] arrayOfFileStatus 
 *  
 */
 
 
int srm21_RemoveFiles(
	TRequestToken reqTok, 
	TUserID userId, 
	TSURL* siteSurls,
	TReturnStatus retStatus,
	ArrayOfTSURLReturnStatus arrayOfFileStatus,
	char *errbuf, 
	int errbufsz,
	int timeout
)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	STANDARD_SURL_CHECK_INIT(siteSurls[0]);
	//nonstandard decls
	struct srm2__srmRemoveFilesResponse_ response;
	struct srm2__srmRemoveFilesRequest request;
	//prepare input data
	//call
	if( ret = soap_call_srm2__srmRemoveFiles(&soap,srm_endpoint, "srmRemoveFiles", &request, &response))
		 {
		 	HANDLE_SOAP_ERROR;
		 };


	//LOGIC

	// cleanup and return
	STANDARD_CLEANUP_RETURN (0);	
}

//int srm2_PrepareToPut

// srmReleaseFiles
// srmPrepareToGet
// srmPrepareToPut
	//
// srmPutDone
// srmAbortRequest
// srmAbortFiles
// srmStatusOfGetRequest
// srmStatusOfPutRequest
// srmGetRequestSummary
// srmGetRequestID
//
//change it so ptr to err struct is returned on error 
// all manipulation methods are defined for it like gen_ and freeType_.

#define CALL_SOAP(_call) soap_call_srm2__##_call(&soap,srm_endpoint, #_call, _req, &_res)

//#define GFAL_WRAP_DEC(_typeName)	\
//	int srm21_##_typeName(_typeName##Request* elem,_typeName##Response* res, int timeout, char* errbuf, int errbufsz)

//debugging exemplary placeholder
GFAL_WRAP_DEC(srmCheckPermission)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmCheckPermission);
	STANDARD_SURL_CHECK_INIT(get_srmCheckPermissionRequest_TSurl(elem));
	//do translation	
	_req = conv2soap_srmCheckPermissionRequest(&soap,elem);
	//execute
	if( ret = CALL_SOAP(srmCheckPermission))
		 {
		 	HANDLE_SOAP_ERROR;
		 };	
	//return converted		 
	res = conv2gfal_srmCheckPermissionResponse(_res.srmCheckPermissionResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}


#define GFAL_SOAP_WRAP(_callType)	\
GFAL_WRAP_DEC(_callType)				\
{									\
	INTERNAL_SOAP_BODY_DECL;		\
	DEF_INOUT(_callType);			\
	STANDARD_SURL_CHECK_INIT(get_##_callType##Request_TSurl(elem));			\
	_req = conv2soap_##_callType##Request(&soap,elem);						\
	if( ret = CALL_SOAP(_callType))											\
		 {																	\
		 	HANDLE_SOAP_ERROR;												\
		 };																	\
	res = conv2gfal_##_callType##Response(_res._callType##Response); 		\
	STANDARD_CLEANUP_RETURN (0);											\
}

GFAL_SOAP_WRAP(srmLs);
GFAL_SOAP_WRAP(srmPrepareToGet);
GFAL_SOAP_WRAP(srmPrepareToPut);
GFAL_SOAP_WRAP(srmStatusOfGetRequest);
GFAL_SOAP_WRAP(srmStatusOfPutRequest);
GFAL_SOAP_WRAP(srmReleaseFiles);
GFAL_WRAP_DEC(srmPutDone);


