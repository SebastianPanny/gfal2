#ifndef SRM2_IFCE_H_
#define SRM2_IFCE_H_

#include "gfal_base.h"

#undef GFAL_WRAP_DEC
#define GFAL_WRAP_DEC(_typeName)	\
	int srm21_##_typeName(_typeName##Request* elem,_typeName##Response* res, int timeout, char* errbuf, int errbufsz)

//////////////////////////////////////////////
//types
#undef DEF_ARRAYOF
#define DEF_ARRAYOF(_name, _sizeN, arrN)	\
typedef struct _##ArrayOf##_name 		\
{ 								\
	int _sizeN;					\
	_name** arrN;				\
} ArrayOf##_name;				


//could be the same but for clarity let's distinguish
#undef DEF_GFAL_ARRAY
#define DEF_GFAL_ARRAY(_typeName,fieldName)	\
typedef struct _ArrayOf##_typeName	\
{									\
	int __size##fieldName;	\
	_typeName** fieldName;	\
} ArrayOf##_typeName;

#define INIT_CTX_PTR	\
	gfal_ctx ctx;		\
	strcpy((char*)&ctx.user,""); 		

// generates new struct out of fields args. 
#define CONSTRUCTOR_DEC(_n,...) \
	_n* gen_##_n(gfal_ctx* ctx,##__VA_ARGS__)

