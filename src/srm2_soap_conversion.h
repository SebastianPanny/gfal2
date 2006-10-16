#ifndef SRM2_SOAP_CONVERSION_H_
#define SRM2_SOAP_CONVERSION_H_

#include "srm2Stub.h"
#include "srm2_ifce.h"

/*
 * Conversion methods for gsoap to gfal types
 * Types are defined in gfal_api.h.
 * For arrays we always use compound type with ptr and sizeArr.
 * 
 * <<Convention>>:
 * Embedded structures if hold pointers, such as arrays wrappers 
 * are hold as pointers for unification of freeing.
*/


#define DEF_GFALCONV_HEADERS_IN(_name)	\
struct srm2__ ## _name * conv2soap_ ## _name(struct soap *soap, const _name * _elem)
	
#define DEF_GFALCONV_HEADERS_OUT(_name)	\
_name * conv2gfal_ ## _name(const struct srm2__ ## _name * _elem)

#define DEF_GFALCONV_HEADERS_OUT_EXT(_nameOut,_nameIn)	\
_nameOut * conv2gfal_ ## _nameOut ## _ ## _nameIn(const struct _nameIn * _elem)


#define DEF_GFALCONV_HEADERS(_name)	\
DEF_GFALCONV_HEADERS_IN(_name); \
DEF_GFALCONV_HEADERS_OUT(_name);

#define DEF_GFALCONV_HEADERS_PTRIN(_name)	\
struct srm2__ ## _name * conv2soap_ ## _name(struct soap *soap, const _name _elem)
	
#define DEF_GFALCONV_HEADERS_PTROUT(_name)	\
_name conv2gfal_ ## _name(const struct srm2__ ## _name * _elem)


#define DEF_GFALCONVENUM_DEC(_name)	\
enum srm2__ ## _name* convEnum2soap_ ## _name(struct soap* soap, int _res);


#define SRM2_GFAL_FREEARRAY_TYPE_DEC(_typeName)	\
void freegfalArray_##_typeName(_typeName* _elem);
		
#define SRM2_CALL_GFAL_FREEARRAY_TYPE(_typeName,ptrName);	\
freegfalArray_##_typeName(ptrName);

#define SRM2_GFAL_FREE_TYPE_DEC(_typeName) void freeType_##_typeName(_typeName* _arg)

#define SRM2_GFAL_FREEARRAY_DEC(_name)	\
void freeArray_##_name(int i, _name** ptrArr)


//getter for single surl to get the endpoint
#undef getSURL_DEC
#define getSURL_DEC(_typeName)	TSURL get_##_typeName##_TSurl(_typeName* _arg)


DEF_GFALCONV_HEADERS_OUT(ArrayOfTRequestToken);	
SRM2_GFAL_FREEARRAY_DEC(TRequestToken);


DEF_GFALCONV_HEADERS_PTRIN(TSURL);
DEF_GFALCONV_HEADERS_PTROUT(TSURL);
void freeType_TSURL(TSURL _arg);

//CALL_GFAL_FREEARRAY_TYPE
DEF_GFALCONV_HEADERS_IN(ArrayOfTSURL);
SRM2_GFAL_FREEARRAY_TYPE_DEC(ArrayOfTSURL);



void freeArray_TSURL(int i, TSURL* ptrArr);


DEF_GFALCONVENUM_DEC(TStatusCode);
DEF_GFALCONVENUM_DEC(TSpaceType);
DEF_GFALCONVENUM_DEC(TFileStorageType);
DEF_GFALCONVENUM_DEC(TFileType);
DEF_GFALCONVENUM_DEC(TPermissionMode);
//////////////////////////////////

SRM2_GFAL_FREE_TYPE_DEC(TPermission);
SRM2_GFAL_FREEARRAY_TYPE_DEC(ArrayOfTPermission);

//principal+enum wrappers
DEF_GFALCONV_HEADERS_OUT_EXT(TPermission,srm2__TUserPermission);
DEF_GFALCONV_HEADERS_OUT_EXT(TPermission,srm2__TGroupPermission);

DEF_GFALCONV_HEADERS_OUT_EXT(ArrayOfTPermission,srm2__ArrayOfTUserPermission);
DEF_GFALCONV_HEADERS_OUT_EXT(ArrayOfTPermission,srm2__ArrayOfTGroupPermission);
////////////////////////////////// 

