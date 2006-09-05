


#ifndef srm2_2_IFCE_H_
#define srm2_2_IFCE_H_

#include "gfal_base.h"

#undef GFAL_WRAP_DEC_SURL
#define GFAL_WRAP_DEC_SURL(_typeName)	\
    int srm22_##_typeName(srm22_##_typeName##Request* elem,srm22_##_typeName##Response* res, gfal_ctx* ctx)

#undef GFAL_WRAP_DEC
#define GFAL_WRAP_DEC(_typeName)	\
int srm22_##_typeName(srm22_##_typeName##Request* elem,srm22_##_typeName##Response* res, char* srm_endpoint, gfal_ctx* ctx)
    
//types

/* types of declarations:
* 	1. STRUCT
* 	2. ARRAY
* 	3. 1+destructor 
* 	4. 1+3+constructor
* 	5. 1+3+4+getSurl
*  6. 2+destructor 
* 	7. 2+6+constructor
* 	8. 2+6+7+getSurl
* 
*/

#undef DEF_ARRAYOF
#define DEF_ARRAYOF(_name, _sizeN, arrN)\
typedef struct _srm22_##ArrayOf##_name 	\
{ 										\
int _sizeN;							\
srm22_##_name** arrN;				\
} srm22_ArrayOf##_name;				

//could be the same but for clarity let's distinguish
#undef DEF_GFAL_ARRAY
#define DEF_GFAL_ARRAY(_typeName,fieldName)	\
typedef struct _srm22_ArrayOf##_typeName	\
{									\
int __size##fieldName;	\
srm22_##_typeName** fieldName;	\
} srm22_ArrayOf##_typeName;

#undef INIT_CTX_PTR
#define INIT_CTX_PTR	\
gfal_ctx ctx;		\
strcpy((char*)&ctx.user, "");

// generates new struct out of fields args.
#undef  CONSTRUCTOR_DEC
#define CONSTRUCTOR_DEC(_n,...) \
srm22_##_n* srm22_gen_##_n(gfal_ctx* ctx,##__VA_ARGS__)

#undef CONSTRUCTOR_ARRAY_DEC
#define CONSTRUCTOR_ARRAY_DEC(_type,_Name)	\
CONSTRUCTOR_DEC(ArrayOf##_type,srm22_##_type** _Name ,int _size);

// free dynamic objects' fields
#define GFAL_FREEARRAY_TYPE_DEC(_typeName)	\
void freegfalArray_srm22_##_typeName(srm22_##_typeName* _elem);

#define CALL_GFAL_FREEARRAY_TYPE(_typeName,ptrName);	\
freegfalArray_srm22_##_typeName(ptrName);

#define GFAL_FREE_TYPE_DEC(_typeName) void freeType_srm22_##_typeName(srm22_##_typeName* _arg)

#define GFAL_FREEARRAY_DEC(_name)	\
void freeArray_srm22_##_name(int i, _name** ptrArr)

//getter for single surl to get the endpoint
#undef getSURL_DEC
#define getSURL_DEC(_typeName)	char* get_srm22_##_typeName##_TSurl(srm22_##_typeName* _arg)


#define STRUCT_DEC(_n,...)		\
typedef struct _srm22_##_n {	\
__VA_ARGS__ }				\
srm22_##_n;

/* all input structs have constructor/destructor and sometimes getSurl exposed */
#define STRUCT_DEC_IN(_n,...)			\
STRUCT_DEC(_n,__VA_ARGS__);				\
GFAL_FREE_TYPE_DEC(_n);					

#define STRUCT_DEC_REQ(_n,...)			\
STRUCT_DEC_IN(_n,__VA_ARGS__);			\
getSURL_DEC(_n);

#define STRUCT_DEC_OUT(_n,...)			\
STRUCT_DEC(_n,__VA_ARGS__);				\
GFAL_FREE_TYPE_DEC(_n);

// iface version mapping
#define TYPEDEFCHAR(_name) typedef const char* srm22_##_name;
#define TYPEDEFSUB(_name1,_name2) typedef srm22_##_name1 srm22_##_name2;

#define TYPEDENUM_DEC(_n,...)	\
enum srm22_##_n {__VA_ARGS__};		\
typedef enum srm22_##_n srm22_##_n;
//predef types

STRUCT_DEC(ArrayOfString,	
int __sizestringArray;
char **stringArray;
);   

void freegfalArray_srm22_String(srm22_ArrayOfString* _elem);	
CONSTRUCTOR_DEC(ArrayOfString,char** stringArray ,int _size);	
	
STRUCT_DEC(ArrayOfUnsignedLong,	
int __sizeunsignedLongArray;
ULONG64 **unsignedLongArray;
);
void freegfalArray_srm22_UnsignedLong(srm22_ArrayOfUnsignedLong* _elem);	
CONSTRUCTOR_DEC(ArrayOfUnsignedLong,ULONG64** unsignedLongArray ,int _size);	
	
/// typedefs
TYPEDENUM_DEC(boolean ,srm22_false_ = 0, srm22_true_ = 1);    
        

        
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
        
        
        
        
        
        
TYPEDENUM_DEC(TFileStorageType, srm22_VOLATILE =0, srm22_DURABLE =1, srm22_PERMANENT =2);       

TYPEDENUM_DEC(TFileType, srm22_FILE =0, srm22_DIRECTORY =1, srm22_LINK =2);       

TYPEDENUM_DEC(TRetentionPolicy, srm22_REPLICA =0, srm22_OUTPUT =1, srm22_CUSTODIAL =2);       

