


#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "srm2_2H.h"

#if _DUMA
#include "duma.h"
#endif

#include "srm2_2SoapBinding+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "srm2_2C.c"
#include "srm2_2Client.c"

#include "srm2_2_conversion.h"

#define COPY_CHK(_name)	\
tdata1->_name = _name;

	
#define getSURL_DEF(_n,_ret)		\
TSURL get_##_n##_TSurl(_n* _arg){	\
if(!_arg) return NULL;		\
return (char*)_arg->_ret;			\
};

#define getARRSURL_DEF(_typeName,_arrName)	\
getSURL_DEC(srm22_ArrayOf##_typeName)	\
{						\
if(!_arg || (_arg->_arrName[0])) return NULL;			\
return get_##_typeName##_TSurl(_arg->_arrName[0]);		\
}

#define getSUBSURL_DEF(_typeName,_subType,_subName)					\
getSURL_DEC(_typeName)										\
{															\
if(!_arg || (_arg->_subName)) return NULL;			\
return get_##_subType##_TSurl(_arg->_subName);	\
}         

#define FREE_TYPE(_type,_n)	freeType_##_type(_arg->_n); 
#define FREE_SUB_OBJ(_name) 	if(_arg->_name)free((void*)_arg->_name);		
	
#define END_GFAL_FREE	\
free((void*)_arg);	\
_arg=NULL;	


	
#define GFAL_FREEARRAY_TYPE_DEF(_typeName,_sizeName,_arrName)	\
void freegfalArray_ArrayOf##_typeName(srm22_ArrayOf##_typeName* _elem) 			\
{	int i;													\
if(!_elem) return;										\
for(i=0;i<_elem->_sizeName;i++)							\
    {													\
    if(_elem->_arrName[i]) 								\
    freeType_##_typeName(_elem->_arrName[i]);		\
    };													\
    free((void*)_elem->_arrName);							\
    free((void*)_elem);										\
    _elem = NULL;											\
}        
static int
srm22_init (struct soap *soap, const char *surl, char *srm_endpoint,
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
struct soap soap;


#define DEF_INOUT(_typeName)						\
struct srm22__##_typeName##Response_ _res;	\
struct srm22__##_typeName##Request* _req;


#define STANDARD_SURL_CHECK_INIT(_surl)	\
if (srm22_init (&soap, _surl, srm_endpoint, sizeof(srm_endpoint), ctx->errbuf, GFAL_ERRSIZE) < 0)	\
return (-1);	\
soap.send_timeout = ctx->soap_timeout;	\
soap.recv_timeout = ctx->soap_timeout;	


#define STANDARD_CLEANUP_RETURN(_n)	\
soap_end (&soap);	\
soap_done (&soap);	\
return (_n);

#define HANDLE_SOAP_ERROR	\
if (soap.error == SOAP_EOF) {	\
	gfal_errmsg(&ctx->errbuf, GFAL_ERRSIZE, "connection fails or timeout");	\
	ctx->errnom = soap.error;	\
	STANDARD_CLEANUP_RETURN(-1);	\
	}	\
if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)	\
	gfal_errmsg(&ctx->errbuf, GFAL_ERRSIZE, soap.fault->faultstring);	\
STANDARD_CLEANUP_RETURN(-1);


// all manipulation methods are defined for it like gen_ and freeType_.

#define CALL_SOAP(_call) soap_call_srm22__##_call(&soap,"_call", #_call, _req, &_res)


        
        /*       
        All input types:
        srmReserveSpaceRequest srmStatusOfReserveSpaceRequestRequest srmReleaseSpaceRequest srmUpdateSpaceRequest srmStatusOfUpdateSpaceRequestRequest srmGetSpaceMetaDataRequest srmChangeSpaceForFilesRequest srmStatusOfChangeSpaceForFilesRequestRequest srmExtendFileLifeTimeInSpaceRequest srmPurgeFromSpaceRequest srmGetSpaceTokensRequest srmSetPermissionRequest srmCheckPermissionRequest srmGetPermissionRequest srmMkdirRequest srmRmdirRequest srmRmRequest srmLsRequest srmStatusOfLsRequestRequest srmMvRequest srmPrepareToGetRequest srmStatusOfGetRequestRequest srmBringOnlineRequest srmStatusOfBringOnlineRequestRequest srmPrepareToPutRequest srmStatusOfPutRequestRequest srmCopyRequest srmStatusOfCopyRequestRequest srmReleaseFilesRequest srmPutDoneRequest srmAbortRequestRequest srmAbortFilesRequest srmSuspendRequestRequest srmResumeRequestRequest srmGetRequestSummaryRequest srmExtendFileLifeTimeRequest srmGetRequestTokensRequest srmGetTransferProtocolsRequest srmPingRequest TDirOption TCopyFileRequest ArrayOfTCopyFileRequest TPutFileRequest ArrayOfTPutFileRequest TGetFileRequest ArrayOfTGetFileRequest TGroupPermission ArrayOfTGroupPermission TUserPermission ArrayOfTUserPermission ArrayOfString TTransferParameters TExtraInfo ArrayOfTExtraInfo ArrayOfUnsignedLong TRetentionPolicyInfo 
        All output types:
        srmReserveSpaceResponse srmStatusOfReserveSpaceRequestResponse srmReleaseSpaceResponse srmUpdateSpaceResponse srmStatusOfUpdateSpaceRequestResponse srmGetSpaceMetaDataResponse srmChangeSpaceForFilesResponse srmStatusOfChangeSpaceForFilesRequestResponse srmExtendFileLifeTimeInSpaceResponse srmPurgeFromSpaceResponse srmGetSpaceTokensResponse srmSetPermissionResponse srmCheckPermissionResponse srmGetPermissionResponse srmMkdirResponse srmRmdirResponse srmRmResponse srmLsResponse srmStatusOfLsRequestResponse srmMvResponse srmPrepareToGetResponse srmStatusOfGetRequestResponse srmBringOnlineResponse srmStatusOfBringOnlineRequestResponse srmPrepareToPutResponse srmStatusOfPutRequestResponse srmCopyResponse srmStatusOfCopyRequestResponse srmReleaseFilesResponse srmPutDoneResponse srmAbortRequestResponse srmAbortFilesResponse srmSuspendRequestResponse srmResumeRequestResponse srmGetRequestSummaryResponse srmExtendFileLifeTimeResponse srmGetRequestTokensResponse srmGetTransferProtocolsResponse srmPingResponse TExtraInfo ArrayOfTExtraInfo TSupportedTransferProtocol ArrayOfTSupportedTransferProtocol TRequestTokenReturn ArrayOfTRequestTokenReturn TReturnStatus TRequestSummary ArrayOfTRequestSummary TCopyRequestFileStatus ArrayOfTCopyRequestFileStatus TPutRequestFileStatus ArrayOfTPutRequestFileStatus TBringOnlineRequestFileStatus ArrayOfTBringOnlineRequestFileStatus TGetRequestFileStatus ArrayOfTGetRequestFileStatus struct ArrayOfTMetaDataPathDetail TGroupPermission TUserPermission ArrayOfString TRetentionPolicyInfo TMetaDataPathDetail ArrayOfTMetaDataPathDetail ArrayOfTGroupPermission ArrayOfTUserPermission TPermissionReturn ArrayOfTPermissionReturn TSURLPermissionReturn ArrayOfTSURLPermissionReturn TSURLLifetimeReturnStatus ArrayOfTSURLLifetimeReturnStatus TSURLReturnStatus ArrayOfTSURLReturnStatus TMetaDataSpace ArrayOfTMetaDataSpace 
        Types overlapping:
        TExtraInfo ArrayOfTExtraInfo TGroupPermission TUserPermission ArrayOfString TRetentionPolicyInfo ArrayOfTGroupPermission ArrayOfTUserPermission 
        Input types wo common:
        TDirOption TCopyFileRequest ArrayOfTCopyFileRequest TPutFileRequest ArrayOfTPutFileRequest TGetFileRequest ArrayOfTGetFileRequest TTransferParameters ArrayOfUnsignedLong
        Output types wo common:
        TSupportedTransferProtocol ArrayOfTSupportedTransferProtocol TRequestTokenReturn ArrayOfTRequestTokenReturn TReturnStatus TRequestSummary ArrayOfTRequestSummary TCopyRequestFileStatus ArrayOfTCopyRequestFileStatus TPutRequestFileStatus ArrayOfTPutRequestFileStatus TBringOnlineRequestFileStatus ArrayOfTBringOnlineRequestFileStatus TGetRequestFileStatus ArrayOfTGetRequestFileStatus struct ArrayOfTMetaDataPathDetail TMetaDataPathDetail ArrayOfTMetaDataPathDetail TPermissionReturn ArrayOfTPermissionReturn TSURLPermissionReturn ArrayOfTSURLPermissionReturn TSURLLifetimeReturnStatus ArrayOfTSURLLifetimeReturnStatus TSURLReturnStatus ArrayOfTSURLReturnStatus TMetaDataSpace ArrayOfTMetaDataSpace
        */
        
        



	
/*
TRetentionPolicyInfo    
is the 
input/output 
	
*/           

CONSTRUCTOR_DEC(TRetentionPolicyInfo,
    srm22_TRetentionPolicy retentionPolicy,
    srm22_TAccessLatency accessLatency)
{
    srm22_TRetentionPolicyInfo* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(retentionPolicy);
	COPY_CHK(accessLatency);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(TRetentionPolicyInfo)
{
	if(!_arg)return;

	END_GFAL_FREE;
}



	
/*
TUserPermission    
is the 
input/output 
	
*/           

CONSTRUCTOR_DEC(TUserPermission,
    char*  userID,
    srm22_TPermissionMode mode)
{
    srm22_TUserPermission* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(userID);
	COPY_CHK(mode);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(TUserPermission)
{
	if(!_arg)return;
	FREE_SUB_OBJ(userID);

	END_GFAL_FREE;
}


	
    
/* 
ArrayOfTUserPermission    
is the 
input/output
*/
CONSTRUCTOR_DEC(ArrayOfTUserPermission,srm22_TUserPermission** userPermissionArray,int _size)	
{												
	srm22_ArrayOfTUserPermission* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->userPermissionArray = malloc(sizeof(srm22_TUserPermission*)*_size);	
	GCuAssertPtrNotNull(tdata1->userPermissionArray);	
	tdata1->__sizeuserPermissionArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TUserPermission,__sizeuserPermissionArray, userPermissionArray);
    

	
/*
TGroupPermission    
is the 
input/output 
	
*/           

CONSTRUCTOR_DEC(TGroupPermission,
    char*  groupID,
    srm22_TPermissionMode mode)
{
    srm22_TGroupPermission* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(groupID);
	COPY_CHK(mode);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(TGroupPermission)
{
	if(!_arg)return;
	FREE_SUB_OBJ(groupID);

	END_GFAL_FREE;
}


	
    
/* 
ArrayOfTGroupPermission    
is the 
input/output
*/
CONSTRUCTOR_DEC(ArrayOfTGroupPermission,srm22_TGroupPermission** groupPermissionArray,int _size)	
{												
	srm22_ArrayOfTGroupPermission* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->groupPermissionArray = malloc(sizeof(srm22_TGroupPermission*)*_size);	
	GCuAssertPtrNotNull(tdata1->groupPermissionArray);	
	tdata1->__sizegroupPermissionArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TGroupPermission,__sizegroupPermissionArray, groupPermissionArray);
    

	
/*
TReturnStatus    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TReturnStatus)
{
	if(!_arg)return;
		FREE_SUB_OBJ(explanation);

	END_GFAL_FREE;
}
                


	
/*
TSURLReturnStatus    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TSURLReturnStatus)
{
	if(!_arg)return;
		FREE_SUB_OBJ(surl);
	FREE_TYPE(TReturnStatus,status);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTSURLReturnStatus    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTSURLReturnStatus,srm22_TSURLReturnStatus** statusArray,int _size)	
{												
	srm22_ArrayOfTSURLReturnStatus* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->statusArray = malloc(sizeof(srm22_TSURLReturnStatus*)*_size);	
	GCuAssertPtrNotNull(tdata1->statusArray);	
	tdata1->__sizestatusArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TSURLReturnStatus,__sizestatusArray, statusArray);
    

	
/*
TSURLLifetimeReturnStatus    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TSURLLifetimeReturnStatus)
{
	if(!_arg)return;
		FREE_SUB_OBJ(surl);
	FREE_TYPE(TReturnStatus,status);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTSURLLifetimeReturnStatus    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTSURLLifetimeReturnStatus,srm22_TSURLLifetimeReturnStatus** statusArray,int _size)	
{												
	srm22_ArrayOfTSURLLifetimeReturnStatus* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->statusArray = malloc(sizeof(srm22_TSURLLifetimeReturnStatus*)*_size);	
	GCuAssertPtrNotNull(tdata1->statusArray);	
	tdata1->__sizestatusArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TSURLLifetimeReturnStatus,__sizestatusArray, statusArray);
    

	
/*
TMetaDataPathDetail    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TMetaDataPathDetail)
{
	if(!_arg)return;
		FREE_SUB_OBJ(surl);
	FREE_TYPE(TReturnStatus,status);
	FREE_SUB_OBJ(createdAtTime);
	FREE_SUB_OBJ(lastModificationTime);
	FREE_TYPE(TRetentionPolicyInfo,retentionPolicyInfo);
	FREE_TYPE(ArrayOfString,arrayOfSpaceTokens);
	FREE_TYPE(TUserPermission,ownerPermission);
	FREE_TYPE(TGroupPermission,groupPermission);
	FREE_SUB_OBJ(checkSumType);
	FREE_SUB_OBJ(checkSumValue);
	FREE_TYPE(ArrayOfTMetaDataPathDetail,arrayOfSubPaths);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTMetaDataPathDetail    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTMetaDataPathDetail,srm22_TMetaDataPathDetail** pathDetailArray,int _size)	
{												
	srm22_ArrayOfTMetaDataPathDetail* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->pathDetailArray = malloc(sizeof(srm22_TMetaDataPathDetail*)*_size);	
	GCuAssertPtrNotNull(tdata1->pathDetailArray);	
	tdata1->__sizepathDetailArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TMetaDataPathDetail,__sizepathDetailArray, pathDetailArray);
    

	
/*
TMetaDataSpace    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TMetaDataSpace)
{
	if(!_arg)return;
		FREE_SUB_OBJ(spaceToken);
	FREE_TYPE(TReturnStatus,status);
	FREE_TYPE(TRetentionPolicyInfo,retentionPolicyInfo);
	FREE_SUB_OBJ(owner);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTMetaDataSpace    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTMetaDataSpace,srm22_TMetaDataSpace** spaceDataArray,int _size)	
{												
	srm22_ArrayOfTMetaDataSpace* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->spaceDataArray = malloc(sizeof(srm22_TMetaDataSpace*)*_size);	
	GCuAssertPtrNotNull(tdata1->spaceDataArray);	
	tdata1->__sizespaceDataArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TMetaDataSpace,__sizespaceDataArray, spaceDataArray);
    

	
/*
TDirOption    
is the 
deep-input 
	
*/           

CONSTRUCTOR_DEC(TDirOption,
    short int isSourceADirectory,
    short int allLevelRecursive,
    int numOfLevels)
{
    srm22_TDirOption* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(isSourceADirectory);
	COPY_CHK(allLevelRecursive);
	COPY_CHK(numOfLevels);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(TDirOption)
{
	if(!_arg)return;

	END_GFAL_FREE;
}



	
/*
TExtraInfo    
is the 
input/output 
	
*/           

CONSTRUCTOR_DEC(TExtraInfo,
    char*  key,
    char*  value)
{
    srm22_TExtraInfo* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(key);
	COPY_CHK(value);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(TExtraInfo)
{
	if(!_arg)return;
	FREE_SUB_OBJ(key);
	FREE_SUB_OBJ(value);

	END_GFAL_FREE;
}


	
    
/* 
ArrayOfTExtraInfo    
is the 
input/output
*/
CONSTRUCTOR_DEC(ArrayOfTExtraInfo,srm22_TExtraInfo** extraInfoArray,int _size)	
{												
	srm22_ArrayOfTExtraInfo* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->extraInfoArray = malloc(sizeof(srm22_TExtraInfo*)*_size);	
	GCuAssertPtrNotNull(tdata1->extraInfoArray);	
	tdata1->__sizeextraInfoArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TExtraInfo,__sizeextraInfoArray, extraInfoArray);
    

	
/*
TTransferParameters    
is the 
deep-input 
	
*/           

CONSTRUCTOR_DEC(TTransferParameters,
    srm22_TAccessPattern accessPattern,
    srm22_TConnectionType connectionType,
    srm22_ArrayOfString*   arrayOfClientNetworks,
    srm22_ArrayOfString*   arrayOfTransferProtocols)
{
    srm22_TTransferParameters* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(accessPattern);
	COPY_CHK(connectionType);
	COPY_CHK(arrayOfClientNetworks);
	COPY_CHK(arrayOfTransferProtocols);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(TTransferParameters)
{
	if(!_arg)return;
	FREE_TYPE(ArrayOfString,arrayOfClientNetworks);
	FREE_TYPE(ArrayOfString,arrayOfTransferProtocols);

	END_GFAL_FREE;
}



	
/*
TGetFileRequest    
is the 
deep-input
    and getSurl is for:
    sourceSURL:anyURI! 
	
*/           

CONSTRUCTOR_DEC(TGetFileRequest,
    char*  sourceSURL,
    srm22_TDirOption*   dirOption)
{
srm22_TGetFileRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(sourceSURL);
	COPY_CHK(dirOption);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(TGetFileRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(sourceSURL);
	FREE_TYPE(TDirOption,dirOption);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_TGetFileRequest_TSurl(srm22_TGetFileRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)_arg->sourceSURL;
};

	
    
/* 
ArrayOfTGetFileRequest    
is the 
deep-input
*/
CONSTRUCTOR_DEC(ArrayOfTGetFileRequest,srm22_TGetFileRequest** requestArray,int _size)	
{												
	srm22_ArrayOfTGetFileRequest* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->requestArray = malloc(sizeof(srm22_TGetFileRequest*)*_size);	
	GCuAssertPtrNotNull(tdata1->requestArray);	
	tdata1->__sizerequestArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TGetFileRequest,__sizerequestArray, requestArray);
    

char* get_srm22_ArrayOfTGetFileRequest_TSurl(srm22_ArrayOfTGetFileRequest* _arg)    	
{						
	if(!_arg || (_arg->requestArray[0])) return NULL;
	return get_srm22_TGetFileRequest_TSurl(_arg->requestArray[0]);
}
    	
    

	
/*
TPutFileRequest    
is the 
deep-input
    and getSurl is for:
    targetSURL:anyURI! 
	
*/           

CONSTRUCTOR_DEC(TPutFileRequest,
    char*  targetSURL,
    ULONG64 expectedFileSize)
{
srm22_TPutFileRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(targetSURL);
	COPY_CHK(expectedFileSize);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(TPutFileRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(targetSURL);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_TPutFileRequest_TSurl(srm22_TPutFileRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)_arg->targetSURL;
};

	
    
/* 
ArrayOfTPutFileRequest    
is the 
deep-input
*/
CONSTRUCTOR_DEC(ArrayOfTPutFileRequest,srm22_TPutFileRequest** requestArray,int _size)	
{												
	srm22_ArrayOfTPutFileRequest* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->requestArray = malloc(sizeof(srm22_TPutFileRequest*)*_size);	
	GCuAssertPtrNotNull(tdata1->requestArray);	
	tdata1->__sizerequestArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TPutFileRequest,__sizerequestArray, requestArray);
    

char* get_srm22_ArrayOfTPutFileRequest_TSurl(srm22_ArrayOfTPutFileRequest* _arg)    	
{						
	if(!_arg || (_arg->requestArray[0])) return NULL;
	return get_srm22_TPutFileRequest_TSurl(_arg->requestArray[0]);
}
    	
    

	
/*
TCopyFileRequest    
is the 
deep-input
    and getSurl is for:
    sourceSURL:anyURI! 
	
*/           

CONSTRUCTOR_DEC(TCopyFileRequest,
    char*  sourceSURL,
    char*  targetSURL,
    srm22_TDirOption*   dirOption)
{
srm22_TCopyFileRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(sourceSURL);
	COPY_CHK(targetSURL);
	COPY_CHK(dirOption);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(TCopyFileRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(sourceSURL);
	FREE_SUB_OBJ(targetSURL);
	FREE_TYPE(TDirOption,dirOption);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_TCopyFileRequest_TSurl(srm22_TCopyFileRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)_arg->sourceSURL;
};

	
    
/* 
ArrayOfTCopyFileRequest    
is the 
deep-input
*/
CONSTRUCTOR_DEC(ArrayOfTCopyFileRequest,srm22_TCopyFileRequest** requestArray,int _size)	
{												
	srm22_ArrayOfTCopyFileRequest* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->requestArray = malloc(sizeof(srm22_TCopyFileRequest*)*_size);	
	GCuAssertPtrNotNull(tdata1->requestArray);	
	tdata1->__sizerequestArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TCopyFileRequest,__sizerequestArray, requestArray);
    

char* get_srm22_ArrayOfTCopyFileRequest_TSurl(srm22_ArrayOfTCopyFileRequest* _arg)    	
{						
	if(!_arg || (_arg->requestArray[0])) return NULL;
	return get_srm22_TCopyFileRequest_TSurl(_arg->requestArray[0]);
}
    	
    

	
/*
TGetRequestFileStatus    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TGetRequestFileStatus)
{
	if(!_arg)return;
		FREE_SUB_OBJ(sourceSURL);
	FREE_TYPE(TReturnStatus,status);
	FREE_SUB_OBJ(transferURL);
	FREE_TYPE(ArrayOfTExtraInfo,transferProtocolInfo);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTGetRequestFileStatus    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTGetRequestFileStatus,srm22_TGetRequestFileStatus** statusArray,int _size)	
{												
	srm22_ArrayOfTGetRequestFileStatus* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->statusArray = malloc(sizeof(srm22_TGetRequestFileStatus*)*_size);	
	GCuAssertPtrNotNull(tdata1->statusArray);	
	tdata1->__sizestatusArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TGetRequestFileStatus,__sizestatusArray, statusArray);
    

	
/*
TBringOnlineRequestFileStatus    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TBringOnlineRequestFileStatus)
{
	if(!_arg)return;
		FREE_SUB_OBJ(sourceSURL);
	FREE_TYPE(TReturnStatus,status);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTBringOnlineRequestFileStatus    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTBringOnlineRequestFileStatus,srm22_TBringOnlineRequestFileStatus** statusArray,int _size)	
{												
	srm22_ArrayOfTBringOnlineRequestFileStatus* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->statusArray = malloc(sizeof(srm22_TBringOnlineRequestFileStatus*)*_size);	
	GCuAssertPtrNotNull(tdata1->statusArray);	
	tdata1->__sizestatusArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TBringOnlineRequestFileStatus,__sizestatusArray, statusArray);
    

	
/*
TPutRequestFileStatus    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TPutRequestFileStatus)
{
	if(!_arg)return;
		FREE_SUB_OBJ(SURL);
	FREE_TYPE(TReturnStatus,status);
	FREE_SUB_OBJ(transferURL);
	FREE_TYPE(ArrayOfTExtraInfo,transferProtocolInfo);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTPutRequestFileStatus    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTPutRequestFileStatus,srm22_TPutRequestFileStatus** statusArray,int _size)	
{												
	srm22_ArrayOfTPutRequestFileStatus* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->statusArray = malloc(sizeof(srm22_TPutRequestFileStatus*)*_size);	
	GCuAssertPtrNotNull(tdata1->statusArray);	
	tdata1->__sizestatusArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TPutRequestFileStatus,__sizestatusArray, statusArray);
    

	
/*
TCopyRequestFileStatus    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TCopyRequestFileStatus)
{
	if(!_arg)return;
		FREE_SUB_OBJ(sourceSURL);
	FREE_SUB_OBJ(targetSURL);
	FREE_TYPE(TReturnStatus,status);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTCopyRequestFileStatus    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTCopyRequestFileStatus,srm22_TCopyRequestFileStatus** statusArray,int _size)	
{												
	srm22_ArrayOfTCopyRequestFileStatus* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->statusArray = malloc(sizeof(srm22_TCopyRequestFileStatus*)*_size);	
	GCuAssertPtrNotNull(tdata1->statusArray);	
	tdata1->__sizestatusArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TCopyRequestFileStatus,__sizestatusArray, statusArray);
    

	
/*
TRequestSummary    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TRequestSummary)
{
	if(!_arg)return;
		FREE_SUB_OBJ(requestToken);
	FREE_TYPE(TReturnStatus,status);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTRequestSummary    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTRequestSummary,srm22_TRequestSummary** summaryArray,int _size)	
{												
	srm22_ArrayOfTRequestSummary* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->summaryArray = malloc(sizeof(srm22_TRequestSummary*)*_size);	
	GCuAssertPtrNotNull(tdata1->summaryArray);	
	tdata1->__sizesummaryArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TRequestSummary,__sizesummaryArray, summaryArray);
    

	
/*
TSURLPermissionReturn    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TSURLPermissionReturn)
{
	if(!_arg)return;
		FREE_SUB_OBJ(surl);
	FREE_TYPE(TReturnStatus,status);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTSURLPermissionReturn    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTSURLPermissionReturn,srm22_TSURLPermissionReturn** surlPermissionArray,int _size)	
{												
	srm22_ArrayOfTSURLPermissionReturn* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->surlPermissionArray = malloc(sizeof(srm22_TSURLPermissionReturn*)*_size);	
	GCuAssertPtrNotNull(tdata1->surlPermissionArray);	
	tdata1->__sizesurlPermissionArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TSURLPermissionReturn,__sizesurlPermissionArray, surlPermissionArray);
    

	
/*
TPermissionReturn    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TPermissionReturn)
{
	if(!_arg)return;
		FREE_SUB_OBJ(surl);
	FREE_TYPE(TReturnStatus,status);
	FREE_SUB_OBJ(owner);
	FREE_TYPE(ArrayOfTUserPermission,arrayOfUserPermissions);
	FREE_TYPE(ArrayOfTGroupPermission,arrayOfGroupPermissions);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTPermissionReturn    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTPermissionReturn,srm22_TPermissionReturn** permissionArray,int _size)	
{												
	srm22_ArrayOfTPermissionReturn* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->permissionArray = malloc(sizeof(srm22_TPermissionReturn*)*_size);	
	GCuAssertPtrNotNull(tdata1->permissionArray);	
	tdata1->__sizepermissionArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TPermissionReturn,__sizepermissionArray, permissionArray);
    

	
/*
TRequestTokenReturn    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TRequestTokenReturn)
{
	if(!_arg)return;
		FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(createdAtTime);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTRequestTokenReturn    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTRequestTokenReturn,srm22_TRequestTokenReturn** tokenArray,int _size)	
{												
	srm22_ArrayOfTRequestTokenReturn* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->tokenArray = malloc(sizeof(srm22_TRequestTokenReturn*)*_size);	
	GCuAssertPtrNotNull(tdata1->tokenArray);	
	tdata1->__sizetokenArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TRequestTokenReturn,__sizetokenArray, tokenArray);
    

	
/*
TSupportedTransferProtocol    
is the 
deep-output 
	
*/           

GFAL_FREE_TYPE_DEC(TSupportedTransferProtocol)
{
	if(!_arg)return;
		FREE_SUB_OBJ(transferProtocol);
	FREE_TYPE(ArrayOfTExtraInfo,attributes);

	END_GFAL_FREE;
}
                

	
    
/* 
ArrayOfTSupportedTransferProtocol    
is the 
deep-output
*/
CONSTRUCTOR_DEC(ArrayOfTSupportedTransferProtocol,srm22_TSupportedTransferProtocol** protocolArray,int _size)	
{												
	srm22_ArrayOfTSupportedTransferProtocol* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1->protocolArray = malloc(sizeof(srm22_TSupportedTransferProtocol*)*_size);	
	GCuAssertPtrNotNull(tdata1->protocolArray);	
	tdata1->__sizeprotocolArray = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(TSupportedTransferProtocol,__sizeprotocolArray, protocolArray);
    

	
/*
srmReserveSpaceRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmReserveSpaceRequest,
    char*  authorizationID,
    char*  userSpaceTokenDescription,
    srm22_TRetentionPolicyInfo*   retentionPolicyInfo,
    ULONG64 desiredSizeOfTotalSpace,
    ULONG64 desiredSizeOfGuaranteedSpace,
    int desiredLifetimeOfReservedSpace,
    srm22_ArrayOfUnsignedLong*   arrayOfExpectedFileSizes,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo,
    srm22_TTransferParameters*   transferParameters)
{
    srm22_srmReserveSpaceRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(userSpaceTokenDescription);
	COPY_CHK(retentionPolicyInfo);
	COPY_CHK(desiredSizeOfTotalSpace);
	COPY_CHK(desiredSizeOfGuaranteedSpace);
	COPY_CHK(desiredLifetimeOfReservedSpace);
	COPY_CHK(arrayOfExpectedFileSizes);
	COPY_CHK(storageSystemInfo);
	COPY_CHK(transferParameters);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmReserveSpaceRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(userSpaceTokenDescription);
	FREE_TYPE(TRetentionPolicyInfo,retentionPolicyInfo);
	FREE_TYPE(ArrayOfUnsignedLong,arrayOfExpectedFileSizes);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);
	FREE_TYPE(TTransferParameters,transferParameters);

	END_GFAL_FREE;
}



	
/*
srmReserveSpaceResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmReserveSpaceResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_SUB_OBJ(requestToken);
	FREE_TYPE(TRetentionPolicyInfo,retentionPolicyInfo);
	FREE_SUB_OBJ(spaceToken);

	END_GFAL_FREE;
}
                


	
/*
srmStatusOfReserveSpaceRequestRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmStatusOfReserveSpaceRequestRequest,
    char*  authorizationID,
    char*  requestToken)
{
    srm22_srmStatusOfReserveSpaceRequestRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(requestToken);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmStatusOfReserveSpaceRequestRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(requestToken);

	END_GFAL_FREE;
}



	
/*
srmStatusOfReserveSpaceRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmStatusOfReserveSpaceRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(TRetentionPolicyInfo,retentionPolicyInfo);
	FREE_SUB_OBJ(spaceToken);

	END_GFAL_FREE;
}
                


	
/*
srmReleaseSpaceRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmReleaseSpaceRequest,
    char*  authorizationID,
    char*  spaceToken,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo,
    short int forceFileRelease)
{
    srm22_srmReleaseSpaceRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(spaceToken);
	COPY_CHK(storageSystemInfo);
	COPY_CHK(forceFileRelease);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmReleaseSpaceRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(spaceToken);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;
}



	
/*
srmReleaseSpaceResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmReleaseSpaceResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);

	END_GFAL_FREE;
}
                


	
/*
srmUpdateSpaceRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmUpdateSpaceRequest,
    char*  authorizationID,
    char*  spaceToken,
    ULONG64 newSizeOfTotalSpaceDesired,
    ULONG64 newSizeOfGuaranteedSpaceDesired,
    int newLifeTime,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo)
{
    srm22_srmUpdateSpaceRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(spaceToken);
	COPY_CHK(newSizeOfTotalSpaceDesired);
	COPY_CHK(newSizeOfGuaranteedSpaceDesired);
	COPY_CHK(newLifeTime);
	COPY_CHK(storageSystemInfo);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmUpdateSpaceRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(spaceToken);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;
}



	
/*
srmUpdateSpaceResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmUpdateSpaceResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_SUB_OBJ(requestToken);

	END_GFAL_FREE;
}
                


	
/*
srmStatusOfUpdateSpaceRequestRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmStatusOfUpdateSpaceRequestRequest,
    char*  authorizationID,
    char*  requestToken)
{
    srm22_srmStatusOfUpdateSpaceRequestRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(requestToken);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmStatusOfUpdateSpaceRequestRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(requestToken);

	END_GFAL_FREE;
}



	
/*
srmStatusOfUpdateSpaceRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmStatusOfUpdateSpaceRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);

	END_GFAL_FREE;
}
                


	
/*
srmGetSpaceMetaDataRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmGetSpaceMetaDataRequest,
    char*  authorizationID,
    srm22_ArrayOfString*   arrayOfSpaceTokens)
{
    srm22_srmGetSpaceMetaDataRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSpaceTokens);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmGetSpaceMetaDataRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfString,arrayOfSpaceTokens);

	END_GFAL_FREE;
}



	
/*
srmGetSpaceMetaDataResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmGetSpaceMetaDataResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTMetaDataSpace,arrayOfSpaceDetails);

	END_GFAL_FREE;
}
                


	
/*
srmChangeSpaceForFilesRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmChangeSpaceForFilesRequest,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSURLs,
    char*  targetSpaceToken,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo)
{
srm22_srmChangeSpaceForFilesRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(targetSpaceToken);
	COPY_CHK(storageSystemInfo);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmChangeSpaceForFilesRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);
	FREE_SUB_OBJ(targetSpaceToken);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmChangeSpaceForFilesRequest_TSurl(srm22_srmChangeSpaceForFilesRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmChangeSpaceForFilesResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmChangeSpaceForFilesResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_SUB_OBJ(requestToken);
	FREE_TYPE(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmStatusOfChangeSpaceForFilesRequestRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmStatusOfChangeSpaceForFilesRequestRequest,
    char*  authorizationID,
    char*  requestToken)
{
    srm22_srmStatusOfChangeSpaceForFilesRequestRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(requestToken);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmStatusOfChangeSpaceForFilesRequestRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(requestToken);

	END_GFAL_FREE;
}



	
/*
srmStatusOfChangeSpaceForFilesRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmStatusOfChangeSpaceForFilesRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmExtendFileLifeTimeInSpaceRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmExtendFileLifeTimeInSpaceRequest,
    char*  authorizationID,
    char*  spaceToken,
    srm22_ArrayOfString*  arrayOfSURLs,
    int newLifeTime)
{
srm22_srmExtendFileLifeTimeInSpaceRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(spaceToken);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(newLifeTime);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmExtendFileLifeTimeInSpaceRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(spaceToken);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmExtendFileLifeTimeInSpaceRequest_TSurl(srm22_srmExtendFileLifeTimeInSpaceRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmExtendFileLifeTimeInSpaceResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmExtendFileLifeTimeInSpaceResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSURLLifetimeReturnStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmPurgeFromSpaceRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmPurgeFromSpaceRequest,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSURLs,
    char*  spaceToken,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo)
{
srm22_srmPurgeFromSpaceRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(spaceToken);
	COPY_CHK(storageSystemInfo);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmPurgeFromSpaceRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);
	FREE_SUB_OBJ(spaceToken);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmPurgeFromSpaceRequest_TSurl(srm22_srmPurgeFromSpaceRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmPurgeFromSpaceResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmPurgeFromSpaceResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmGetSpaceTokensRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmGetSpaceTokensRequest,
    char*  userSpaceTokenDescription,
    char*  authorizationID)
{
    srm22_srmGetSpaceTokensRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(userSpaceTokenDescription);
	COPY_CHK(authorizationID);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmGetSpaceTokensRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(userSpaceTokenDescription);
	FREE_SUB_OBJ(authorizationID);

	END_GFAL_FREE;
}



	
/*
srmGetSpaceTokensResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmGetSpaceTokensResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfString,arrayOfSpaceTokens);

	END_GFAL_FREE;
}
                


	
/*
srmSetPermissionRequest    
is the 
main-input
    and getSurl is for:
    SURL:anyURI! 
	
*/           

CONSTRUCTOR_DEC(srmSetPermissionRequest,
    char*  authorizationID,
    char*  SURL,
    srm22_TPermissionType permissionType,
    srm22_TPermissionMode ownerPermission,
    srm22_ArrayOfTUserPermission*   arrayOfUserPermissions,
    srm22_ArrayOfTGroupPermission*   arrayOfGroupPermissions,
    srm22_TPermissionMode otherPermission,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo)
{
srm22_srmSetPermissionRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(SURL);
	COPY_CHK(permissionType);
	COPY_CHK(ownerPermission);
	COPY_CHK(arrayOfUserPermissions);
	COPY_CHK(arrayOfGroupPermissions);
	COPY_CHK(otherPermission);
	COPY_CHK(storageSystemInfo);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmSetPermissionRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(SURL);
	FREE_TYPE(ArrayOfTUserPermission,arrayOfUserPermissions);
	FREE_TYPE(ArrayOfTGroupPermission,arrayOfGroupPermissions);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmSetPermissionRequest_TSurl(srm22_srmSetPermissionRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)_arg->SURL;
};


	
/*
srmSetPermissionResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmSetPermissionResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);

	END_GFAL_FREE;
}
                


	
/*
srmCheckPermissionRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmCheckPermissionRequest,
    srm22_ArrayOfString*  arrayOfSURLs,
    char*  authorizationID,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo)
{
srm22_srmCheckPermissionRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(authorizationID);
	COPY_CHK(storageSystemInfo);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmCheckPermissionRequest)
{
	if(!_arg)return;
		FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);
	FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmCheckPermissionRequest_TSurl(srm22_srmCheckPermissionRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmCheckPermissionResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmCheckPermissionResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSURLPermissionReturn,arrayOfPermissions);

	END_GFAL_FREE;
}
                


	
/*
srmGetPermissionRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmGetPermissionRequest,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSURLs,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo)
{
srm22_srmGetPermissionRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(storageSystemInfo);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmGetPermissionRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmGetPermissionRequest_TSurl(srm22_srmGetPermissionRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmGetPermissionResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmGetPermissionResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTPermissionReturn,arrayOfPermissionReturns);

	END_GFAL_FREE;
}
                


	
/*
srmMkdirRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmMkdirRequest,
    char*  authorizationID,
    char*  directoryPath,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo)
{
    srm22_srmMkdirRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(directoryPath);
	COPY_CHK(storageSystemInfo);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmMkdirRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(directoryPath);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;
}



	
/*
srmMkdirResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmMkdirResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);

	END_GFAL_FREE;
}
                


	
/*
srmRmdirRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmRmdirRequest,
    char*  authorizationID,
    char*  directoryPath,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo,
    short int recursive)
{
    srm22_srmRmdirRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(directoryPath);
	COPY_CHK(storageSystemInfo);
	COPY_CHK(recursive);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmRmdirRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(directoryPath);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;
}



	
/*
srmRmdirResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmRmdirResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);

	END_GFAL_FREE;
}
                


	
/*
srmRmRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmRmRequest,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSURLs,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo)
{
srm22_srmRmRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(storageSystemInfo);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmRmRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmRmRequest_TSurl(srm22_srmRmRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmRmResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmRmResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmLsRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmLsRequest,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSURLs,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo,
    srm22_TFileStorageType fileStorageType,
    short int fullDetailedList,
    short int allLevelRecursive,
    int numOfLevels,
    int offset,
    int count)
{
srm22_srmLsRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(storageSystemInfo);
	COPY_CHK(fileStorageType);
	COPY_CHK(fullDetailedList);
	COPY_CHK(allLevelRecursive);
	COPY_CHK(numOfLevels);
	COPY_CHK(offset);
	COPY_CHK(count);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmLsRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmLsRequest_TSurl(srm22_srmLsRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmLsResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmLsResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_SUB_OBJ(requestToken);
	FREE_TYPE(ArrayOfTMetaDataPathDetail,details);

	END_GFAL_FREE;
}
                


	
/*
srmStatusOfLsRequestRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmStatusOfLsRequestRequest,
    char*  authorizationID,
    char*  requestToken,
    int offset,
    int count)
{
    srm22_srmStatusOfLsRequestRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(requestToken);
	COPY_CHK(offset);
	COPY_CHK(count);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmStatusOfLsRequestRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(requestToken);

	END_GFAL_FREE;
}



	
/*
srmStatusOfLsRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmStatusOfLsRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTMetaDataPathDetail,details);

	END_GFAL_FREE;
}
                


	
/*
srmMvRequest    
is the 
main-input
    and getSurl is for:
    fromSURL:anyURI! 
	
*/           

CONSTRUCTOR_DEC(srmMvRequest,
    char*  authorizationID,
    char*  fromSURL,
    char*  toSURL,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo)
{
srm22_srmMvRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(fromSURL);
	COPY_CHK(toSURL);
	COPY_CHK(storageSystemInfo);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmMvRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(fromSURL);
	FREE_SUB_OBJ(toSURL);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmMvRequest_TSurl(srm22_srmMvRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)_arg->fromSURL;
};


	
/*
srmMvResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmMvResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);

	END_GFAL_FREE;
}
                


	
/*
srmPrepareToGetRequest    
is the 
main-input
    and getSurl is for:
    sourceSURL:anyURI!requestArray:TGetFileRequest->arrayOfFileRequests:ArrayOfTGetFileRequest-> 
	
*/           

CONSTRUCTOR_DEC(srmPrepareToGetRequest,
    char*  authorizationID,
    srm22_ArrayOfTGetFileRequest*   arrayOfFileRequests,
    char*  userRequestDescription,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo,
    srm22_TFileStorageType desiredFileStorageType,
    int desiredTotalRequestTime,
    int desiredPinLifeTime,
    char*  targetSpaceToken,
    srm22_TRetentionPolicyInfo*   targetFileRetentionPolicyInfo,
    srm22_TTransferParameters*   transferParameters)
{
srm22_srmPrepareToGetRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfFileRequests);
	COPY_CHK(userRequestDescription);
	COPY_CHK(storageSystemInfo);
	COPY_CHK(desiredFileStorageType);
	COPY_CHK(desiredTotalRequestTime);
	COPY_CHK(desiredPinLifeTime);
	COPY_CHK(targetSpaceToken);
	COPY_CHK(targetFileRetentionPolicyInfo);
	COPY_CHK(transferParameters);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmPrepareToGetRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfTGetFileRequest,arrayOfFileRequests);
	FREE_SUB_OBJ(userRequestDescription);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);
	FREE_SUB_OBJ(targetSpaceToken);
	FREE_TYPE(TRetentionPolicyInfo,targetFileRetentionPolicyInfo);
	FREE_TYPE(TTransferParameters,transferParameters);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmPrepareToGetRequest_TSurl(srm22_srmPrepareToGetRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfTGetFileRequest_TSurl(_arg->arrayOfFileRequests);
};


	
/*
srmPrepareToGetResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmPrepareToGetResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_SUB_OBJ(requestToken);
	FREE_TYPE(ArrayOfTGetRequestFileStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmStatusOfGetRequestRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSourceSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmStatusOfGetRequestRequest,
    char*  requestToken,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSourceSURLs)
{
srm22_srmStatusOfGetRequestRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSourceSURLs);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmStatusOfGetRequestRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSourceSURLs);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmStatusOfGetRequestRequest_TSurl(srm22_srmStatusOfGetRequestRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSourceSURLs);
};


	
/*
srmStatusOfGetRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmStatusOfGetRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTGetRequestFileStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmBringOnlineRequest    
is the 
main-input
    and getSurl is for:
    sourceSURL:anyURI!requestArray:TGetFileRequest->arrayOfFileRequests:ArrayOfTGetFileRequest-> 
	
*/           

CONSTRUCTOR_DEC(srmBringOnlineRequest,
    char*  authorizationID,
    srm22_ArrayOfTGetFileRequest*   arrayOfFileRequests,
    char*  userRequestDescription,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo,
    srm22_TFileStorageType desiredFileStorageType,
    int desiredTotalRequestTime,
    int desiredLifeTime,
    char*  targetSpaceToken,
    srm22_TRetentionPolicyInfo*   targetFileRetentionPolicyInfo,
    srm22_TTransferParameters*   transferParameters,
    int deferredStartTime)
{
srm22_srmBringOnlineRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfFileRequests);
	COPY_CHK(userRequestDescription);
	COPY_CHK(storageSystemInfo);
	COPY_CHK(desiredFileStorageType);
	COPY_CHK(desiredTotalRequestTime);
	COPY_CHK(desiredLifeTime);
	COPY_CHK(targetSpaceToken);
	COPY_CHK(targetFileRetentionPolicyInfo);
	COPY_CHK(transferParameters);
	COPY_CHK(deferredStartTime);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmBringOnlineRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfTGetFileRequest,arrayOfFileRequests);
	FREE_SUB_OBJ(userRequestDescription);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);
	FREE_SUB_OBJ(targetSpaceToken);
	FREE_TYPE(TRetentionPolicyInfo,targetFileRetentionPolicyInfo);
	FREE_TYPE(TTransferParameters,transferParameters);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmBringOnlineRequest_TSurl(srm22_srmBringOnlineRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfTGetFileRequest_TSurl(_arg->arrayOfFileRequests);
};


	
/*
srmBringOnlineResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmBringOnlineResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_SUB_OBJ(requestToken);
	FREE_TYPE(ArrayOfTBringOnlineRequestFileStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmStatusOfBringOnlineRequestRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSourceSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmStatusOfBringOnlineRequestRequest,
    char*  requestToken,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSourceSURLs)
{
srm22_srmStatusOfBringOnlineRequestRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSourceSURLs);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmStatusOfBringOnlineRequestRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSourceSURLs);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmStatusOfBringOnlineRequestRequest_TSurl(srm22_srmStatusOfBringOnlineRequestRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSourceSURLs);
};


	
/*
srmStatusOfBringOnlineRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmStatusOfBringOnlineRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTBringOnlineRequestFileStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmPrepareToPutRequest    
is the 
main-input
    and getSurl is for:
    targetSURL:anyURI!requestArray:TPutFileRequest->arrayOfFileRequests:ArrayOfTPutFileRequest-> 
	
*/           

CONSTRUCTOR_DEC(srmPrepareToPutRequest,
    char*  authorizationID,
    srm22_ArrayOfTPutFileRequest*   arrayOfFileRequests,
    char*  userRequestDescription,
    srm22_TOverwriteMode overwriteOption,
    srm22_ArrayOfTExtraInfo*   storageSystemInfo,
    int desiredTotalRequestTime,
    int desiredPinLifeTime,
    int desiredFileLifeTime,
    srm22_TFileStorageType desiredFileStorageType,
    char*  targetSpaceToken,
    srm22_TRetentionPolicyInfo*   targetFileRetentionPolicyInfo,
    srm22_TTransferParameters*   transferParameters)
{
srm22_srmPrepareToPutRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfFileRequests);
	COPY_CHK(userRequestDescription);
	COPY_CHK(overwriteOption);
	COPY_CHK(storageSystemInfo);
	COPY_CHK(desiredTotalRequestTime);
	COPY_CHK(desiredPinLifeTime);
	COPY_CHK(desiredFileLifeTime);
	COPY_CHK(desiredFileStorageType);
	COPY_CHK(targetSpaceToken);
	COPY_CHK(targetFileRetentionPolicyInfo);
	COPY_CHK(transferParameters);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmPrepareToPutRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfTPutFileRequest,arrayOfFileRequests);
	FREE_SUB_OBJ(userRequestDescription);
	FREE_TYPE(ArrayOfTExtraInfo,storageSystemInfo);
	FREE_SUB_OBJ(targetSpaceToken);
	FREE_TYPE(TRetentionPolicyInfo,targetFileRetentionPolicyInfo);
	FREE_TYPE(TTransferParameters,transferParameters);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmPrepareToPutRequest_TSurl(srm22_srmPrepareToPutRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfTPutFileRequest_TSurl(_arg->arrayOfFileRequests);
};


	
/*
srmPrepareToPutResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmPrepareToPutResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_SUB_OBJ(requestToken);
	FREE_TYPE(ArrayOfTPutRequestFileStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmStatusOfPutRequestRequest    
is the 
main-input
    and getSurl is for:
    arrayOfTargetSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmStatusOfPutRequestRequest,
    char*  requestToken,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfTargetSURLs)
{
srm22_srmStatusOfPutRequestRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfTargetSURLs);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmStatusOfPutRequestRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfTargetSURLs);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmStatusOfPutRequestRequest_TSurl(srm22_srmStatusOfPutRequestRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfTargetSURLs);
};


	
/*
srmStatusOfPutRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmStatusOfPutRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTPutRequestFileStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmCopyRequest    
is the 
main-input
    and getSurl is for:
    sourceSURL:anyURI!requestArray:TCopyFileRequest->arrayOfFileRequests:ArrayOfTCopyFileRequest-> 
	
*/           

CONSTRUCTOR_DEC(srmCopyRequest,
    char*  authorizationID,
    srm22_ArrayOfTCopyFileRequest*   arrayOfFileRequests,
    char*  userRequestDescription,
    srm22_TOverwriteMode overwriteOption,
    int desiredTotalRequestTime,
    int desiredTargetSURLLifeTime,
    srm22_TFileStorageType targetFileStorageType,
    char*  targetSpaceToken,
    srm22_TRetentionPolicyInfo*   targetFileRetentionPolicyInfo,
    srm22_ArrayOfTExtraInfo*   sourceStorageSystemInfo,
    srm22_ArrayOfTExtraInfo*   targetStorageSystemInfo)
{
srm22_srmCopyRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfFileRequests);
	COPY_CHK(userRequestDescription);
	COPY_CHK(overwriteOption);
	COPY_CHK(desiredTotalRequestTime);
	COPY_CHK(desiredTargetSURLLifeTime);
	COPY_CHK(targetFileStorageType);
	COPY_CHK(targetSpaceToken);
	COPY_CHK(targetFileRetentionPolicyInfo);
	COPY_CHK(sourceStorageSystemInfo);
	COPY_CHK(targetStorageSystemInfo);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmCopyRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfTCopyFileRequest,arrayOfFileRequests);
	FREE_SUB_OBJ(userRequestDescription);
	FREE_SUB_OBJ(targetSpaceToken);
	FREE_TYPE(TRetentionPolicyInfo,targetFileRetentionPolicyInfo);
	FREE_TYPE(ArrayOfTExtraInfo,sourceStorageSystemInfo);
	FREE_TYPE(ArrayOfTExtraInfo,targetStorageSystemInfo);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmCopyRequest_TSurl(srm22_srmCopyRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfTCopyFileRequest_TSurl(_arg->arrayOfFileRequests);
};


	
/*
srmCopyResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmCopyResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_SUB_OBJ(requestToken);
	FREE_TYPE(ArrayOfTCopyRequestFileStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmStatusOfCopyRequestRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSourceSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmStatusOfCopyRequestRequest,
    char*  requestToken,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSourceSURLs,
    srm22_ArrayOfString*  arrayOfTargetSURLs)
{
srm22_srmStatusOfCopyRequestRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSourceSURLs);
	COPY_CHK(arrayOfTargetSURLs);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmStatusOfCopyRequestRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSourceSURLs);
	FREE_TYPE(ArrayOfAnyURI,arrayOfTargetSURLs);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmStatusOfCopyRequestRequest_TSurl(srm22_srmStatusOfCopyRequestRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSourceSURLs);
};


	
/*
srmStatusOfCopyRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmStatusOfCopyRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTCopyRequestFileStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmReleaseFilesRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmReleaseFilesRequest,
    char*  requestToken,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSURLs,
    short int doRemove)
{
srm22_srmReleaseFilesRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(doRemove);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmReleaseFilesRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmReleaseFilesRequest_TSurl(srm22_srmReleaseFilesRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmReleaseFilesResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmReleaseFilesResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmPutDoneRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmPutDoneRequest,
    char*  requestToken,
    char*  authorizationID,
    srm22_ArrayOfString*  arrayOfSURLs)
{
srm22_srmPutDoneRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(authorizationID);
	COPY_CHK(arrayOfSURLs);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmPutDoneRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(authorizationID);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmPutDoneRequest_TSurl(srm22_srmPutDoneRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmPutDoneResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmPutDoneResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmAbortRequestRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmAbortRequestRequest,
    char*  requestToken,
    char*  authorizationID)
{
    srm22_srmAbortRequestRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(authorizationID);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmAbortRequestRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(authorizationID);

	END_GFAL_FREE;
}



	
/*
srmAbortRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmAbortRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);

	END_GFAL_FREE;
}
                


	
/*
srmAbortFilesRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmAbortFilesRequest,
    char*  requestToken,
    srm22_ArrayOfString*  arrayOfSURLs,
    char*  authorizationID)
{
srm22_srmAbortFilesRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(authorizationID);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmAbortFilesRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(requestToken);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);
	FREE_SUB_OBJ(authorizationID);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmAbortFilesRequest_TSurl(srm22_srmAbortFilesRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmAbortFilesResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmAbortFilesResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmSuspendRequestRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmSuspendRequestRequest,
    char*  requestToken,
    char*  authorizationID)
{
    srm22_srmSuspendRequestRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(authorizationID);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmSuspendRequestRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(authorizationID);

	END_GFAL_FREE;
}



	
/*
srmSuspendRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmSuspendRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);

	END_GFAL_FREE;
}
                


	
/*
srmResumeRequestRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmResumeRequestRequest,
    char*  requestToken,
    char*  authorizationID)
{
    srm22_srmResumeRequestRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(requestToken);
	COPY_CHK(authorizationID);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmResumeRequestRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(requestToken);
	FREE_SUB_OBJ(authorizationID);

	END_GFAL_FREE;
}



	
/*
srmResumeRequestResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmResumeRequestResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);

	END_GFAL_FREE;
}
                


	
/*
srmGetRequestSummaryRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmGetRequestSummaryRequest,
    srm22_ArrayOfString*   arrayOfRequestTokens,
    char*  authorizationID)
{
    srm22_srmGetRequestSummaryRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(arrayOfRequestTokens);
	COPY_CHK(authorizationID);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmGetRequestSummaryRequest)
{
	if(!_arg)return;
	FREE_TYPE(ArrayOfString,arrayOfRequestTokens);
	FREE_SUB_OBJ(authorizationID);

	END_GFAL_FREE;
}



	
/*
srmGetRequestSummaryResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmGetRequestSummaryResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTRequestSummary,arrayOfRequestSummaries);

	END_GFAL_FREE;
}
                


	
/*
srmExtendFileLifeTimeRequest    
is the 
main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
	
*/           

CONSTRUCTOR_DEC(srmExtendFileLifeTimeRequest,
    char*  authorizationID,
    char*  requestToken,
    srm22_ArrayOfString*  arrayOfSURLs,
    int newFileLifeTime,
    int newPinLifeTime)
{
srm22_srmExtendFileLifeTimeRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);
	COPY_CHK(requestToken);
	COPY_CHK(arrayOfSURLs);
	COPY_CHK(newFileLifeTime);
	COPY_CHK(newPinLifeTime);

    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(srmExtendFileLifeTimeRequest)
{
	if(!_arg)return;
		FREE_SUB_OBJ(authorizationID);
	FREE_SUB_OBJ(requestToken);
	FREE_TYPE(ArrayOfAnyURI,arrayOfSURLs);

	END_GFAL_FREE;	
}
	

	
	
char* get_srm22_srmExtendFileLifeTimeRequest_TSurl(srm22_srmExtendFileLifeTimeRequest* _arg)
{	
	if(!_arg) return NULL;
	return (char*)get_srm22_ArrayOfAnyURI_TSurl(_arg->arrayOfSURLs);
};


	
/*
srmExtendFileLifeTimeResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmExtendFileLifeTimeResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSURLLifetimeReturnStatus,arrayOfFileStatuses);

	END_GFAL_FREE;
}
                


	
/*
srmGetRequestTokensRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmGetRequestTokensRequest,
    char*  userRequestDescription,
    char*  authorizationID)
{
    srm22_srmGetRequestTokensRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(userRequestDescription);
	COPY_CHK(authorizationID);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmGetRequestTokensRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(userRequestDescription);
	FREE_SUB_OBJ(authorizationID);

	END_GFAL_FREE;
}



	
/*
srmGetRequestTokensResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmGetRequestTokensResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTRequestTokenReturn,arrayOfRequestTokens);

	END_GFAL_FREE;
}
                


	
/*
srmGetTransferProtocolsRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmGetTransferProtocolsRequest,
    char*  authorizationID)
{
    srm22_srmGetTransferProtocolsRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmGetTransferProtocolsRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);

	END_GFAL_FREE;
}



	
/*
srmGetTransferProtocolsResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmGetTransferProtocolsResponse)
{
	if(!_arg)return;
		FREE_TYPE(TReturnStatus,returnStatus);
	FREE_TYPE(ArrayOfTSupportedTransferProtocol,protocolInfo);

	END_GFAL_FREE;
}
                


	
/*
srmPingRequest    
is the 
main-input 
	
*/           

CONSTRUCTOR_DEC(srmPingRequest,
    char*  authorizationID)
{
    srm22_srmPingRequest* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
	COPY_CHK(authorizationID);

	return tdata1;                
}
GFAL_FREE_TYPE_DEC(srmPingRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(authorizationID);

	END_GFAL_FREE;
}



	
/*
srmPingResponse    
is the 
main-output 
	
*/           

GFAL_FREE_TYPE_DEC(srmPingResponse)
{
	if(!_arg)return;
		FREE_SUB_OBJ(versionInfo);
	FREE_TYPE(ArrayOfTExtraInfo,otherInfo);

	END_GFAL_FREE;
}
                
/********** methods **********/
GFAL_WRAP_DEC(srmReserveSpace)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmReserveSpace);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmReserveSpaceRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmReserveSpace(&soap,srm_endpoint, "srmReserveSpace", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmReserveSpaceResponse(_res.srmReserveSpaceResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmStatusOfReserveSpaceRequest)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmStatusOfReserveSpaceRequest);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmStatusOfReserveSpaceRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmStatusOfReserveSpaceRequest(&soap,srm_endpoint, "srmStatusOfReserveSpaceRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmStatusOfReserveSpaceRequestResponse(_res.srmStatusOfReserveSpaceRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmReleaseSpace)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmReleaseSpace);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmReleaseSpaceRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmReleaseSpace(&soap,srm_endpoint, "srmReleaseSpace", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmReleaseSpaceResponse(_res.srmReleaseSpaceResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmUpdateSpace)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmUpdateSpace);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmUpdateSpaceRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmUpdateSpace(&soap,srm_endpoint, "srmUpdateSpace", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmUpdateSpaceResponse(_res.srmUpdateSpaceResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmStatusOfUpdateSpaceRequest)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmStatusOfUpdateSpaceRequest);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmStatusOfUpdateSpaceRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmStatusOfUpdateSpaceRequest(&soap,srm_endpoint, "srmStatusOfUpdateSpaceRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmStatusOfUpdateSpaceRequestResponse(_res.srmStatusOfUpdateSpaceRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmGetSpaceMetaData)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmGetSpaceMetaData);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmGetSpaceMetaDataRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmGetSpaceMetaData(&soap,srm_endpoint, "srmGetSpaceMetaData", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmGetSpaceMetaDataResponse(_res.srmGetSpaceMetaDataResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC_SURL(srmChangeSpaceForFiles)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmChangeSpaceForFiles);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmChangeSpaceForFilesRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmChangeSpaceForFilesRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmChangeSpaceForFiles(&soap,srm_endpoint, "srmChangeSpaceForFiles", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmChangeSpaceForFilesResponse(_res.srmChangeSpaceForFilesResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC(srmStatusOfChangeSpaceForFilesRequest)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmStatusOfChangeSpaceForFilesRequest);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmStatusOfChangeSpaceForFilesRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmStatusOfChangeSpaceForFilesRequest(&soap,srm_endpoint, "srmStatusOfChangeSpaceForFilesRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmStatusOfChangeSpaceForFilesRequestResponse(_res.srmStatusOfChangeSpaceForFilesRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC_SURL(srmExtendFileLifeTimeInSpace)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmExtendFileLifeTimeInSpace);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmExtendFileLifeTimeInSpaceRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmExtendFileLifeTimeInSpaceRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmExtendFileLifeTimeInSpace(&soap,srm_endpoint, "srmExtendFileLifeTimeInSpace", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmExtendFileLifeTimeInSpaceResponse(_res.srmExtendFileLifeTimeInSpaceResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmPurgeFromSpace)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmPurgeFromSpace);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmPurgeFromSpaceRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmPurgeFromSpaceRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmPurgeFromSpace(&soap,srm_endpoint, "srmPurgeFromSpace", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmPurgeFromSpaceResponse(_res.srmPurgeFromSpaceResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC(srmGetSpaceTokens)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmGetSpaceTokens);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmGetSpaceTokensRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmGetSpaceTokens(&soap,srm_endpoint, "srmGetSpaceTokens", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmGetSpaceTokensResponse(_res.srmGetSpaceTokensResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC_SURL(srmSetPermission)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmSetPermission);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmSetPermissionRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmSetPermissionRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmSetPermission(&soap,srm_endpoint, "srmSetPermission", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmSetPermissionResponse(_res.srmSetPermissionResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmCheckPermission)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmCheckPermission);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmCheckPermissionRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmCheckPermissionRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmCheckPermission(&soap,srm_endpoint, "srmCheckPermission", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmCheckPermissionResponse(_res.srmCheckPermissionResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmGetPermission)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmGetPermission);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmGetPermissionRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmGetPermissionRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmGetPermission(&soap,srm_endpoint, "srmGetPermission", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmGetPermissionResponse(_res.srmGetPermissionResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC(srmMkdir)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmMkdir);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmMkdirRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmMkdir(&soap,srm_endpoint, "srmMkdir", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmMkdirResponse(_res.srmMkdirResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmRmdir)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmRmdir);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmRmdirRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmRmdir(&soap,srm_endpoint, "srmRmdir", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmRmdirResponse(_res.srmRmdirResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC_SURL(srmRm)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmRm);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmRmRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmRmRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmRm(&soap,srm_endpoint, "srmRm", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmRmResponse(_res.srmRmResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmLs)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmLs);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmLsRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmLsRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmLs(&soap,srm_endpoint, "srmLs", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmLsResponse(_res.srmLsResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC(srmStatusOfLsRequest)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmStatusOfLsRequest);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmStatusOfLsRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmStatusOfLsRequest(&soap,srm_endpoint, "srmStatusOfLsRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmStatusOfLsRequestResponse(_res.srmStatusOfLsRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC_SURL(srmMv)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmMv);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmMvRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmMvRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmMv(&soap,srm_endpoint, "srmMv", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmMvResponse(_res.srmMvResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmPrepareToGet)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmPrepareToGet);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmPrepareToGetRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmPrepareToGetRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmPrepareToGet(&soap,srm_endpoint, "srmPrepareToGet", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmPrepareToGetResponse(_res.srmPrepareToGetResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmStatusOfGetRequest)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmStatusOfGetRequest);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmStatusOfGetRequestRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmStatusOfGetRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmStatusOfGetRequest(&soap,srm_endpoint, "srmStatusOfGetRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmStatusOfGetRequestResponse(_res.srmStatusOfGetRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmBringOnline)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmBringOnline);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmBringOnlineRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmBringOnlineRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmBringOnline(&soap,srm_endpoint, "srmBringOnline", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmBringOnlineResponse(_res.srmBringOnlineResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmStatusOfBringOnlineRequest)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmStatusOfBringOnlineRequest);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmStatusOfBringOnlineRequestRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmStatusOfBringOnlineRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmStatusOfBringOnlineRequest(&soap,srm_endpoint, "srmStatusOfBringOnlineRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmStatusOfBringOnlineRequestResponse(_res.srmStatusOfBringOnlineRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmPrepareToPut)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmPrepareToPut);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmPrepareToPutRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmPrepareToPutRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmPrepareToPut(&soap,srm_endpoint, "srmPrepareToPut", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmPrepareToPutResponse(_res.srmPrepareToPutResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmStatusOfPutRequest)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmStatusOfPutRequest);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmStatusOfPutRequestRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmStatusOfPutRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmStatusOfPutRequest(&soap,srm_endpoint, "srmStatusOfPutRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmStatusOfPutRequestResponse(_res.srmStatusOfPutRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmCopy)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmCopy);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmCopyRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmCopyRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmCopy(&soap,srm_endpoint, "srmCopy", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmCopyResponse(_res.srmCopyResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmStatusOfCopyRequest)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmStatusOfCopyRequest);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmStatusOfCopyRequestRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmStatusOfCopyRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmStatusOfCopyRequest(&soap,srm_endpoint, "srmStatusOfCopyRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmStatusOfCopyRequestResponse(_res.srmStatusOfCopyRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmReleaseFiles)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmReleaseFiles);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmReleaseFilesRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmReleaseFilesRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmReleaseFiles(&soap,srm_endpoint, "srmReleaseFiles", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmReleaseFilesResponse(_res.srmReleaseFilesResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC_SURL(srmPutDone)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmPutDone);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmPutDoneRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmPutDoneRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmPutDone(&soap,srm_endpoint, "srmPutDone", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmPutDoneResponse(_res.srmPutDoneResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC(srmAbortRequest)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmAbortRequest);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmAbortRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmAbortRequest(&soap,srm_endpoint, "srmAbortRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmAbortRequestResponse(_res.srmAbortRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC_SURL(srmAbortFiles)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmAbortFiles);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmAbortFilesRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmAbortFilesRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmAbortFiles(&soap,srm_endpoint, "srmAbortFiles", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmAbortFilesResponse(_res.srmAbortFilesResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC(srmSuspendRequest)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmSuspendRequest);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmSuspendRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmSuspendRequest(&soap,srm_endpoint, "srmSuspendRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmSuspendRequestResponse(_res.srmSuspendRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmResumeRequest)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmResumeRequest);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmResumeRequestRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmResumeRequest(&soap,srm_endpoint, "srmResumeRequest", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmResumeRequestResponse(_res.srmResumeRequestResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmGetRequestSummary)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmGetRequestSummary);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmGetRequestSummaryRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmGetRequestSummary(&soap,srm_endpoint, "srmGetRequestSummary", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmGetRequestSummaryResponse(_res.srmGetRequestSummaryResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC_SURL(srmExtendFileLifeTime)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(srmExtendFileLifeTime);
	STANDARD_SURL_CHECK_INIT(get_srm22_srmExtendFileLifeTimeRequest_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmExtendFileLifeTimeRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmExtendFileLifeTime(&soap,srm_endpoint, "srmExtendFileLifeTime", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmExtendFileLifeTimeResponse(_res.srmExtendFileLifeTimeResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}

GFAL_WRAP_DEC(srmGetRequestTokens)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmGetRequestTokens);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmGetRequestTokensRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmGetRequestTokens(&soap,srm_endpoint, "srmGetRequestTokens", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmGetRequestTokensResponse(_res.srmGetRequestTokensResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmGetTransferProtocols)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmGetTransferProtocols);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmGetTransferProtocolsRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmGetTransferProtocols(&soap,srm_endpoint, "srmGetTransferProtocols", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmGetTransferProtocolsResponse(_res.srmGetTransferProtocolsResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    

GFAL_WRAP_DEC(srmPing)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(srmPing);
	//do translation, check for conversion problems
	_req = conv2soap_srm22_srmPingRequest(&soap,elem);
	//execute
	if( ret =
	soap_call_srm22__srmPing(&soap,srm_endpoint, "srmPing", _req, &_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_srm22_srmPingResponse(_res.srmPingResponse);		
	STANDARD_CLEANUP_RETURN (0);	
}
    