DEF_GFALCONVENUM_DEC(TPermissionType);
DEF_GFALCONVENUM_DEC(TRequestType);
DEF_GFALCONVENUM_DEC(TOverwriteMode);

DEF_GFALCONV_HEADERS_OUT(TReturnStatus);
SRM2_GFAL_FREE_TYPE_DEC(TReturnStatus);

DEF_GFALCONV_HEADERS_OUT(TSURLReturnStatus);
SRM2_GFAL_FREE_TYPE_DEC(TSURLReturnStatus);

DEF_GFALCONV_HEADERS_OUT(ArrayOfTSURLReturnStatus);
SRM2_GFAL_FREEARRAY_TYPE_DEC(ArrayOfTSURLReturnStatus);

DEF_GFALCONV_HEADERS(TDirOption);
SRM2_GFAL_FREE_TYPE_DEC(TDirOption);


DEF_GFALCONV_HEADERS_IN(TSURLInfo);
SRM2_GFAL_FREE_TYPE_DEC(TSURLInfo);
getSURL_DEC(TSURLInfo);



DEF_GFALCONV_HEADERS_IN(ArrayOfTSURLInfo);
SRM2_GFAL_FREEARRAY_TYPE_DEC(ArrayOfTSURLInfo);	
getSURL_DEC(ArrayOfTSURLInfo);


DEF_GFALCONV_HEADERS_IN(TGetFileRequest);
SRM2_GFAL_FREE_TYPE_DEC(TGetFileRequest);
getSURL_DEC(TGetFileRequest);

DEF_GFALCONV_HEADERS_IN(ArrayOfTGetFileRequest);
SRM2_GFAL_FREEARRAY_TYPE_DEC(ArrayOfTGetFileRequest);	

DEF_GFALCONV_HEADERS_IN(TPutFileRequest);
SRM2_GFAL_FREE_TYPE_DEC(TPutFileRequest);

DEF_GFALCONV_HEADERS_IN(ArrayOfTPutFileRequest);
SRM2_GFAL_FREEARRAY_TYPE_DEC(ArrayOfTPutFileRequest);	


//

DEF_GFALCONV_HEADERS_OUT(TPutRequestFileStatus);
SRM2_GFAL_FREE_TYPE_DEC(TPutRequestFileStatus);

DEF_GFALCONV_HEADERS_OUT(ArrayOfTPutRequestFileStatus);
SRM2_GFAL_FREEARRAY_TYPE_DEC(ArrayOfTPutRequestFileStatus);


// srmReleaseFiles types: TUserID, TSURL, Boolean, TReturnStataus, TSURLReturnStatus


DEF_GFALCONV_HEADERS_OUT(TGetRequestFileStatus);
SRM2_GFAL_FREE_TYPE_DEC(TGetRequestFileStatus);

DEF_GFALCONV_HEADERS_OUT(ArrayOfTGetRequestFileStatus);

// srmPrepareToGet
DEF_GFALCONV_HEADERS_IN(srmPrepareToGetRequest);
SRM2_GFAL_FREE_TYPE_DEC(srmPrepareToGetRequest);
getSURL_DEC(srmPrepareToGetRequest);

DEF_GFALCONV_HEADERS_OUT(srmPrepareToGetResponse);

//=============================
// srmPrepareToPut
DEF_GFALCONV_HEADERS_IN(srmPrepareToPutRequest);
SRM2_GFAL_FREE_TYPE_DEC(srmPrepareToPutRequest);
getSURL_DEC(srmPrepareToPutRequest);

DEF_GFALCONV_HEADERS_OUT(srmPrepareToPutResponse);
SRM2_GFAL_FREE_TYPE_DEC(srmPrepareToPutResponse);

// srmPutDone
DEF_GFALCONV_HEADERS_IN(srmPutDoneRequest);
SRM2_GFAL_FREE_TYPE_DEC(srmPutDoneRequest);
getSURL_DEC(srmPutDoneRequest);

DEF_GFALCONV_HEADERS_OUT(srmPutDoneResponse);
SRM2_GFAL_FREE_TYPE_DEC(srmPutDoneResponse);

// srmAbortRequest
/* srmAbortRequestRequest: 
struct srmAbortRequestRequest
{
	struct TRequestToken *requestToken;
	struct TUserID *userID;
};

 srmAbortRequestResponse: 
struct srmAbortRequestResponse
{
	struct TReturnStatus *returnStatus;
};
*/