TYPEDENUM_DEC(TAccessLatency, srm22_TAccessLatency_ONLINE =0, srm22_TAccessLatency_NEARLINE =1);       

TYPEDENUM_DEC(TPermissionMode, srm22_TPermissionMode_NONE =0, srm22_X =1, srm22_W =2, srm22_WX =3, srm22_R =4, srm22_RX =5, srm22_RW =6, srm22_RWX =7);       

TYPEDENUM_DEC(TPermissionType, srm22_ADD =0, srm22_REMOVE =1, srm22_CHANGE =2);       

TYPEDENUM_DEC(TRequestType, srm22_PREPARE_TO_GET =0, srm22_PREPARE_TO_PUT =1, srm22_COPY =2, srm22_BRING_ONLINE =3, srm22_RESERVE_SPACE =4, srm22_UPDATE_SPACE =5, srm22_CHANGE_SPACE_FOR_FILES =6, srm22_LS =7);       

TYPEDENUM_DEC(TOverwriteMode, srm22_NEVER =0, srm22_ALWAYS =1, srm22_WHEN_FILES_ARE_DIFFERENT =2);       

TYPEDENUM_DEC(TFileLocality, srm22_TFileLocality_ONLINE =0, srm22_TFileLocality_NEARLINE =1, srm22_ONLINE_AND_NEARLINE =2, srm22_LOST =3, srm22_TFileLocality_NONE =4, srm22_UNAVAILABLE =5);       

TYPEDENUM_DEC(TAccessPattern, srm22_TRANSFER_MODE =0, srm22_PROCESSING_MODE =1);       

TYPEDENUM_DEC(TConnectionType, srm22_WAN =0, srm22_LAN =1);       

TYPEDENUM_DEC(TStatusCode, srm22_SRM_SUCCESS =0, srm22_SRM_FAILURE =1, srm22_SRM_AUTHENTICATION_FAILURE =2, srm22_SRM_AUTHORIZATION_FAILURE =3, srm22_SRM_INVALID_REQUEST =4, srm22_SRM_INVALID_PATH =5, srm22_SRM_FILE_LIFETIME_EXPIRED =6, srm22_SRM_SPACE_LIFETIME_EXPIRED =7, srm22_SRM_EXCEED_ALLOCATION =8, srm22_SRM_NO_USER_SPACE =9, srm22_SRM_NO_FREE_SPACE =10, srm22_SRM_DUPLICATION_ERROR =11, srm22_SRM_NON_EMPTY_DIRECTORY =12, srm22_SRM_TOO_MANY_RESULTS =13, srm22_SRM_INTERNAL_ERROR =14, srm22_SRM_FATAL_INTERNAL_ERROR =15, srm22_SRM_NOT_SUPPORTED =16, srm22_SRM_REQUEST_QUEUED =17, srm22_SRM_REQUEST_INPROGRESS =18, srm22_SRM_REQUEST_SUSPENDED =19, srm22_SRM_ABORTED =20, srm22_SRM_RELEASED =21, srm22_SRM_FILE_PINNED =22, srm22_SRM_FILE_IN_CACHE =23, srm22_SRM_SPACE_AVAILABLE =24, srm22_SRM_LOWER_SPACE_GRANTED =25, srm22_SRM_DONE =26, srm22_SRM_PARTIAL_SUCCESS =27, srm22_SRM_REQUEST_TIMED_OUT =28, srm22_SRM_LAST_COPY =29, srm22_SRM_FILE_BUSY =30, srm22_SRM_FILE_LOST =31, srm22_SRM_FILE_UNAVAILABLE =32, srm22_SRM_CUSTOM_STATUS =33);       

/*
    TRetentionPolicyInfo    
    is the 
    input/output
*/    
STRUCT_DEC_IN(
        TRetentionPolicyInfo,
    srm22_TRetentionPolicy retentionPolicy;
    srm22_TAccessLatency accessLatency;
);

/*
    TUserPermission    
    is the 
    input/output
*/    
STRUCT_DEC_IN(
        TUserPermission,
    char*  userID;
    srm22_TPermissionMode mode;
);

/* 
    ArrayOfTUserPermission    
    is the 
    input/output
*/
	

DEF_GFAL_ARRAY(TUserPermission,userPermissionArray);		               
GFAL_FREEARRAY_TYPE_DEC(TUserPermission);			
CONSTRUCTOR_DEC(ArrayOfTUserPermission,srm22_TUserPermission** userPermissionArray,int _size);		

    
/*
    TGroupPermission    
    is the 
    input/output
*/    
STRUCT_DEC_IN(
        TGroupPermission,
    char*  groupID;
    srm22_TPermissionMode mode;
);

/* 
    ArrayOfTGroupPermission    
    is the 
    input/output
*/
	