#define CONSTRUCTOR_ARRAY_DEC(_type,_Name)	\
CONSTRUCTOR_DEC(ArrayOf##_type,_type** _Name##Array ,int _size);


/// typedefs
typedef const char* TRequestToken;
DEF_ARRAYOF(TRequestToken,__sizerequestTokenArray,requestTokenArray);


typedef const char* TSpaceToken;
typedef const char* TUserID;
typedef const char* TGroupID;
typedef const char* TCheckSumType;
typedef const char* TCheckSumValue;
typedef const char* TGMTTime;

typedef const char* anyURI;
typedef anyURI TSURL;

typedef struct _ArrayOfTSURL
{
	int __sizesurlArray;	/* sequence of elements <surlArray> */
	TSURL *surlArray;
} ArrayOfTSURL;

CONSTRUCTOR_ARRAY_DEC(TSURL,surl);

/* srm2:ArrayOfTStrings: */
typedef struct _ArrayOfTStrings
{
	int __sizeArray;	/* sequence of elements <surlArray> */
	const char** Array;
} ArrayOfTStrings;


typedef anyURI TTURL;

typedef const char* TStorageSystemInfo;
typedef unsigned long TSizeInBytes;
typedef unsigned long TLifeTimeInSeconds;


// copied from srm2Stub.h
#ifndef _GFAL_TStatusCode
#define _GFAL_TStatusCode
typedef enum TStatusCode_ {SRM_SUCCESS = 0, SRM_FAILURE = 1, SRM_AUTHENTICATION_FAILURE = 2, SRM_UNAUTHORIZED_ACCESS = 3, SRM_INVALID_REQUEST = 4, SRM_INVALID_PATH = 5, SRM_FILE_LIFETIME_EXPIRED = 6, SRM_SPACE_LIFETIME_EXPIRED = 7, SRM_EXCEED_ALLOCATION = 8, SRM_NO_USER_SPACE = 9, SRM_NO_FREE_SPACE = 10, SRM_DUPLICATION_ERROR = 11, SRM_NON_EMPTY_DIRECTORY = 12, SRM_TOO_MANY_RESULTS = 13, SRM_INTERNAL_ERROR = 14, SRM_FATAL_INTERNAL_ERROR = 15, SRM_NOT_SUPPORTED = 16, SRM_REQUEST_QUEUED = 17, SRM_REQUEST_INPROGRESS = 18, SRM_REQUEST_SUSPENDED = 19, SRM_ABORTED = 20, SRM_RELEASED = 21, SRM_FILE_PINNED = 22, SRM_FILE_IN_CACHE = 23, SRM_SPACE_AVAILABLE = 24, SRM_LOWER_SPACE_GRANTED = 25, SRM_DONE = 26, SRM_CUSTOM_STATUS = 27}
	TStatusCode;
#endif

#ifndef _GFAL_TSpaceType
#define _GFAL_TSpaceType
typedef enum TSpaceType_ {gfal_Volatile = 0, gfal_Durable = 1, gfal_Permanent = 2}
	TSpaceType;

#endif


#ifndef _GFAL_TFileStorageType
#define _GFAL_TFileStorageType
typedef enum TFileStorageType_ {gfal_Volatile_ = 0, gfal_Durable_ = 1, gfal_Permanent_ = 2}
	TFileStorageType;

#endif


#ifndef _GFAL_TFileType
#define _GFAL_TFileType
typedef enum TFileType_ {gfal_File = 0, gfal_Directory = 1, gfal_Link = 2} 
	TFileType;
#endif

#ifndef _GFAL_TPermissionMode
#define _GFAL_TPermissionMode
typedef enum TPermissionMode_ {gfal_None = 0, gfal_X = 1, gfal_W = 2, gfal_WX = 3, gfal_R = 4, gfal_RX = 5, gfal_RW = 6, gfal_RWX = 7} 
	TPermissionMode;
#endif
typedef struct _TPermission 
{
	TPermissionMode mode;
	char* principal;
} TPermission;

//repeated types cludge	
typedef TPermission TUserPermission;
typedef TPermission TGroupPermission;
//////////////////////////////////

typedef TPermissionMode TOtherPermission; //no principal, regular enum
typedef TPermissionMode TOwnerPermission;
//////////////////////////////////
typedef struct _ArrayOfTPermission
{
	int __sizepermissionArray;	/* sequence of elements <surlReturnStatusArray> */
	TPermission **permissionArray;
} ArrayOfTPermission;


#ifndef _GFAL_TPermissionType
#define _GFAL_TPermissionType
typedef enum TPermissionType_ {gfal_ADD = 0, gfal_REMOVE = 1, gfal_CHANGE = 2} 
	TPermissionType;
#endif

#ifndef _GFAL_TRequestType
#define _GFAL_TRequestType
typedef enum TRequestType_ {gfal_PrepareToGet = 0, gfal_PrepareToPut = 1, gfal_Copy = 2}
	TRequestType;
#endif

#ifndef _GFAL_TOverwriteMode
#define _GFAL_TOverwriteMode
typedef enum TOverwriteMode_ {gfal_Never = 0, gfal_Always = 1, gfal_WhenFilesAreDifferent = 2}
	TOverwriteMode;
	
#endif

// structures 

typedef struct _TReturnStatus
{
	char *explanation;
	TStatusCode statusCode;
} TReturnStatus;


typedef struct _TSURLReturnStatus
{
	TReturnStatus* status;
	TSURL  surl;
} TSURLReturnStatus; 

DEF_GFAL_ARRAY(TSURLReturnStatus,surlReturnStatusArray);

/* TDirOption: */
typedef struct _TDirOption
{
	short int allLevelRecursive:2;
	short int isSourceADirectory:2;
	int numOfLevels;
} TDirOption;
CONSTRUCTOR_DEC(TDirOption,short int _allLevelRecursive,short int _isSourceADirectory,int _numOfLevels);

/* TSURLInfo: */
typedef struct _TSURLInfo
{
	TSURL SURLOrStFN;
	TStorageSystemInfo storageSystemInfo;
} TSURLInfo;

CONSTRUCTOR_DEC(TSURLInfo,TSURL t1, TStorageSystemInfo t2);

DEF_GFAL_ARRAY(TSURLInfo,surlInfoArray);
CONSTRUCTOR_ARRAY_DEC(TSURLInfo,surlInfo);

/* TGetFileRequest: */
typedef struct _TGetFileRequest
{
	TDirOption* dirOption;
	TFileStorageType fileStorageType;
	TSURLInfo* fromSURLInfo;
	TLifeTimeInSeconds lifetime;
	TSpaceToken spaceToken;
} TGetFileRequest; 
CONSTRUCTOR_DEC(TGetFileRequest,
	TDirOption* dirOption,
	TFileStorageType fileStorageType,
	TSURLInfo* fromSURLInfo,
	TLifeTimeInSeconds lifetime,
	TSpaceToken spaceToken);

/* ArrayOfTGetFileRequest: */
DEF_GFAL_ARRAY(TGetFileRequest,getRequestArray);
CONSTRUCTOR_ARRAY_DEC(TGetFileRequest,getRequest);

/* TPutFileRequest: */
typedef struct _TPutFileRequest
{
	TFileStorageType fileStorageType;
	TSizeInBytes knownSizeOfThisFile;
	TLifeTimeInSeconds lifetime;
	TSpaceToken spaceToken;
	TSURLInfo* toSURLInfo;
} TPutFileRequest;
CONSTRUCTOR_DEC(TPutFileRequest,
	TFileStorageType fileStorageType,
	TSizeInBytes knownSizeOfThisFile,
	TLifeTimeInSeconds lifetime,
	TSpaceToken spaceToken,
	TSURLInfo* toSURLInfo
	);
/* ArrayOfTPutFileRequest: */
DEF_GFAL_ARRAY(TPutFileRequest,putRequestArray);
CONSTRUCTOR_ARRAY_DEC(TPutFileRequest,putRequest);


/* TPutRequestFileStatus: */
typedef struct _TPutRequestFileStatus
{
	TLifeTimeInSeconds estimatedProcessingTime;
	TLifeTimeInSeconds estimatedWaitTimeOnQueue;
	TSizeInBytes fileSize;
	TLifeTimeInSeconds remainingPinTime;
	TSURL siteURL;
	TReturnStatus* status;
	TTURL transferURL;
} TPutRequestFileStatus;
/* ArrayOfTPutRequestFileStatus: */
DEF_GFAL_ARRAY(TPutRequestFileStatus,putStatusArray);

typedef struct _TGetRequestFileStatus
{
	TLifeTimeInSeconds estimatedProcessingTime;
	TLifeTimeInSeconds estimatedWaitTimeOnQueue;
	TSizeInBytes fileSize;
	TSURL fromSURLInfo;
	TLifeTimeInSeconds remainingPinTime;
	TReturnStatus* status;
	TTURL transferURL;
} TGetRequestFileStatus;
/* ArrayOfTGetRequestFileStatus: */
typedef struct _ArrayOfTGetRequestFileStatus
{
	int __sizegetStatusArray;	/* sequence of elements <getStatusArray> */
	TGetRequestFileStatus **getStatusArray;
} ArrayOfTGetRequestFileStatus;

typedef struct _srmPrepareToGetRequest
{
	TUserID userID;
	ArrayOfTGetFileRequest* arrayOfFileRequests;
	int nTransferProtocols;
	char** arrayOfTransferProtocols;
	char *userRequestDescription;
	TStorageSystemInfo storageSystemInfo;
	TLifeTimeInSeconds totalRetryTime;
} srmPrepareToGetRequest;
CONSTRUCTOR_DEC(srmPrepareToGetRequest,
	TUserID userID,
	ArrayOfTGetFileRequest* arrayOfFileRequests,
	int nTransferProtocols,
	char** arrayOfTransferProtocols,
	char *userRequestDescription,
	TStorageSystemInfo storageSystemInfo,
	TLifeTimeInSeconds totalRetryTime);


// srmPrepareToGetResponse: 
typedef struct _srmPrepareToGetResponse
{
	TRequestToken requestToken;
	TReturnStatus* returnStatus;
	ArrayOfTGetRequestFileStatus* arrayOfFileStatuses;
} srmPrepareToGetResponse ;
	
// srmPrepareToPut	
/* srmPrepareToPutRequest: */
typedef struct _srmPrepareToPutRequest
{
	TUserID userID;
	ArrayOfTPutFileRequest* arrayOfFileRequests;
	int nTransferProtocols;
	char** arrayOfTransferProtocols;
	char *userRequestDescription;
	TOverwriteMode overwriteOption;
	TStorageSystemInfo storageSystemInfo;
	TLifeTimeInSeconds totalRetryTime;
} srmPrepareToPutRequest;

CONSTRUCTOR_DEC(srmPrepareToPutRequest,
	TUserID userID,
	ArrayOfTPutFileRequest* arrayOfFileRequests,
	int nTransferProtocols,
	char** arrayOfTransferProtocols,
	char *userRequestDescription,
	TOverwriteMode overwriteOption,
	TStorageSystemInfo storageSystemInfo,
	TLifeTimeInSeconds totalRetryTime);

// srmPrepareToPutResponse: 
typedef struct _srmPrepareToPutResponse
{
	TRequestToken requestToken;
	TReturnStatus* returnStatus;
	ArrayOfTPutRequestFileStatus* arrayOfFileStatuses;
} srmPrepareToPutResponse;


/* srmPutDoneRequest: */
typedef struct _srmPutDoneRequest
{
	TRequestToken requestToken;
	TUserID userID;
	ArrayOfTSURL* arrayOfSiteURLs;
}srmPutDoneRequest;

// srmPutDoneResponse: 
typedef struct _srmPutDoneResponse
{
	TReturnStatus* returnStatus;
	ArrayOfTSURLReturnStatus* arrayOfFileStatuses;
} srmPutDoneResponse;

// srmAbortFilesRequest: 
typedef struct _srmAbortFilesRequest
{
	TRequestToken requestToken;
	ArrayOfTSURL* arrayOfSiteURLs;
	TUserID userID;
}srmAbortFilesRequest;

typedef struct _srmAbortFilesResponse
{
	TReturnStatus* returnStatus;
	ArrayOfTSURLReturnStatus* arrayOfFileStatuses;
} srmAbortFilesResponse;

// srmStatusOfGetRequestRequest: 
typedef struct _srmStatusOfGetRequestRequest
{
	TRequestToken requestToken;
	TUserID userID;
	ArrayOfTSURL* arrayOfFromSURLs;
} srmStatusOfGetRequestRequest;

// srmStatusOfGetRequestResponse: 
typedef struct _srmStatusOfGetRequestResponse
{
	TReturnStatus* returnStatus;
	ArrayOfTGetRequestFileStatus* arrayOfFileStatuses;
} srmStatusOfGetRequestResponse;

// srmStatusOfPutRequestRequest: 
typedef struct _srmStatusOfPutRequestRequest
{
	TRequestToken requestToken;
	TUserID userID;
	ArrayOfTSURL* arrayOfToSURLs;
} srmStatusOfPutRequestRequest;

// srmStatusOfPutRequestResponse: 
typedef struct _srmStatusOfPutRequestResponse
{
	TReturnStatus* returnStatus;
	ArrayOfTPutRequestFileStatus* arrayOfFileStatuses;
} srmStatusOfPutRequestResponse;

// srmGetRequestSummaryRequest: 
typedef struct _srmGetRequestSummaryRequest
{
	ArrayOfTStrings* arrayOfRequestTokens;
	TUserID userID;
} srmGetRequestSummaryRequest;

typedef struct _TRequestSummary
{
	short int isSuspended;
	int numOfFinishedRequests;
	int numOfProgressingRequests;
	int numOfQueuedRequests;
	struct TRequestToken *requestToken;
	enum TRequestType *requestType;
	int totalFilesInThisRequest;
} TRequestSummary;

/* ArrayOfTRequestSummary: */
typedef struct _ArrayOfTRequestSummary
{
	int __sizesummaryArray;	/* sequence of elements <summaryArray> */
	TRequestSummary **summaryArray;
} ArrayOfTRequestSummary;

// srmGetRequestSummaryResponse: 
typedef struct _srmGetRequestSummaryResponse
{
	ArrayOfTRequestSummary* arrayOfRequestSummaries;
	TReturnStatus* returnStatus;
} srmGetRequestSummaryResponse;

/* srmGetRequestIDRequest: */
typedef struct _srmGetRequestIDRequest
{
	char *userRequestDescription;
	TUserID userID;
} srmGetRequestIDRequest;

typedef struct _srmGetRequestIDResponse
{
	ArrayOfTStrings* arrayOfRequestTokens;
	TReturnStatus* returnStatus;
} srmGetRequestIDResponse;


typedef struct _TMetaDataPathDetail
{
	char *path;
	TReturnStatus* status;
	TSizeInBytes size;
	TPermissionMode ownerPermission;
	ArrayOfTPermission *userPermissions;
	ArrayOfTPermission *groupPermissions;
	TPermissionMode otherPermission;
	TGMTTime createdAtTime;
	TGMTTime lastModificationTime;
	TUserID owner;
	TFileStorageType fileStorageType;
	TFileType type;
	TLifeTimeInSeconds lifetimeAssigned;
	TLifeTimeInSeconds lifetimeLeft;
	TCheckSumType checkSumType;
	TCheckSumValue checkSumValue;
	TSURL originalSURL;
	struct _ArrayOfTMetaDataPathDetail *subPaths;
} TMetaDataPathDetail;
DEF_GFAL_ARRAY(TMetaDataPathDetail,pathDetailArray);

typedef struct _srmLsRequest
{
	TUserID userID;
	ArrayOfTSURLInfo *paths;
	TFileStorageType fileStorageType;
	short int fullDetailedList;	//boolean
	short int allLevelRecursive;	//boolean
	int numOfLevels;
	int offset;
	int count;
} srmLsRequest;

CONSTRUCTOR_DEC(srmLsRequest,
	TUserID userID,
	ArrayOfTSURLInfo *paths,
	TFileStorageType fileStorageType,
	short int fullDetailedList,		//boolean
	short int allLevelRecursive,	//boolean
	int numOfLevels,
	int offset,
	int count
);


typedef struct _srmLsResponse
{
	ArrayOfTMetaDataPathDetail *details;
	TReturnStatus *returnStatus;
} srmLsResponse;

typedef struct _srmCheckPermissionRequest
{
	ArrayOfTSURLInfo *arrayOfSiteURLs;
	TUserID userID;
	short int checkInLocalCacheOnly;	
} srmCheckPermissionRequest;
CONSTRUCTOR_DEC(srmCheckPermissionRequest,	
	TUserID userID,
	ArrayOfTSURLInfo *arrayOfSiteURLs,
	short int checkInLocalCacheOnly	
);

typedef struct _TSURLPermissionReturn
{
	TReturnStatus *status;
	TSURL surl;
	TPermissionMode userPermission;
} TSURLPermissionReturn;
DEF_GFAL_ARRAY(TSURLPermissionReturn,surlPermissionArray);

typedef struct _srmCheckPermissionResponse
{
	ArrayOfTSURLPermissionReturn *arrayOfPermissions;
	TReturnStatus *returnStatus;
} srmCheckPermissionResponse;

typedef struct _srmReleaseFilesRequest
{
	TRequestToken requestToken;
	ArrayOfTSURL *siteURLs;
	TUserID userID;
	short int keepSpace;
} srmReleaseFilesRequest;


typedef struct _srmReleaseFilesResponse
{
	ArrayOfTSURLReturnStatus *arrayOfFileStatuses;
	TReturnStatus *returnStatus;
} srmReleaseFilesResponse;
	
///////////////////////////////////////////
GFAL_WRAP_DEC(srmLs);
GFAL_WRAP_DEC(srmPrepareToGet);
GFAL_WRAP_DEC(srmPrepareToPut);
GFAL_WRAP_DEC(srmStatusOfGetRequest);
GFAL_WRAP_DEC(srmStatusOfPutRequest);
GFAL_WRAP_DEC(srmReleaseFiles);
GFAL_WRAP_DEC(srmCheckPermission);
GFAL_WRAP_DEC(srmPutDone);

#endif /*SRM2_IFCE_H_*/