// srmAbortFiles
DEF_GFALCONV_HEADERS_IN(srmAbortFilesRequest);
// srmAbortFilesResponse: 
DEF_GFALCONV_HEADERS_OUT(srmAbortFilesResponse);

// srmStatusOfGetRequest
DEF_GFALCONV_HEADERS_IN(srmStatusOfGetRequestRequest);
SRM2_GFAL_FREE_TYPE_DEC(srmStatusOfGetRequestRequest);
getSURL_DEC(srmStatusOfGetRequestRequest);

DEF_GFALCONV_HEADERS_OUT(srmStatusOfGetRequestResponse);
SRM2_GFAL_FREE_TYPE_DEC(srmStatusOfGetRequestResponse);

// srmStatusOfPutRequest
DEF_GFALCONV_HEADERS_IN(srmStatusOfPutRequestRequest);
SRM2_GFAL_FREE_TYPE_DEC(srmStatusOfPutRequestRequest);
getSURL_DEC(srmStatusOfPutRequestRequest);

DEF_GFALCONV_HEADERS_OUT(srmStatusOfPutRequestResponse);

// srmGetRequestSummary
DEF_GFALCONV_HEADERS_IN(srmGetRequestSummaryRequest);

/* TRequestSummary: */
DEF_GFALCONV_HEADERS_OUT(TRequestSummary);

DEF_GFALCONV_HEADERS_OUT(ArrayOfTRequestSummary);


DEF_GFALCONV_HEADERS_OUT(srmGetRequestSummaryResponse);


// srmGetRequestID
DEF_GFALCONV_HEADERS_IN(srmGetRequestIDRequest);

/* srmGetRequestIDResponse: */
DEF_GFALCONV_HEADERS_OUT(srmGetRequestIDResponse);

//

/* left for now
 srm2:srmSuspendRequestRequest: 
struct srm2__srmSuspendRequestRequest
{
	struct srm2__TRequestToken *requestToken;
	struct srm2__TUserID *userID;
};

 srm2:srmSuspendRequestResponse: 
struct srm2__srmSuspendRequestResponse
{
	struct srm2__TReturnStatus *returnStatus;
};
*/
/*
 srm2:srmResumeRequestRequest: 
struct srm2__srmResumeRequestRequest
{
	struct srm2__TRequestToken *requestToken;
	struct srm2__TUserID *userID;
};

 srm2:srmResumeRequestResponse: 
struct srm2__srmResumeRequestResponse
{
	struct srm2__TReturnStatus *returnStatus;
};*/
/* srm2:srmStatusOfCopyRequestRequest: 
struct srm2__srmStatusOfCopyRequestRequest
{
	struct srm2__TRequestToken *requestToken;
	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTSURL *arrayOfFromSURLs;
	struct srm2__ArrayOfTSURL *arrayOfToSURLs;
};

 srm2:srmStatusOfCopyRequestResponse: 
struct srm2__srmStatusOfCopyRequestResponse
{
	struct srm2__TReturnStatus *returnStatus;
	struct srm2__ArrayOfTCopyRequestFileStatus *arrayOfFileStatuses;
};
*/

//srmLs

/*struct srm2__TMetaDataPathDetail
{
	char *path;
	struct srm2__TReturnStatus *status;
	struct srm2__TSizeInBytes *size;
	struct srm2__TOwnerPermission *ownerPermission;
	struct srm2__ArrayOfTUserPermission *userPermissions;
	struct srm2__ArrayOfTGroupPermission *groupPermissions;
	struct srm2__TOtherPermission *otherPermission;
	struct srm2__TGMTTime *createdAtTime;
	struct srm2__TGMTTime *lastModificationTime;
	struct srm2__TUserID *owner;
	enum srm2__TFileStorageType *fileStorageType;
	enum srm2__TFileType *type;
	struct srm2__TLifeTimeInSeconds *lifetimeAssigned;
	struct srm2__TLifeTimeInSeconds *lifetimeLeft;
	struct srm2__TCheckSumType *checkSumType;
	struct srm2__TCheckSumValue *checkSumValue;
	struct srm2__TSURL *originalSURL;
	struct srm2__ArrayOfTMetaDataPathDetail *subPaths;
};

 srm2:ArrayOfTMetaDataPathDetail: 
struct srm2__ArrayOfTMetaDataPathDetail
{
	int __sizepathDetailArray;	 sequence of elements <pathDetailArray> 
	struct srm2__TMetaDataPathDetail **pathDetailArray;
};
*/