DEF_GFAL_ARRAY(TGroupPermission,groupPermissionArray);		               
GFAL_FREEARRAY_TYPE_DEC(TGroupPermission);			
CONSTRUCTOR_DEC(ArrayOfTGroupPermission,srm22_TGroupPermission** groupPermissionArray,int _size);		

    
/*
    TReturnStatus    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TReturnStatus,
    srm22_TStatusCode statusCode;
    char*  explanation;
);

/*
    TSURLReturnStatus    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TSURLReturnStatus,
    char*  surl;
    srm22_TReturnStatus*   status;
);

/* 
    ArrayOfTSURLReturnStatus    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TSURLReturnStatus,statusArray);		               
GFAL_FREEARRAY_TYPE_DEC(TSURLReturnStatus);			
CONSTRUCTOR_DEC(ArrayOfTSURLReturnStatus,srm22_TSURLReturnStatus** statusArray,int _size);		

    
/*
    TSURLLifetimeReturnStatus    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TSURLLifetimeReturnStatus,
    char*  surl;
    srm22_TReturnStatus*   status;
    int fileLifetime;
    int pinLifetime;
);

/* 
    ArrayOfTSURLLifetimeReturnStatus    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TSURLLifetimeReturnStatus,statusArray);		               
GFAL_FREEARRAY_TYPE_DEC(TSURLLifetimeReturnStatus);			
CONSTRUCTOR_DEC(ArrayOfTSURLLifetimeReturnStatus,srm22_TSURLLifetimeReturnStatus** statusArray,int _size);		

    
/*
    TMetaDataPathDetail    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TMetaDataPathDetail,
    char*  surl;
    srm22_TReturnStatus*   status;
    ULONG64 size;
    char* createdAtTime;
    char* lastModificationTime;
    srm22_TFileStorageType fileStorageType;
    srm22_TRetentionPolicyInfo*   retentionPolicyInfo;
    srm22_TFileLocality fileLocality;
    srm22_ArrayOfString*   arrayOfSpaceTokens;
    srm22_TFileType type;
    int lifetimeAssigned;
    int lifetimeLeft;
    srm22_TUserPermission*   ownerPermission;
    srm22_TGroupPermission*   groupPermission;
    srm22_TPermissionMode otherPermission;
    char*  checkSumType;
    char*  checkSumValue;
    struct srm22_ArrayOfTMetaDataPathDetail*   arrayOfSubPaths;
);

/* 
    ArrayOfTMetaDataPathDetail    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TMetaDataPathDetail,pathDetailArray);		               
GFAL_FREEARRAY_TYPE_DEC(TMetaDataPathDetail);			
CONSTRUCTOR_DEC(ArrayOfTMetaDataPathDetail,srm22_TMetaDataPathDetail** pathDetailArray,int _size);		

    
/*
    TMetaDataSpace    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TMetaDataSpace,
    char*  spaceToken;
    srm22_TReturnStatus*   status;
    srm22_TRetentionPolicyInfo*   retentionPolicyInfo;
    char*  owner;
    ULONG64 totalSize;
    ULONG64 guaranteedSize;
    ULONG64 unusedSize;
    int lifetimeAssigned;
    int lifetimeLeft;
);

/* 
    ArrayOfTMetaDataSpace    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TMetaDataSpace,spaceDataArray);		               
GFAL_FREEARRAY_TYPE_DEC(TMetaDataSpace);			
CONSTRUCTOR_DEC(ArrayOfTMetaDataSpace,srm22_TMetaDataSpace** spaceDataArray,int _size);		

    
/*
    TDirOption    
    is the 
    deep-input
*/    
STRUCT_DEC_IN(
        TDirOption,
    short int isSourceADirectory;
    short int allLevelRecursive;
    int numOfLevels;
);

/*
    TExtraInfo    
    is the 
    input/output
*/    
STRUCT_DEC_IN(
        TExtraInfo,
    char*  key;
    char*  value;
);

/* 
    ArrayOfTExtraInfo    
    is the 
    input/output
*/
	

DEF_GFAL_ARRAY(TExtraInfo,extraInfoArray);		               
GFAL_FREEARRAY_TYPE_DEC(TExtraInfo);			
CONSTRUCTOR_DEC(ArrayOfTExtraInfo,srm22_TExtraInfo** extraInfoArray,int _size);		

    
/*
    TTransferParameters    
    is the 
    deep-input
*/    
STRUCT_DEC_IN(
        TTransferParameters,
    srm22_TAccessPattern accessPattern;
    srm22_TConnectionType connectionType;
    srm22_ArrayOfString*   arrayOfClientNetworks;
    srm22_ArrayOfString*   arrayOfTransferProtocols;
);

/*
    TGetFileRequest    
    is the 
    deep-input
    and getSurl is for:
    sourceSURL:anyURI! 
*/    
STRUCT_DEC_REQ(
        TGetFileRequest,
    char*  sourceSURL;
    srm22_TDirOption*   dirOption;
);

/* 
    ArrayOfTGetFileRequest    
    is the 
    deep-input
*/
	

DEF_GFAL_ARRAY(TGetFileRequest,requestArray);		               
GFAL_FREEARRAY_TYPE_DEC(TGetFileRequest);			
CONSTRUCTOR_DEC(ArrayOfTGetFileRequest,srm22_TGetFileRequest** requestArray,int _size);		

    
getSURL_DEC(ArrayOfTGetFileRequest);
    
/*
    TPutFileRequest    
    is the 
    deep-input
    and getSurl is for:
    targetSURL:anyURI! 
*/    
STRUCT_DEC_REQ(
        TPutFileRequest,
    char*  targetSURL;
    ULONG64 expectedFileSize;
);

/* 
    ArrayOfTPutFileRequest    
    is the 
    deep-input
*/
	

DEF_GFAL_ARRAY(TPutFileRequest,requestArray);		               
GFAL_FREEARRAY_TYPE_DEC(TPutFileRequest);			
CONSTRUCTOR_DEC(ArrayOfTPutFileRequest,srm22_TPutFileRequest** requestArray,int _size);		

    
getSURL_DEC(ArrayOfTPutFileRequest);
    
/*
    TCopyFileRequest    
    is the 
    deep-input
    and getSurl is for:
    sourceSURL:anyURI! 
*/    
STRUCT_DEC_REQ(
        TCopyFileRequest,
    char*  sourceSURL;
    char*  targetSURL;
    srm22_TDirOption*   dirOption;
);

/* 
    ArrayOfTCopyFileRequest    
    is the 
    deep-input
*/
	

DEF_GFAL_ARRAY(TCopyFileRequest,requestArray);		               
GFAL_FREEARRAY_TYPE_DEC(TCopyFileRequest);			
CONSTRUCTOR_DEC(ArrayOfTCopyFileRequest,srm22_TCopyFileRequest** requestArray,int _size);		

    
getSURL_DEC(ArrayOfTCopyFileRequest);
    
/*
    TGetRequestFileStatus    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TGetRequestFileStatus,
    char*  sourceSURL;
    ULONG64 fileSize;
    srm22_TReturnStatus*   status;
    int estimatedWaitTime;
    int remainingPinTime;
    char*  transferURL;
    srm22_ArrayOfTExtraInfo*   transferProtocolInfo;
);

/* 
    ArrayOfTGetRequestFileStatus    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TGetRequestFileStatus,statusArray);		               
GFAL_FREEARRAY_TYPE_DEC(TGetRequestFileStatus);			
CONSTRUCTOR_DEC(ArrayOfTGetRequestFileStatus,srm22_TGetRequestFileStatus** statusArray,int _size);		

    
/*
    TBringOnlineRequestFileStatus    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TBringOnlineRequestFileStatus,
    char*  sourceSURL;
    srm22_TReturnStatus*   status;
    ULONG64 fileSize;
    int estimatedWaitTime;
    int remainingPinTime;
);

/* 
    ArrayOfTBringOnlineRequestFileStatus    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TBringOnlineRequestFileStatus,statusArray);		               
GFAL_FREEARRAY_TYPE_DEC(TBringOnlineRequestFileStatus);			
CONSTRUCTOR_DEC(ArrayOfTBringOnlineRequestFileStatus,srm22_TBringOnlineRequestFileStatus** statusArray,int _size);		

    
/*
    TPutRequestFileStatus    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TPutRequestFileStatus,
    char*  SURL;
    srm22_TReturnStatus*   status;
    ULONG64 fileSize;
    int estimatedWaitTime;
    int remainingPinLifetime;
    int remainingFileLifetime;
    char*  transferURL;
    srm22_ArrayOfTExtraInfo*   transferProtocolInfo;
);

/* 
    ArrayOfTPutRequestFileStatus    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TPutRequestFileStatus,statusArray);		               
GFAL_FREEARRAY_TYPE_DEC(TPutRequestFileStatus);			
CONSTRUCTOR_DEC(ArrayOfTPutRequestFileStatus,srm22_TPutRequestFileStatus** statusArray,int _size);		

    
/*
    TCopyRequestFileStatus    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TCopyRequestFileStatus,
    char*  sourceSURL;
    char*  targetSURL;
    srm22_TReturnStatus*   status;
    ULONG64 fileSize;
    int estimatedWaitTime;
    int remainingFileLifetime;
);

/* 
    ArrayOfTCopyRequestFileStatus    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TCopyRequestFileStatus,statusArray);		               
GFAL_FREEARRAY_TYPE_DEC(TCopyRequestFileStatus);			
CONSTRUCTOR_DEC(ArrayOfTCopyRequestFileStatus,srm22_TCopyRequestFileStatus** statusArray,int _size);		

    
/*
    TRequestSummary    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TRequestSummary,
    char*  requestToken;
    srm22_TReturnStatus*   status;
    srm22_TRequestType requestType;
    int totalNumFilesInRequest;
    int numOfCompletedFiles;
    int numOfWaitingFiles;
    int numOfFailedFiles;
);

/* 
    ArrayOfTRequestSummary    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TRequestSummary,summaryArray);		               
GFAL_FREEARRAY_TYPE_DEC(TRequestSummary);			
CONSTRUCTOR_DEC(ArrayOfTRequestSummary,srm22_TRequestSummary** summaryArray,int _size);		

    
/*
    TSURLPermissionReturn    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TSURLPermissionReturn,
    char*  surl;
    srm22_TReturnStatus*   status;
    srm22_TPermissionMode permission;
);

/* 
    ArrayOfTSURLPermissionReturn    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TSURLPermissionReturn,surlPermissionArray);		               
GFAL_FREEARRAY_TYPE_DEC(TSURLPermissionReturn);			
CONSTRUCTOR_DEC(ArrayOfTSURLPermissionReturn,srm22_TSURLPermissionReturn** surlPermissionArray,int _size);		

    
/*
    TPermissionReturn    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TPermissionReturn,
    char*  surl;
    srm22_TReturnStatus*   status;
    char*  owner;
    srm22_TPermissionMode ownerPermission;
    srm22_ArrayOfTUserPermission*   arrayOfUserPermissions;
    srm22_ArrayOfTGroupPermission*   arrayOfGroupPermissions;
    srm22_TPermissionMode otherPermission;
);

/* 
    ArrayOfTPermissionReturn    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TPermissionReturn,permissionArray);		               
GFAL_FREEARRAY_TYPE_DEC(TPermissionReturn);			
CONSTRUCTOR_DEC(ArrayOfTPermissionReturn,srm22_TPermissionReturn** permissionArray,int _size);		

    
/*
    TRequestTokenReturn    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TRequestTokenReturn,
    char*  requestToken;
    char* createdAtTime;
);

/* 
    ArrayOfTRequestTokenReturn    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TRequestTokenReturn,tokenArray);		               
GFAL_FREEARRAY_TYPE_DEC(TRequestTokenReturn);			
CONSTRUCTOR_DEC(ArrayOfTRequestTokenReturn,srm22_TRequestTokenReturn** tokenArray,int _size);		

    
/*
    TSupportedTransferProtocol    
    is the 
    deep-output
*/
STRUCT_DEC_OUT(
         TSupportedTransferProtocol,
    char*  transferProtocol;
    srm22_ArrayOfTExtraInfo*   attributes;
);