DEF_GFALCONV_HEADERS_OUT(TMetaDataPathDetail);
SRM2_GFAL_FREE_TYPE_DEC(TMetaDataPathDetail);


DEF_GFALCONV_HEADERS_OUT(ArrayOfTMetaDataPathDetail);
SRM2_GFAL_FREEARRAY_TYPE_DEC(ArrayOfTMetaDataPathDetail);

/*struct srm2__srmLsRequest
{
	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTSURLInfo *paths;
	enum srm2__TFileStorageType *fileStorageType;
	enum xsd__boolean *fullDetailedList;
	enum xsd__boolean *allLevelRecursive;
	int *numOfLevels;
	int *offset;
	int *count;
};

 srm2:srmLsResponse: 
struct srm2__srmLsResponse
{
	struct srm2__ArrayOfTMetaDataPathDetail *details;
	struct srm2__TReturnStatus *returnStatus;
};
*/

DEF_GFALCONV_HEADERS_IN(srmLsRequest);
SRM2_GFAL_FREE_TYPE_DEC(srmLsRequest);
getSURL_DEC(srmLsRequest);

DEF_GFALCONV_HEADERS_OUT(srmLsResponse);
SRM2_GFAL_FREE_TYPE_DEC(srmLsResponse);


/* srm2:srmCheckPermissionRequest: 
struct srm2__srmCheckPermissionRequest
{
	struct srm2__ArrayOfTSURLInfo *arrayOfSiteURLs;
	struct srm2__TUserID *userID;
	enum xsd__boolean *checkInLocalCacheOnly;
};
*/

DEF_GFALCONV_HEADERS_IN(srmCheckPermissionRequest);
SRM2_GFAL_FREE_TYPE_DEC(srmCheckPermissionRequest);
getSURL_DEC(srmCheckPermissionRequest);

/* srm2:TSURLPermissionReturn: 
struct srm2__TSURLPermissionReturn
{
	struct srm2__TReturnStatus *status;
	struct srm2__TSURL *surl;
	enum srm2__TPermissionMode *userPermission;
};
*/
DEF_GFALCONV_HEADERS_OUT(TSURLPermissionReturn);
SRM2_GFAL_FREE_TYPE_DEC(TSURLPermissionReturn);

/*
 srm2:ArrayOfTSURLPermissionReturn: 
struct srm2__ArrayOfTSURLPermissionReturn
{
	int __sizesurlPermissionArray;	 sequence of elements <surlPermissionArray> 
	struct srm2__TSURLPermissionReturn **surlPermissionArray;
};
*/

DEF_GFALCONV_HEADERS_OUT(ArrayOfTSURLPermissionReturn);
SRM2_GFAL_FREEARRAY_TYPE_DEC(ArrayOfTSURLPermissionReturn);


/* srm2:srmCheckPermissionResponse: 
struct srm2__srmCheckPermissionResponse
{
	struct srm2__ArrayOfTSURLPermissionReturn *arrayOfPermissions;
	struct srm2__TReturnStatus *returnStatus;
};
*/

DEF_GFALCONV_HEADERS_OUT(srmCheckPermissionResponse);
SRM2_GFAL_FREE_TYPE_DEC(srmCheckPermissionResponse);

/* srm2:srmReleaseFilesRequest: */
/*struct srm2__srmReleaseFilesRequest
{
	struct srm2__TRequestToken *requestToken;
	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTSURL *siteURLs;
	enum xsd__boolean *keepSpace;
};
*/

DEF_GFALCONV_HEADERS_IN(srmReleaseFilesRequest);
SRM2_GFAL_FREE_TYPE_DEC(srmReleaseFilesRequest);
getSURL_DEC(srmReleaseFilesRequest);


/*srm2:srmReleaseFilesResponse: 
struct srm2__srmReleaseFilesResponse
{
	struct srm2__TReturnStatus *returnStatus;
	struct srm2__ArrayOfTSURLReturnStatus *arrayOfFileStatuses;
};
*/

DEF_GFALCONV_HEADERS_OUT(srmReleaseFilesResponse);
SRM2_GFAL_FREE_TYPE_DEC(srmReleaseFilesResponse);


#endif /*SRM2_SOAP_CONVERSION_H_*/