/* 
    ArrayOfTSupportedTransferProtocol    
    is the 
    deep-output
*/
	

DEF_GFAL_ARRAY(TSupportedTransferProtocol,protocolArray);		               
GFAL_FREEARRAY_TYPE_DEC(TSupportedTransferProtocol);			
CONSTRUCTOR_DEC(ArrayOfTSupportedTransferProtocol,srm22_TSupportedTransferProtocol** protocolArray,int _size);		

    
/*
    srmReserveSpaceRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmReserveSpaceRequest,
    char*  authorizationID;
    char*  userSpaceTokenDescription;
    srm22_TRetentionPolicyInfo*   retentionPolicyInfo;
    ULONG64 desiredSizeOfTotalSpace;
    ULONG64 desiredSizeOfGuaranteedSpace;
    int desiredLifetimeOfReservedSpace;
    srm22_ArrayOfUnsignedLong*   arrayOfExpectedFileSizes;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
    srm22_TTransferParameters*   transferParameters;
);

/*
    srmReserveSpaceResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmReserveSpaceResponse,
    srm22_TReturnStatus*   returnStatus;
    char*  requestToken;
    int estimatedProcessingTime;
    srm22_TRetentionPolicyInfo*   retentionPolicyInfo;
    ULONG64 sizeOfTotalReservedSpace;
    ULONG64 sizeOfGuaranteedReservedSpace;
    int lifetimeOfReservedSpace;
    char*  spaceToken;
);

/*
    srmStatusOfReserveSpaceRequestRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmStatusOfReserveSpaceRequestRequest,
    char*  authorizationID;
    char*  requestToken;
);

/*
    srmStatusOfReserveSpaceRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmStatusOfReserveSpaceRequestResponse,
    srm22_TReturnStatus*   returnStatus;
    int estimatedProcessingTime;
    srm22_TRetentionPolicyInfo*   retentionPolicyInfo;
    ULONG64 sizeOfTotalReservedSpace;
    ULONG64 sizeOfGuaranteedReservedSpace;
    int lifetimeOfReservedSpace;
    char*  spaceToken;
);

/*
    srmReleaseSpaceRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmReleaseSpaceRequest,
    char*  authorizationID;
    char*  spaceToken;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
    short int forceFileRelease;
);

/*
    srmReleaseSpaceResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmReleaseSpaceResponse,
    srm22_TReturnStatus*   returnStatus;
);

/*
    srmUpdateSpaceRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmUpdateSpaceRequest,
    char*  authorizationID;
    char*  spaceToken;
    ULONG64 newSizeOfTotalSpaceDesired;
    ULONG64 newSizeOfGuaranteedSpaceDesired;
    int newLifeTime;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
);

/*
    srmUpdateSpaceResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmUpdateSpaceResponse,
    srm22_TReturnStatus*   returnStatus;
    char*  requestToken;
    ULONG64 sizeOfTotalSpace;
    ULONG64 sizeOfGuaranteedSpace;
    int lifetimeGranted;
);

/*
    srmStatusOfUpdateSpaceRequestRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmStatusOfUpdateSpaceRequestRequest,
    char*  authorizationID;
    char*  requestToken;
);

/*
    srmStatusOfUpdateSpaceRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmStatusOfUpdateSpaceRequestResponse,
    srm22_TReturnStatus*   returnStatus;
    ULONG64 sizeOfTotalSpace;
    ULONG64 sizeOfGuaranteedSpace;
    int lifetimeGranted;
);

/*
    srmGetSpaceMetaDataRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmGetSpaceMetaDataRequest,
    char*  authorizationID;
    srm22_ArrayOfString*   arrayOfSpaceTokens;
);

/*
    srmGetSpaceMetaDataResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmGetSpaceMetaDataResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTMetaDataSpace*   arrayOfSpaceDetails;
);

/*
    srmChangeSpaceForFilesRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmChangeSpaceForFilesRequest,
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSURLs;
    char*  targetSpaceToken;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
);

/*
    srmChangeSpaceForFilesResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmChangeSpaceForFilesResponse,
    srm22_TReturnStatus*   returnStatus;
    char*  requestToken;
    int estimatedProcessingTime;
    srm22_ArrayOfTSURLReturnStatus*   arrayOfFileStatuses;
);

/*
    srmStatusOfChangeSpaceForFilesRequestRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmStatusOfChangeSpaceForFilesRequestRequest,
    char*  authorizationID;
    char*  requestToken;
);

/*
    srmStatusOfChangeSpaceForFilesRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmStatusOfChangeSpaceForFilesRequestResponse,
    srm22_TReturnStatus*   returnStatus;
    int estimatedProcessingTime;
    srm22_ArrayOfTSURLReturnStatus*   arrayOfFileStatuses;
);

/*
    srmExtendFileLifeTimeInSpaceRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmExtendFileLifeTimeInSpaceRequest,
    char*  authorizationID;
    char*  spaceToken;
    srm22_ArrayOfString*  arrayOfSURLs;
    int newLifeTime;
);

/*
    srmExtendFileLifeTimeInSpaceResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmExtendFileLifeTimeInSpaceResponse,
    srm22_TReturnStatus*   returnStatus;
    int newTimeExtended;
    srm22_ArrayOfTSURLLifetimeReturnStatus*   arrayOfFileStatuses;
);

/*
    srmPurgeFromSpaceRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmPurgeFromSpaceRequest,
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSURLs;
    char*  spaceToken;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
);

/*
    srmPurgeFromSpaceResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmPurgeFromSpaceResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTSURLReturnStatus*   arrayOfFileStatuses;
);

/*
    srmGetSpaceTokensRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmGetSpaceTokensRequest,
    char*  userSpaceTokenDescription;
    char*  authorizationID;
);

/*
    srmGetSpaceTokensResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmGetSpaceTokensResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfString*   arrayOfSpaceTokens;
);

/*
    srmSetPermissionRequest    
    is the 
    main-input
    and getSurl is for:
    SURL:anyURI! 
*/    
STRUCT_DEC_REQ(
        srmSetPermissionRequest,
    char*  authorizationID;
    char*  SURL;
    srm22_TPermissionType permissionType;
    srm22_TPermissionMode ownerPermission;
    srm22_ArrayOfTUserPermission*   arrayOfUserPermissions;
    srm22_ArrayOfTGroupPermission*   arrayOfGroupPermissions;
    srm22_TPermissionMode otherPermission;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
);

/*
    srmSetPermissionResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmSetPermissionResponse,
    srm22_TReturnStatus*   returnStatus;
);

/*
    srmCheckPermissionRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmCheckPermissionRequest,
    srm22_ArrayOfString*  arrayOfSURLs;
    char*  authorizationID;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
);

/*
    srmCheckPermissionResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmCheckPermissionResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTSURLPermissionReturn*   arrayOfPermissions;
);

/*
    srmGetPermissionRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmGetPermissionRequest,
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSURLs;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
);

/*
    srmGetPermissionResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmGetPermissionResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTPermissionReturn*   arrayOfPermissionReturns;
);

/*
    srmMkdirRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmMkdirRequest,
    char*  authorizationID;
    char*  directoryPath;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
);

/*
    srmMkdirResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmMkdirResponse,
    srm22_TReturnStatus*   returnStatus;
);

/*
    srmRmdirRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmRmdirRequest,
    char*  authorizationID;
    char*  directoryPath;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
    short int recursive;
);

/*
    srmRmdirResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmRmdirResponse,
    srm22_TReturnStatus*   returnStatus;
);

/*
    srmRmRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmRmRequest,
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSURLs;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
);

/*
    srmRmResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmRmResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTSURLReturnStatus*   arrayOfFileStatuses;
);

/*
    srmLsRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmLsRequest,
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSURLs;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
    srm22_TFileStorageType fileStorageType;
    short int fullDetailedList;
    short int allLevelRecursive;
    int numOfLevels;
    int offset;
    int count;
);

/*
    srmLsResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmLsResponse,
    srm22_TReturnStatus*   returnStatus;
    char*  requestToken;
    srm22_ArrayOfTMetaDataPathDetail*   details;
);

/*
    srmStatusOfLsRequestRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmStatusOfLsRequestRequest,
    char*  authorizationID;
    char*  requestToken;
    int offset;
    int count;
);

/*
    srmStatusOfLsRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmStatusOfLsRequestResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTMetaDataPathDetail*   details;
);

/*
    srmMvRequest    
    is the 
    main-input
    and getSurl is for:
    fromSURL:anyURI! 
*/    
STRUCT_DEC_REQ(
        srmMvRequest,
    char*  authorizationID;
    char*  fromSURL;
    char*  toSURL;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
);

/*
    srmMvResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmMvResponse,
    srm22_TReturnStatus*   returnStatus;
);

/*
    srmPrepareToGetRequest    
    is the 
    main-input
    and getSurl is for:
    sourceSURL:anyURI!requestArray:TGetFileRequest->arrayOfFileRequests:ArrayOfTGetFileRequest-> 
*/    
STRUCT_DEC_REQ(
        srmPrepareToGetRequest,
    char*  authorizationID;
    srm22_ArrayOfTGetFileRequest*   arrayOfFileRequests;
    char*  userRequestDescription;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
    srm22_TFileStorageType desiredFileStorageType;
    int desiredTotalRequestTime;
    int desiredPinLifeTime;
    char*  targetSpaceToken;
    srm22_TRetentionPolicyInfo*   targetFileRetentionPolicyInfo;
    srm22_TTransferParameters*   transferParameters;
);

/*
    srmPrepareToGetResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmPrepareToGetResponse,
    srm22_TReturnStatus*   returnStatus;
    char*  requestToken;
    srm22_ArrayOfTGetRequestFileStatus*   arrayOfFileStatuses;
    int remainingTotalRequestTime;
);

/*
    srmStatusOfGetRequestRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSourceSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmStatusOfGetRequestRequest,
    char*  requestToken;
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSourceSURLs;
);

/*
    srmStatusOfGetRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmStatusOfGetRequestResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTGetRequestFileStatus*   arrayOfFileStatuses;
    int remainingTotalRequestTime;
);

/*
    srmBringOnlineRequest    
    is the 
    main-input
    and getSurl is for:
    sourceSURL:anyURI!requestArray:TGetFileRequest->arrayOfFileRequests:ArrayOfTGetFileRequest-> 
*/    
STRUCT_DEC_REQ(
        srmBringOnlineRequest,
    char*  authorizationID;
    srm22_ArrayOfTGetFileRequest*   arrayOfFileRequests;
    char*  userRequestDescription;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
    srm22_TFileStorageType desiredFileStorageType;
    int desiredTotalRequestTime;
    int desiredLifeTime;
    char*  targetSpaceToken;
    srm22_TRetentionPolicyInfo*   targetFileRetentionPolicyInfo;
    srm22_TTransferParameters*   transferParameters;
    int deferredStartTime;
);

/*
    srmBringOnlineResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmBringOnlineResponse,
    srm22_TReturnStatus*   returnStatus;
    char*  requestToken;
    srm22_ArrayOfTBringOnlineRequestFileStatus*   arrayOfFileStatuses;
    int remainingTotalRequestTime;
    int remainingDeferredStartTime;
);

/*
    srmStatusOfBringOnlineRequestRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSourceSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmStatusOfBringOnlineRequestRequest,
    char*  requestToken;
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSourceSURLs;
);

/*
    srmStatusOfBringOnlineRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmStatusOfBringOnlineRequestResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTBringOnlineRequestFileStatus*   arrayOfFileStatuses;
    int remainingTotalRequestTime;
    int remainingDeferredStartTime;
);

/*
    srmPrepareToPutRequest    
    is the 
    main-input
    and getSurl is for:
    targetSURL:anyURI!requestArray:TPutFileRequest->arrayOfFileRequests:ArrayOfTPutFileRequest-> 
*/    
STRUCT_DEC_REQ(
        srmPrepareToPutRequest,
    char*  authorizationID;
    srm22_ArrayOfTPutFileRequest*   arrayOfFileRequests;
    char*  userRequestDescription;
    srm22_TOverwriteMode overwriteOption;
    srm22_ArrayOfTExtraInfo*   storageSystemInfo;
    int desiredTotalRequestTime;
    int desiredPinLifeTime;
    int desiredFileLifeTime;
    srm22_TFileStorageType desiredFileStorageType;
    char*  targetSpaceToken;
    srm22_TRetentionPolicyInfo*   targetFileRetentionPolicyInfo;
    srm22_TTransferParameters*   transferParameters;
);

/*
    srmPrepareToPutResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmPrepareToPutResponse,
    srm22_TReturnStatus*   returnStatus;
    char*  requestToken;
    srm22_ArrayOfTPutRequestFileStatus*   arrayOfFileStatuses;
    int remainingTotalRequestTime;
);

/*
    srmStatusOfPutRequestRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfTargetSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmStatusOfPutRequestRequest,
    char*  requestToken;
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfTargetSURLs;
);

/*
    srmStatusOfPutRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmStatusOfPutRequestResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTPutRequestFileStatus*   arrayOfFileStatuses;
    int remainingTotalRequestTime;
);

/*
    srmCopyRequest    
    is the 
    main-input
    and getSurl is for:
    sourceSURL:anyURI!requestArray:TCopyFileRequest->arrayOfFileRequests:ArrayOfTCopyFileRequest-> 
*/    
STRUCT_DEC_REQ(
        srmCopyRequest,
    char*  authorizationID;
    srm22_ArrayOfTCopyFileRequest*   arrayOfFileRequests;
    char*  userRequestDescription;
    srm22_TOverwriteMode overwriteOption;
    int desiredTotalRequestTime;
    int desiredTargetSURLLifeTime;
    srm22_TFileStorageType targetFileStorageType;
    char*  targetSpaceToken;
    srm22_TRetentionPolicyInfo*   targetFileRetentionPolicyInfo;
    srm22_ArrayOfTExtraInfo*   sourceStorageSystemInfo;
    srm22_ArrayOfTExtraInfo*   targetStorageSystemInfo;
);

/*
    srmCopyResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmCopyResponse,
    srm22_TReturnStatus*   returnStatus;
    char*  requestToken;
    srm22_ArrayOfTCopyRequestFileStatus*   arrayOfFileStatuses;
    int remainingTotalRequestTime;
);

/*
    srmStatusOfCopyRequestRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSourceSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmStatusOfCopyRequestRequest,
    char*  requestToken;
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSourceSURLs;
    srm22_ArrayOfString*  arrayOfTargetSURLs;
);

/*
    srmStatusOfCopyRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmStatusOfCopyRequestResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTCopyRequestFileStatus*   arrayOfFileStatuses;
    int remainingTotalRequestTime;
);

/*
    srmReleaseFilesRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmReleaseFilesRequest,
    char*  requestToken;
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSURLs;
    short int doRemove;
);

/*
    srmReleaseFilesResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmReleaseFilesResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTSURLReturnStatus*   arrayOfFileStatuses;
);

/*
    srmPutDoneRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmPutDoneRequest,
    char*  requestToken;
    char*  authorizationID;
    srm22_ArrayOfString*  arrayOfSURLs;
);

/*
    srmPutDoneResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmPutDoneResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTSURLReturnStatus*   arrayOfFileStatuses;
);

/*
    srmAbortRequestRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmAbortRequestRequest,
    char*  requestToken;
    char*  authorizationID;
);

/*
    srmAbortRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmAbortRequestResponse,
    srm22_TReturnStatus*   returnStatus;
);

/*
    srmAbortFilesRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmAbortFilesRequest,
    char*  requestToken;
    srm22_ArrayOfString*  arrayOfSURLs;
    char*  authorizationID;
);

/*
    srmAbortFilesResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmAbortFilesResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTSURLReturnStatus*   arrayOfFileStatuses;
);

/*
    srmSuspendRequestRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmSuspendRequestRequest,
    char*  requestToken;
    char*  authorizationID;
);

/*
    srmSuspendRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmSuspendRequestResponse,
    srm22_TReturnStatus*   returnStatus;
);

/*
    srmResumeRequestRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmResumeRequestRequest,
    char*  requestToken;
    char*  authorizationID;
);

/*
    srmResumeRequestResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmResumeRequestResponse,
    srm22_TReturnStatus*   returnStatus;
);

/*
    srmGetRequestSummaryRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmGetRequestSummaryRequest,
    srm22_ArrayOfString*   arrayOfRequestTokens;
    char*  authorizationID;
);

/*
    srmGetRequestSummaryResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmGetRequestSummaryResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTRequestSummary*   arrayOfRequestSummaries;
);

/*
    srmExtendFileLifeTimeRequest    
    is the 
    main-input
    and getSurl is for:
    arrayOfSURLs:ArrayOfAnyURI! 
*/    
STRUCT_DEC_REQ(
        srmExtendFileLifeTimeRequest,
    char*  authorizationID;
    char*  requestToken;
    srm22_ArrayOfString*  arrayOfSURLs;
    int newFileLifeTime;
    int newPinLifeTime;
);

/*
    srmExtendFileLifeTimeResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmExtendFileLifeTimeResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTSURLLifetimeReturnStatus*   arrayOfFileStatuses;
);

/*
    srmGetRequestTokensRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmGetRequestTokensRequest,
    char*  userRequestDescription;
    char*  authorizationID;
);

/*
    srmGetRequestTokensResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmGetRequestTokensResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTRequestTokenReturn*   arrayOfRequestTokens;
);

/*
    srmGetTransferProtocolsRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmGetTransferProtocolsRequest,
    char*  authorizationID;
);

/*
    srmGetTransferProtocolsResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmGetTransferProtocolsResponse,
    srm22_TReturnStatus*   returnStatus;
    srm22_ArrayOfTSupportedTransferProtocol*   protocolInfo;
);

/*
    srmPingRequest    
    is the 
    main-input
*/    
STRUCT_DEC_IN(
        srmPingRequest,
    char*  authorizationID;
);

/*
    srmPingResponse    
    is the 
    main-output
*/
STRUCT_DEC_OUT(
         srmPingResponse,
    char*  versionInfo;
    srm22_ArrayOfTExtraInfo*   otherInfo;
);
/********** methods **********/
GFAL_WRAP_DEC(srmReserveSpace);        

GFAL_WRAP_DEC(srmStatusOfReserveSpaceRequest);        

GFAL_WRAP_DEC(srmReleaseSpace);        

GFAL_WRAP_DEC(srmUpdateSpace);        

GFAL_WRAP_DEC(srmStatusOfUpdateSpaceRequest);        

GFAL_WRAP_DEC(srmGetSpaceMetaData);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmChangeSpaceForFiles);        

GFAL_WRAP_DEC(srmStatusOfChangeSpaceForFilesRequest);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmExtendFileLifeTimeInSpace);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmPurgeFromSpace);        

GFAL_WRAP_DEC(srmGetSpaceTokens);        

/* path to surl is: SURL:anyURI! */     
GFAL_WRAP_DEC_SURL(srmSetPermission);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmCheckPermission);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmGetPermission);        

GFAL_WRAP_DEC(srmMkdir);        

GFAL_WRAP_DEC(srmRmdir);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmRm);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmLs);        

GFAL_WRAP_DEC(srmStatusOfLsRequest);        

/* path to surl is: fromSURL:anyURI! */     
GFAL_WRAP_DEC_SURL(srmMv);        

/* path to surl is: sourceSURL:anyURI!requestArray:TGetFileRequest->arrayOfFileRequests:ArrayOfTGetFileRequest-> */     
GFAL_WRAP_DEC_SURL(srmPrepareToGet);        

/* path to surl is: arrayOfSourceSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmStatusOfGetRequest);        

/* path to surl is: sourceSURL:anyURI!requestArray:TGetFileRequest->arrayOfFileRequests:ArrayOfTGetFileRequest-> */     
GFAL_WRAP_DEC_SURL(srmBringOnline);        

/* path to surl is: arrayOfSourceSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmStatusOfBringOnlineRequest);        

/* path to surl is: targetSURL:anyURI!requestArray:TPutFileRequest->arrayOfFileRequests:ArrayOfTPutFileRequest-> */     
GFAL_WRAP_DEC_SURL(srmPrepareToPut);        

/* path to surl is: arrayOfTargetSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmStatusOfPutRequest);        

/* path to surl is: sourceSURL:anyURI!requestArray:TCopyFileRequest->arrayOfFileRequests:ArrayOfTCopyFileRequest-> */     
GFAL_WRAP_DEC_SURL(srmCopy);        

/* path to surl is: arrayOfSourceSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmStatusOfCopyRequest);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmReleaseFiles);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmPutDone);        

GFAL_WRAP_DEC(srmAbortRequest);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmAbortFiles);        

GFAL_WRAP_DEC(srmSuspendRequest);        

GFAL_WRAP_DEC(srmResumeRequest);        

GFAL_WRAP_DEC(srmGetRequestSummary);        

/* path to surl is: arrayOfSURLs:ArrayOfAnyURI! */     
GFAL_WRAP_DEC_SURL(srmExtendFileLifeTime);        

GFAL_WRAP_DEC(srmGetRequestTokens);        

GFAL_WRAP_DEC(srmGetTransferProtocols);        

GFAL_WRAP_DEC(srmPing);        

        
#endif /*srm2_2_IFCE_H_ */

