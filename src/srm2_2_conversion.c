
        
#include "srm2_2_conversion.h"
#warning pass ctx to all functions and make wrapper for tc form the context or just set ctx error and return with null 

/*
* Conversion routines for gfal<->SOAP interoperability
*/

// helpers
#define SOAP_MAIN_ALLOC	\
if (!_elem) return NULL;	\
res = soap_malloc(soap,sizeof(*res));	\
if(!res) return NULL;	\
memset(res, 0, sizeof(*res));

#define SOAP_PTR_ALLOC(_name)	\
res->_name = soap_malloc(soap,sizeof(*res->_name));	\
if(!res->_name) return NULL;

#define SOAP_PTR_ARRAY_ALLOC(_name,_size)	\
res->_name = soap_malloc(soap,sizeof(*res->_name) * _elem->_size);	\
if(!res->_name) return NULL;

#define GFAL_PTR_ALLOC(_name)	\
res->_name = malloc(sizeof(*(res->_name)));	\
if(!res->_name) return NULL;	

// enum handling
#define DEF_GFALCONVENUM_DEF(_name)	\
enum srm22__ ## _name* convEnum2soap_srm22_ ## _name(struct soap* soap, int _res) \
{ 	enum srm22__ ## _name* res;	\
res = soap_malloc(soap,sizeof(*res));	\
if(!res) return NULL;	\
*res =(enum srm22__ ## _name)_res;	\
return res;	\
}


#define NEW_DATA(_p) _p = malloc(sizeof(*_p));


#define getSURL_DEF(_n,_ret)		\
TSURL get_##_n##_TSurl(_n* _arg){	\
if(!_arg) return NULL;		\
return (char*)_arg->_ret;			\
};

#define getARRSURL_DEF(_typeName,_arrName)	\
getSURL_DEC(ArrayOf##_typeName)	\
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


#define STRING_TO_SOAP(src, dest) 		\
{ 									\
dest = soap_strdup(soap, src); 	\
if (src && !dest) 				\
return NULL; 				\
} 

#define STRING_TO_SOAP_EMB(_name) \
{ \
	res->_name = soap_strdup(soap, _elem->_name); \
	if (_elem->_name && !res->_name) \
	return NULL; \
} 


#define NUM_TO_SOAP(f) \
res->f = _elem->f

#define NUM_TO_SOAP_EMB(f) \
res->f->value = _elem->f

#define STRING_FROM_SOAP(src, dest) \
dest = strdup(src);	\
if(!dest) return NULL;

#define STRING_FROM_SOAP_EMB(f) \
res->f = strdup(_elem->f);	\
if(!res->f) return NULL;


#define NUM_FROM_SOAP_EMB(f) \
res->f = _elem->f->value


#define NUM_FROM_SOAP(f) \
res->f = _elem->f;

#define NUM_FROM_SOAP_PTR(f) \
res->f = *(_elem->f);
    
    
#define CONV2SOAP_OBJ(_type,_name)	\
res->_name=conv2soap_##_type(soap,(_elem->_name));	

#define CONV2GFAL_OBJ(_type,_name)	\
res->_name=conv2gfal_##_type(_elem->_name);	


#define GEN_ARRAY_CONV2SOAP(_typeName,_sizeName,_arrName) 	\
DEF_GFALCONV_HEADERS_IN(ArrayOf##_typeName){    \
int i;	\
struct srm22__ArrayOf ## _typeName* res;	\
SOAP_MAIN_ALLOC;	\
res->_sizeName=_elem->_sizeName;	\
res->_arrName = soap_malloc(soap,res->_sizeName*sizeof(*(res->_arrName)));	\
if(_elem->_sizeName && !res->_arrName) return NULL;	\
for(i=0;i<_elem->_sizeName;i++)	\
{	\
res->_arrName[i]=conv2soap_##_typeName(soap,_elem->_arrName[i]);	\
if(!res->_arrName[i]) return NULL;	\
};    \
return res;} 

/*	
	i.e. GEN_ARRAY_CONV2SOAP_SIMPLE(ArayOfUnsignedLong,ULONG64,__sizeunsignedLongArray,unsignedLongArray); 
*/
	
#define GEN_ARRAY_CONV2SOAP_SIMPLE(_typeName,_subTypeName,_sizeName,_arrName) 	\
DEF_GFALCONV_HEADERS_IN(_typeName){    \
int i;	\
struct srm22__##_typeName* res;	\
SOAP_MAIN_ALLOC;	\
res->_sizeName=_elem->_sizeName;	\
res->_arrName = soap_malloc(soap,res->_sizeName*sizeof(*(res->_arrName)));	\
if(_elem->_sizeName && !res->_arrName) return NULL;	\
for(i=0;i<_elem->_sizeName;i++)	\
{	\
res->_arrName[i]=conv2soap_##_subTypeName(soap,_elem->_arrName[i]);	\
if(!res->_arrName[i]) return NULL;	\
};    \
return res;} 
	
	

#define GEN_ARRAY_CONV2SOAP2(_typeName,_sizeName,_arrName) 	\
int i;	\
struct srm22__ArrayOf ## _typeName* res;	\
SOAP_MAIN_ALLOC;	\
res->_sizeName=_elem->_sizeName;	\
res->_arrName = soap_malloc(soap,res->_sizeName*sizeof(*res->_arrName));	\
if(_elem->_sizeName && !res->_arrName) return NULL;	\
for(i=0;i<_elem->_sizeName;i++)	\
{	\
res->_arrName[i]=conv2soap_##_typeName(soap,(_typeName)&_elem[i]);	\
if(!res->_arrName[i]) return NULL;	\
};


#define GFAL_DECL_ALLOC(_name)	\
srm22_##_name* res;	\
if(!_elem) return NULL;	\
res = (srm22_##_name*) malloc (sizeof(srm22_##_name));	\
if(!res) return NULL;

#define GEN_ARRAY_CONV2GFAL(_typeName,_sizeName,_arrName)	\
DEF_GFALCONV_HEADERS_OUT(ArrayOf##_typeName){            \
    srm22_ArrayOf##_typeName* ret;					\
    int i;										\
    if(!_elem) return NULL;						\
        ret = malloc(sizeof(srm22_ArrayOf##_typeName));	\
        ret->_arrName = malloc(_elem->_sizeName*sizeof(*ret->_arrName));	\
        if ((_elem->_sizeName && !ret) || !ret->_arrName)				\
        {											\
        /* err_outofmemory(ctx);*/				\
        return NULL;							\
        }											\
    for(i = 0; i< _elem->_sizeName;i++)			\
    {											\
    if(_elem->_arrName[i])					\
    {										\
    ret->_arrName[i] = conv2gfal_srm22_##_typeName(_elem->_arrName[i]);	\
    if(!ret->_arrName[i])				\
    {									\
    ret->_sizeName = i+1;			\
    freegfalArray_srm22_ArrayOf##_typeName(ret);	\
    return NULL;					\
    }									\
    } 										\
    else ret->_arrName[i] = NULL;			\
    }											\
    ret->_sizeName = _elem->_sizeName;        \
    return ret;}	
    

#define GEN_ARRAY_CONV2GFAL_EXT(_typeName,_sizeName,_arrName,_typeNameIn,_sizeNameIn,_arrNameIn)	\
    ArrayOf##_typeName* ret;					\
    int i;										\
    if(!_elem) return NULL;						\
    ret = malloc(sizeof(ArrayOf##_typeName));	\
    ret->_arrName = malloc(_elem->_sizeNameIn*sizeof(*ret->_arrName));	\
    if ((_elem->_sizeNameIn && !ret) || !ret->_arrName)				\
    {											\
    /* err_outofmemory(ctx);*/				\
    return NULL;							\
    }											\
    for(i = 0; i< _elem->_sizeNameIn;i++)			\
    {											\
        if(_elem->_arrNameIn[i])					\
        {										\
        ret->_arrName[i] = conv2gfal_ ## _typeName ##_##_typeNameIn(_elem->_arrNameIn[i]);	\
        if(!ret->_arrName[i])				\
        {									\
            ret->_sizeName = i+1;			\
            freegfalArray_ArrayOf##_typeName(ret);	\
            return NULL;					\
        }									\
        } 										\
        else ret->_arrName[i] = NULL;			\
    }											\
    ret->_sizeName = _elem->_sizeNameIn;

	
	
#define GEN_ARRAY_CONV2GFAL_SIMPLE(_typeName,_subTypeName,_sizeName,_arrName)	\
DEF_GFALCONV_HEADERS_OUT(_typeName){            \
    srm22__typeName* ret;					\
    int i;										\
    if(!_elem) return NULL;						\
        ret = malloc(sizeof(srm22_##_typeName));	\
        ret->_arrName = malloc(_elem->_sizeName*sizeof(*ret->_arrName));	\
        if ((_elem->_sizeName && !ret) || !ret->_arrName)				\
        {											\
        /* err_outofmemory(ctx);*/				\
        return NULL;							\
        }											\
    for(i = 0; i< _elem->_sizeName;i++)			\
    {											\
    if(_elem->_arrName[i])					\
    {										\
    ret->_arrName[i] = conv2gfal_srm22_##_subTypeName(_elem->_arrName[i]);	\
    if(!ret->_arrName[i])				\
    {									\
    ret->_sizeName = i+1;			\
    freegfalArray_srm22_##_typeName(ret);	\
    return NULL;					\
    }									\
    } 										\
    else ret->_arrName[i] = NULL;			\
    }											\
    ret->_sizeName = _elem->_sizeName;        \
    return ret;}	
    



#define GFAL_FREEARRAY_TYPE_DEF(_typeName,_sizeName,_arrName)	\
void freegfalArray_ArrayOf##_typeName(ArrayOf##_typeName* _elem) 			\
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

#define GFAL_FREEARRAY(_name)					\
void freeArray_##_name(int i, _name** ptrArr) 	\
{	    for(;i>0;i--){if(ptrArr[i-1]) 				\
    free(ptrArr[i-1]);};}	

#define GFAL_FREEARRAY_CALL(_name,_i,_ptr)	freegfalArray_ArrayOf##_name(_ptr);

#define CONSTRUCTOR_ARRAY_DEF(_n,_Name)	\
CONSTRUCTOR_DEC(ArrayOf##_n,_n** _Name##Array ,int _size)	\
{												\
    ArrayOf##_n* tdata1;    					\
    tdata1 = malloc(sizeof(*tdata1));			\
    GCuAssertPtrNotNull(tdata1);				\
    tdata1->_Name##Array = malloc(sizeof(_n*)*_size);	\
    GCuAssertPtrNotNull(tdata1->_Name##Array);	\
    tdata1->__size##_Name##Array = _size;			\
    return tdata1;								\
}



#define ENUM_TO_SOAP(_TYPE,_n)    \
    res->_n=convEnum2soap_srm22_##_TYPE(soap,_elem->_n);
#define ENUM_TO_SOAP_NOPTR(_TYPE,_n)    \
	res->_n=*((enum srm22__##_TYPE*)(convEnum2soap_srm22_##_TYPE(soap,_elem->_n))); 
	
#define ENUM_FROM_SOAP(_n) res->_n = *(_elem->_n);
#define ENUM_FROM_SOAP_NOPTR(_n) res->_n = (_elem->_n);	
	
//////////////////////////////
#define NUM_TO_SOAP_PTR(_n) 	\
*(res->_n) = _elem->_n;
	


//in
ULONG64 conv2soap_srm22_UnsignedLong(struct soap *soap, ULONG64 _elem)
{
	return _elem; 
}	

char*  conv2soap_srm22_String(struct soap *soap, char* _elem)
{
	char * dest = soap_strdup(soap, _elem); 	
	if (!_elem || !dest)return NULL;
	return dest;
}	

//out
ULONG64 conv2gfal_srm22_UnsignedLong(ULONG64* _elem)
{
	return _elem;
}	
	
char* conv2gfal_srm22_String(char* _elem)
{
	char * dest = strdup( _elem); 	
	if (!_elem || !dest)return NULL;
	return dest;	
}	
	


            
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
            
            
DEF_GFALCONVENUM_DEF(TFileStorageType);
DEF_GFALCONVENUM_DEF(TFileType);
DEF_GFALCONVENUM_DEF(TRetentionPolicy);
DEF_GFALCONVENUM_DEF(TAccessLatency);
DEF_GFALCONVENUM_DEF(TPermissionMode);
DEF_GFALCONVENUM_DEF(TPermissionType);
DEF_GFALCONVENUM_DEF(TRequestType);
DEF_GFALCONVENUM_DEF(TOverwriteMode);
DEF_GFALCONVENUM_DEF(TFileLocality);
DEF_GFALCONVENUM_DEF(TAccessPattern);
DEF_GFALCONVENUM_DEF(TConnectionType);
DEF_GFALCONVENUM_DEF(TStatusCode);

/*
TRetentionPolicyInfo    
is the 
input/output type.
*/           
DEF_GFALCONV_HEADERS_IN(TRetentionPolicyInfo)    
{
	struct  srm22__TRetentionPolicyInfo* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	ENUM_TO_SOAP_NOPTR(TRetentionPolicy,retentionPolicy);
	ENUM_TO_SOAP(TAccessLatency,accessLatency);

	return res;                
}

DEF_GFALCONV_HEADERS_OUT(TRetentionPolicyInfo)
{
	GFAL_DECL_ALLOC(TRetentionPolicyInfo);
	ENUM_FROM_SOAP_NOPTR(retentionPolicy);  
	ENUM_FROM_SOAP(accessLatency);  
     
	return res;		
}                

/*
TUserPermission    
is the 
input/output type.
*/           
DEF_GFALCONV_HEADERS_IN(TUserPermission)    
{
	struct  srm22__TUserPermission* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(userID);
        	ENUM_TO_SOAP_NOPTR(TPermissionMode,mode);

	return res;                
}

DEF_GFALCONV_HEADERS_OUT(TUserPermission)
{
	GFAL_DECL_ALLOC(TUserPermission);
	STRING_FROM_SOAP_EMB(userID);
        	ENUM_FROM_SOAP_NOPTR(mode);  
     
	return res;		
}                
	
/*
    Array handling for:
    ArrayOfTUserPermission    
    is the 
    input/output
*/

GEN_ARRAY_CONV2SOAP(TUserPermission,__sizeuserPermissionArray, userPermissionArray);
GEN_ARRAY_CONV2GFAL(TUserPermission,__sizeuserPermissionArray, userPermissionArray);    

/*
TGroupPermission    
is the 
input/output type.
*/           
DEF_GFALCONV_HEADERS_IN(TGroupPermission)    
{
	struct  srm22__TGroupPermission* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(groupID);
        	ENUM_TO_SOAP_NOPTR(TPermissionMode,mode);

	return res;                
}

DEF_GFALCONV_HEADERS_OUT(TGroupPermission)
{
	GFAL_DECL_ALLOC(TGroupPermission);
	STRING_FROM_SOAP_EMB(groupID);
        	ENUM_FROM_SOAP_NOPTR(mode);  
     
	return res;		
}                
	
/*
    Array handling for:
    ArrayOfTGroupPermission    
    is the 
    input/output
*/

GEN_ARRAY_CONV2SOAP(TGroupPermission,__sizegroupPermissionArray, groupPermissionArray);
GEN_ARRAY_CONV2GFAL(TGroupPermission,__sizegroupPermissionArray, groupPermissionArray);    
	
/*
    Array handling for:
    ArrayOfUnsignedLong    
    is the 
    deep-input
*/

GEN_ARRAY_CONV2SOAP_SIMPLE(ArrayOfUnsignedLong,ULONG64,__sizeunsignedLongArray, unsignedLongArray);
	
/*
    Array handling for:
    ArrayOfString    
    is the 
    input/output
*/

GEN_ARRAY_CONV2SOAP_SIMPLE(ArrayOfString,string,__sizestringArray, stringArray);
GEN_ARRAY_CONV2GFAL_SIMPLE(ArrayOfString,string,__sizestringArray, stringArray);    

/*
TReturnStatus    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TReturnStatus)
{
	GFAL_DECL_ALLOC(TReturnStatus);
	ENUM_FROM_SOAP_NOPTR(statusCode);  
	STRING_FROM_SOAP_EMB(explanation);
             
	return res;
}

/*
TSURLReturnStatus    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TSURLReturnStatus)
{
	GFAL_DECL_ALLOC(TSURLReturnStatus);
	STRING_FROM_SOAP_EMB(surl);
        	CONV2GFAL_OBJ(TReturnStatus,status);
         
	return res;
}
	
/*
    Array handling for:
    ArrayOfTSURLReturnStatus    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TSURLReturnStatus,__sizestatusArray, statusArray);    

/*
TSURLLifetimeReturnStatus    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TSURLLifetimeReturnStatus)
{
	GFAL_DECL_ALLOC(TSURLLifetimeReturnStatus);
	STRING_FROM_SOAP_EMB(surl);
        	CONV2GFAL_OBJ(TReturnStatus,status);
    	NUM_FROM_SOAP_PTR(fileLifetime);
	NUM_FROM_SOAP_PTR(pinLifetime);
     
	return res;
}
	
/*
    Array handling for:
    ArrayOfTSURLLifetimeReturnStatus    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TSURLLifetimeReturnStatus,__sizestatusArray, statusArray);    

/*
TMetaDataPathDetail    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TMetaDataPathDetail)
{
	GFAL_DECL_ALLOC(TMetaDataPathDetail);
	STRING_FROM_SOAP_EMB(surl);
        	CONV2GFAL_OBJ(TReturnStatus,status);
    	NUM_FROM_SOAP_PTR(size);
	STRING_FROM_SOAP_EMB(createdAtTime);
        	STRING_FROM_SOAP_EMB(lastModificationTime);
        	ENUM_FROM_SOAP(fileStorageType);  
	CONV2GFAL_OBJ(TRetentionPolicyInfo,retentionPolicyInfo);
    	ENUM_FROM_SOAP(fileLocality);  
	CONV2GFAL_OBJ(ArrayOfString,arrayOfSpaceTokens);
    	ENUM_FROM_SOAP(type);  
	NUM_FROM_SOAP_PTR(lifetimeAssigned);
	NUM_FROM_SOAP_PTR(lifetimeLeft);
	CONV2GFAL_OBJ(TUserPermission,ownerPermission);
    	CONV2GFAL_OBJ(TGroupPermission,groupPermission);
    	ENUM_FROM_SOAP(otherPermission);  
	STRING_FROM_SOAP_EMB(checkSumType);
        	STRING_FROM_SOAP_EMB(checkSumValue);
        	CONV2GFAL_OBJ(ArrayOfTMetaDataPathDetail,arrayOfSubPaths);
         
	return res;
}
	
/*
    Array handling for:
    ArrayOfTMetaDataPathDetail    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TMetaDataPathDetail,__sizepathDetailArray, pathDetailArray);    

/*
TMetaDataSpace    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TMetaDataSpace)
{
	GFAL_DECL_ALLOC(TMetaDataSpace);
	STRING_FROM_SOAP_EMB(spaceToken);
        	CONV2GFAL_OBJ(TReturnStatus,status);
    	CONV2GFAL_OBJ(TRetentionPolicyInfo,retentionPolicyInfo);
    	STRING_FROM_SOAP_EMB(owner);
        	NUM_FROM_SOAP_PTR(totalSize);
	NUM_FROM_SOAP_PTR(guaranteedSize);
	NUM_FROM_SOAP_PTR(unusedSize);
	NUM_FROM_SOAP_PTR(lifetimeAssigned);
	NUM_FROM_SOAP_PTR(lifetimeLeft);
     
	return res;
}
	
/*
    Array handling for:
    ArrayOfTMetaDataSpace    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TMetaDataSpace,__sizespaceDataArray, spaceDataArray);    

/*
TDirOption    
is the 
deep-input type.
*/           
DEF_GFALCONV_HEADERS_IN(TDirOption)
{
	struct  srm22__TDirOption* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
    NUM_TO_SOAP(isSourceADirectory);                
	SOAP_PTR_ALLOC(allLevelRecursive);
    	NUM_TO_SOAP_PTR(allLevelRecursive);                
	SOAP_PTR_ALLOC(numOfLevels);
    	NUM_TO_SOAP_PTR(numOfLevels);                

	return res;                
}

/*
TExtraInfo    
is the 
input/output type.
*/           
DEF_GFALCONV_HEADERS_IN(TExtraInfo)    
{
	struct  srm22__TExtraInfo* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(key);
        	SOAP_PTR_ALLOC(value);
    	STRING_TO_SOAP_EMB(value);
        
	return res;                
}

DEF_GFALCONV_HEADERS_OUT(TExtraInfo)
{
	GFAL_DECL_ALLOC(TExtraInfo);
	STRING_FROM_SOAP_EMB(key);
        	STRING_FROM_SOAP_EMB(value);
             
	return res;		
}                
	
/*
    Array handling for:
    ArrayOfTExtraInfo    
    is the 
    input/output
*/

GEN_ARRAY_CONV2SOAP(TExtraInfo,__sizeextraInfoArray, extraInfoArray);
GEN_ARRAY_CONV2GFAL(TExtraInfo,__sizeextraInfoArray, extraInfoArray);    
	
/*
    Array handling for:
    ArrayOfAnyURI    
    is the 
    unknown
*/

/*
TTransferParameters    
is the 
deep-input type.
*/           
DEF_GFALCONV_HEADERS_IN(TTransferParameters)
{
	struct  srm22__TTransferParameters* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	ENUM_TO_SOAP(TAccessPattern,accessPattern);
	ENUM_TO_SOAP(TConnectionType,connectionType);
	CONV2SOAP_OBJ(ArrayOfString,arrayOfClientNetworks);
        	CONV2SOAP_OBJ(ArrayOfString,arrayOfTransferProtocols);
        
	return res;                
}

/*
TGetFileRequest    
is the 
deep-input type.
*/           
DEF_GFALCONV_HEADERS_IN(TGetFileRequest)
{
	struct  srm22__TGetFileRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(sourceSURL);
        	CONV2SOAP_OBJ(TDirOption,dirOption);
        
	return res;                
}
	
/*
    Array handling for:
    ArrayOfTGetFileRequest    
    is the 
    deep-input
*/

GEN_ARRAY_CONV2SOAP(TGetFileRequest,__sizerequestArray, requestArray);

/*
TPutFileRequest    
is the 
deep-input type.
*/           
DEF_GFALCONV_HEADERS_IN(TPutFileRequest)
{
	struct  srm22__TPutFileRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(targetSURL);
    	STRING_TO_SOAP_EMB(targetSURL);
        	SOAP_PTR_ALLOC(expectedFileSize);
    	NUM_TO_SOAP_PTR(expectedFileSize);                

	return res;                
}
	
/*
    Array handling for:
    ArrayOfTPutFileRequest    
    is the 
    deep-input
*/

GEN_ARRAY_CONV2SOAP(TPutFileRequest,__sizerequestArray, requestArray);

/*
TCopyFileRequest    
is the 
deep-input type.
*/           
DEF_GFALCONV_HEADERS_IN(TCopyFileRequest)
{
	struct  srm22__TCopyFileRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(sourceSURL);
        	STRING_TO_SOAP_EMB(targetSURL);
        	CONV2SOAP_OBJ(TDirOption,dirOption);
        
	return res;                
}
	
/*
    Array handling for:
    ArrayOfTCopyFileRequest    
    is the 
    deep-input
*/

GEN_ARRAY_CONV2SOAP(TCopyFileRequest,__sizerequestArray, requestArray);

/*
TGetRequestFileStatus    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TGetRequestFileStatus)
{
	GFAL_DECL_ALLOC(TGetRequestFileStatus);
	STRING_FROM_SOAP_EMB(sourceSURL);
        	NUM_FROM_SOAP_PTR(fileSize);
	CONV2GFAL_OBJ(TReturnStatus,status);
    	NUM_FROM_SOAP_PTR(estimatedWaitTime);
	NUM_FROM_SOAP_PTR(remainingPinTime);
	STRING_FROM_SOAP_EMB(transferURL);
        	CONV2GFAL_OBJ(ArrayOfTExtraInfo,transferProtocolInfo);
         
	return res;
}
	
/*
    Array handling for:
    ArrayOfTGetRequestFileStatus    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TGetRequestFileStatus,__sizestatusArray, statusArray);    

/*
TBringOnlineRequestFileStatus    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TBringOnlineRequestFileStatus)
{
	GFAL_DECL_ALLOC(TBringOnlineRequestFileStatus);
	STRING_FROM_SOAP_EMB(sourceSURL);
        	CONV2GFAL_OBJ(TReturnStatus,status);
    	NUM_FROM_SOAP_PTR(fileSize);
	NUM_FROM_SOAP_PTR(estimatedWaitTime);
	NUM_FROM_SOAP_PTR(remainingPinTime);
     
	return res;
}
	
/*
    Array handling for:
    ArrayOfTBringOnlineRequestFileStatus    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TBringOnlineRequestFileStatus,__sizestatusArray, statusArray);    

/*
TPutRequestFileStatus    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TPutRequestFileStatus)
{
	GFAL_DECL_ALLOC(TPutRequestFileStatus);
	STRING_FROM_SOAP_EMB(SURL);
        	CONV2GFAL_OBJ(TReturnStatus,status);
    	NUM_FROM_SOAP_PTR(fileSize);
	NUM_FROM_SOAP_PTR(estimatedWaitTime);
	NUM_FROM_SOAP_PTR(remainingPinLifetime);
	NUM_FROM_SOAP_PTR(remainingFileLifetime);
	STRING_FROM_SOAP_EMB(transferURL);
        	CONV2GFAL_OBJ(ArrayOfTExtraInfo,transferProtocolInfo);
         
	return res;
}
	
/*
    Array handling for:
    ArrayOfTPutRequestFileStatus    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TPutRequestFileStatus,__sizestatusArray, statusArray);    

/*
TCopyRequestFileStatus    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TCopyRequestFileStatus)
{
	GFAL_DECL_ALLOC(TCopyRequestFileStatus);
	STRING_FROM_SOAP_EMB(sourceSURL);
        	STRING_FROM_SOAP_EMB(targetSURL);
        	CONV2GFAL_OBJ(TReturnStatus,status);
    	NUM_FROM_SOAP_PTR(fileSize);
	NUM_FROM_SOAP_PTR(estimatedWaitTime);
	NUM_FROM_SOAP_PTR(remainingFileLifetime);
     
	return res;
}
	
/*
    Array handling for:
    ArrayOfTCopyRequestFileStatus    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TCopyRequestFileStatus,__sizestatusArray, statusArray);    

/*
TRequestSummary    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TRequestSummary)
{
	GFAL_DECL_ALLOC(TRequestSummary);
	STRING_FROM_SOAP_EMB(requestToken);
        	CONV2GFAL_OBJ(TReturnStatus,status);
    	ENUM_FROM_SOAP(requestType);  
	NUM_FROM_SOAP_PTR(totalNumFilesInRequest);
	NUM_FROM_SOAP_PTR(numOfCompletedFiles);
	NUM_FROM_SOAP_PTR(numOfWaitingFiles);
	NUM_FROM_SOAP_PTR(numOfFailedFiles);
     
	return res;
}
	
/*
    Array handling for:
    ArrayOfTRequestSummary    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TRequestSummary,__sizesummaryArray, summaryArray);    

/*
TSURLPermissionReturn    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TSURLPermissionReturn)
{
	GFAL_DECL_ALLOC(TSURLPermissionReturn);
	STRING_FROM_SOAP_EMB(surl);
        	CONV2GFAL_OBJ(TReturnStatus,status);
    	ENUM_FROM_SOAP(permission);  
     
	return res;
}
	
/*
    Array handling for:
    ArrayOfTSURLPermissionReturn    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TSURLPermissionReturn,__sizesurlPermissionArray, surlPermissionArray);    

/*
TPermissionReturn    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TPermissionReturn)
{
	GFAL_DECL_ALLOC(TPermissionReturn);
	STRING_FROM_SOAP_EMB(surl);
        	CONV2GFAL_OBJ(TReturnStatus,status);
    	STRING_FROM_SOAP_EMB(owner);
        	ENUM_FROM_SOAP(ownerPermission);  
	CONV2GFAL_OBJ(ArrayOfTUserPermission,arrayOfUserPermissions);
    	CONV2GFAL_OBJ(ArrayOfTGroupPermission,arrayOfGroupPermissions);
    	ENUM_FROM_SOAP(otherPermission);  
     
	return res;
}
	
/*
    Array handling for:
    ArrayOfTPermissionReturn    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TPermissionReturn,__sizepermissionArray, permissionArray);    

/*
TRequestTokenReturn    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TRequestTokenReturn)
{
	GFAL_DECL_ALLOC(TRequestTokenReturn);
	STRING_FROM_SOAP_EMB(requestToken);
        	STRING_FROM_SOAP_EMB(createdAtTime);
             
	return res;
}
	
/*
    Array handling for:
    ArrayOfTRequestTokenReturn    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TRequestTokenReturn,__sizetokenArray, tokenArray);    

/*
TSupportedTransferProtocol    
is the 
deep-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(TSupportedTransferProtocol)
{
	GFAL_DECL_ALLOC(TSupportedTransferProtocol);
	STRING_FROM_SOAP_EMB(transferProtocol);
        	CONV2GFAL_OBJ(ArrayOfTExtraInfo,attributes);
         
	return res;
}
	
/*
    Array handling for:
    ArrayOfTSupportedTransferProtocol    
    is the 
    deep-output
*/

GEN_ARRAY_CONV2GFAL(TSupportedTransferProtocol,__sizeprotocolArray, protocolArray);    

/*
srmReserveSpaceRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmReserveSpaceRequest)
{
	struct  srm22__srmReserveSpaceRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	SOAP_PTR_ALLOC(userSpaceTokenDescription);
    	STRING_TO_SOAP_EMB(userSpaceTokenDescription);
        	CONV2SOAP_OBJ(TRetentionPolicyInfo,retentionPolicyInfo);
        	SOAP_PTR_ALLOC(desiredSizeOfTotalSpace);
    	NUM_TO_SOAP_PTR(desiredSizeOfTotalSpace);                
    NUM_TO_SOAP(desiredSizeOfGuaranteedSpace);                
	SOAP_PTR_ALLOC(desiredLifetimeOfReservedSpace);
    	NUM_TO_SOAP_PTR(desiredLifetimeOfReservedSpace);                
	CONV2SOAP_OBJ(ArrayOfUnsignedLong,arrayOfExpectedFileSizes);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        	CONV2SOAP_OBJ(TTransferParameters,transferParameters);
        
	return res;                
}

/*
srmReserveSpaceResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmReserveSpaceResponse)
{
	GFAL_DECL_ALLOC(srmReserveSpaceResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	STRING_FROM_SOAP_EMB(requestToken);
        	NUM_FROM_SOAP_PTR(estimatedProcessingTime);
	CONV2GFAL_OBJ(TRetentionPolicyInfo,retentionPolicyInfo);
    	NUM_FROM_SOAP_PTR(sizeOfTotalReservedSpace);
	NUM_FROM_SOAP_PTR(sizeOfGuaranteedReservedSpace);
	NUM_FROM_SOAP_PTR(lifetimeOfReservedSpace);
	STRING_FROM_SOAP_EMB(spaceToken);
             
	return res;
}

/*
srmStatusOfReserveSpaceRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmStatusOfReserveSpaceRequestRequest)
{
	struct  srm22__srmStatusOfReserveSpaceRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(requestToken);
        
	return res;                
}

/*
srmStatusOfReserveSpaceRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmStatusOfReserveSpaceRequestResponse)
{
	GFAL_DECL_ALLOC(srmStatusOfReserveSpaceRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	NUM_FROM_SOAP_PTR(estimatedProcessingTime);
	CONV2GFAL_OBJ(TRetentionPolicyInfo,retentionPolicyInfo);
    	NUM_FROM_SOAP_PTR(sizeOfTotalReservedSpace);
	NUM_FROM_SOAP_PTR(sizeOfGuaranteedReservedSpace);
	NUM_FROM_SOAP_PTR(lifetimeOfReservedSpace);
	STRING_FROM_SOAP_EMB(spaceToken);
             
	return res;
}

/*
srmReleaseSpaceRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmReleaseSpaceRequest)
{
	struct  srm22__srmReleaseSpaceRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(spaceToken);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        	SOAP_PTR_ALLOC(forceFileRelease);
    	NUM_TO_SOAP_PTR(forceFileRelease);                

	return res;                
}

/*
srmReleaseSpaceResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmReleaseSpaceResponse)
{
	GFAL_DECL_ALLOC(srmReleaseSpaceResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
         
	return res;
}

/*
srmUpdateSpaceRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmUpdateSpaceRequest)
{
	struct  srm22__srmUpdateSpaceRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(spaceToken);
        	SOAP_PTR_ALLOC(newSizeOfTotalSpaceDesired);
    	NUM_TO_SOAP_PTR(newSizeOfTotalSpaceDesired);                
	SOAP_PTR_ALLOC(newSizeOfGuaranteedSpaceDesired);
    	NUM_TO_SOAP_PTR(newSizeOfGuaranteedSpaceDesired);                
	SOAP_PTR_ALLOC(newLifeTime);
    	NUM_TO_SOAP_PTR(newLifeTime);                
	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        
	return res;                
}

/*
srmUpdateSpaceResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmUpdateSpaceResponse)
{
	GFAL_DECL_ALLOC(srmUpdateSpaceResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	STRING_FROM_SOAP_EMB(requestToken);
        	NUM_FROM_SOAP_PTR(sizeOfTotalSpace);
	NUM_FROM_SOAP_PTR(sizeOfGuaranteedSpace);
	NUM_FROM_SOAP_PTR(lifetimeGranted);
     
	return res;
}

/*
srmStatusOfUpdateSpaceRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmStatusOfUpdateSpaceRequestRequest)
{
	struct  srm22__srmStatusOfUpdateSpaceRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(requestToken);
        
	return res;                
}

/*
srmStatusOfUpdateSpaceRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmStatusOfUpdateSpaceRequestResponse)
{
	GFAL_DECL_ALLOC(srmStatusOfUpdateSpaceRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	NUM_FROM_SOAP_PTR(sizeOfTotalSpace);
	NUM_FROM_SOAP_PTR(sizeOfGuaranteedSpace);
	NUM_FROM_SOAP_PTR(lifetimeGranted);
     
	return res;
}

/*
srmGetSpaceMetaDataRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmGetSpaceMetaDataRequest)
{
	struct  srm22__srmGetSpaceMetaDataRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfString,arrayOfSpaceTokens);
        
	return res;                
}

/*
srmGetSpaceMetaDataResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmGetSpaceMetaDataResponse)
{
	GFAL_DECL_ALLOC(srmGetSpaceMetaDataResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTMetaDataSpace,arrayOfSpaceDetails);
         
	return res;
}

/*
srmChangeSpaceForFilesRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmChangeSpaceForFilesRequest)
{
	struct  srm22__srmChangeSpaceForFilesRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	STRING_TO_SOAP_EMB(targetSpaceToken);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        
	return res;                
}

/*
srmChangeSpaceForFilesResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmChangeSpaceForFilesResponse)
{
	GFAL_DECL_ALLOC(srmChangeSpaceForFilesResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	STRING_FROM_SOAP_EMB(requestToken);
        	NUM_FROM_SOAP_PTR(estimatedProcessingTime);
	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
         
	return res;
}

/*
srmStatusOfChangeSpaceForFilesRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmStatusOfChangeSpaceForFilesRequestRequest)
{
	struct  srm22__srmStatusOfChangeSpaceForFilesRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(requestToken);
        
	return res;                
}

/*
srmStatusOfChangeSpaceForFilesRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmStatusOfChangeSpaceForFilesRequestResponse)
{
	GFAL_DECL_ALLOC(srmStatusOfChangeSpaceForFilesRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	NUM_FROM_SOAP_PTR(estimatedProcessingTime);
	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
         
	return res;
}

/*
srmExtendFileLifeTimeInSpaceRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmExtendFileLifeTimeInSpaceRequest)
{
	struct  srm22__srmExtendFileLifeTimeInSpaceRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(spaceToken);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	SOAP_PTR_ALLOC(newLifeTime);
    	NUM_TO_SOAP_PTR(newLifeTime);                

	return res;                
}

/*
srmExtendFileLifeTimeInSpaceResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmExtendFileLifeTimeInSpaceResponse)
{
	GFAL_DECL_ALLOC(srmExtendFileLifeTimeInSpaceResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	NUM_FROM_SOAP_PTR(newTimeExtended);
	CONV2GFAL_OBJ(ArrayOfTSURLLifetimeReturnStatus,arrayOfFileStatuses);
         
	return res;
}

/*
srmPurgeFromSpaceRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmPurgeFromSpaceRequest)
{
	struct  srm22__srmPurgeFromSpaceRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	STRING_TO_SOAP_EMB(spaceToken);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        
	return res;                
}

/*
srmPurgeFromSpaceResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmPurgeFromSpaceResponse)
{
	GFAL_DECL_ALLOC(srmPurgeFromSpaceResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
         
	return res;
}

/*
srmGetSpaceTokensRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmGetSpaceTokensRequest)
{
	struct  srm22__srmGetSpaceTokensRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(userSpaceTokenDescription);
    	STRING_TO_SOAP_EMB(userSpaceTokenDescription);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmGetSpaceTokensResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmGetSpaceTokensResponse)
{
	GFAL_DECL_ALLOC(srmGetSpaceTokensResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfString,arrayOfSpaceTokens);
         
	return res;
}

/*
srmSetPermissionRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmSetPermissionRequest)
{
	struct  srm22__srmSetPermissionRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(SURL);
        	ENUM_TO_SOAP_NOPTR(TPermissionType,permissionType);
	ENUM_TO_SOAP(TPermissionMode,ownerPermission);
	CONV2SOAP_OBJ(ArrayOfTUserPermission,arrayOfUserPermissions);
        	CONV2SOAP_OBJ(ArrayOfTGroupPermission,arrayOfGroupPermissions);
        	ENUM_TO_SOAP(TPermissionMode,otherPermission);
	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        
	return res;                
}

/*
srmSetPermissionResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmSetPermissionResponse)
{
	GFAL_DECL_ALLOC(srmSetPermissionResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
         
	return res;
}

/*
srmCheckPermissionRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmCheckPermissionRequest)
{
	struct  srm22__srmCheckPermissionRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        
	return res;                
}

/*
srmCheckPermissionResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmCheckPermissionResponse)
{
	GFAL_DECL_ALLOC(srmCheckPermissionResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTSURLPermissionReturn,arrayOfPermissions);
         
	return res;
}

/*
srmGetPermissionRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmGetPermissionRequest)
{
	struct  srm22__srmGetPermissionRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        
	return res;                
}

/*
srmGetPermissionResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmGetPermissionResponse)
{
	GFAL_DECL_ALLOC(srmGetPermissionResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTPermissionReturn,arrayOfPermissionReturns);
         
	return res;
}

/*
srmMkdirRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmMkdirRequest)
{
	struct  srm22__srmMkdirRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(directoryPath);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        
	return res;                
}

/*
srmMkdirResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmMkdirResponse)
{
	GFAL_DECL_ALLOC(srmMkdirResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
         
	return res;
}

/*
srmRmdirRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmRmdirRequest)
{
	struct  srm22__srmRmdirRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(directoryPath);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        	SOAP_PTR_ALLOC(recursive);
    	NUM_TO_SOAP_PTR(recursive);                

	return res;                
}

/*
srmRmdirResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmRmdirResponse)
{
	GFAL_DECL_ALLOC(srmRmdirResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
         
	return res;
}

/*
srmRmRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmRmRequest)
{
	struct  srm22__srmRmRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        
	return res;                
}

/*
srmRmResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmRmResponse)
{
	GFAL_DECL_ALLOC(srmRmResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
         
	return res;
}

/*
srmLsRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmLsRequest)
{
	struct  srm22__srmLsRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        	ENUM_TO_SOAP(TFileStorageType,fileStorageType);
	SOAP_PTR_ALLOC(fullDetailedList);
    	NUM_TO_SOAP_PTR(fullDetailedList);                
	SOAP_PTR_ALLOC(allLevelRecursive);
    	NUM_TO_SOAP_PTR(allLevelRecursive);                
	SOAP_PTR_ALLOC(numOfLevels);
    	NUM_TO_SOAP_PTR(numOfLevels);                
	SOAP_PTR_ALLOC(offset);
    	NUM_TO_SOAP_PTR(offset);                
	SOAP_PTR_ALLOC(count);
    	NUM_TO_SOAP_PTR(count);                

	return res;                
}

/*
srmLsResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmLsResponse)
{
	GFAL_DECL_ALLOC(srmLsResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	STRING_FROM_SOAP_EMB(requestToken);
        	CONV2GFAL_OBJ(ArrayOfTMetaDataPathDetail,details);
         
	return res;
}

/*
srmStatusOfLsRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmStatusOfLsRequestRequest)
{
	struct  srm22__srmStatusOfLsRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(requestToken);
            NUM_TO_SOAP(offset);                
	SOAP_PTR_ALLOC(count);
    	NUM_TO_SOAP_PTR(count);                

	return res;                
}

/*
srmStatusOfLsRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmStatusOfLsRequestResponse)
{
	GFAL_DECL_ALLOC(srmStatusOfLsRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTMetaDataPathDetail,details);
         
	return res;
}

/*
srmMvRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmMvRequest)
{
	struct  srm22__srmMvRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(fromSURL);
        	STRING_TO_SOAP_EMB(toSURL);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        
	return res;                
}

/*
srmMvResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmMvResponse)
{
	GFAL_DECL_ALLOC(srmMvResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
         
	return res;
}

/*
srmPrepareToGetRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmPrepareToGetRequest)
{
	struct  srm22__srmPrepareToGetRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfTGetFileRequest,arrayOfFileRequests);
        	SOAP_PTR_ALLOC(userRequestDescription);
    	STRING_TO_SOAP_EMB(userRequestDescription);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        	ENUM_TO_SOAP(TFileStorageType,desiredFileStorageType);
	SOAP_PTR_ALLOC(desiredTotalRequestTime);
    	NUM_TO_SOAP_PTR(desiredTotalRequestTime);                
	SOAP_PTR_ALLOC(desiredPinLifeTime);
    	NUM_TO_SOAP_PTR(desiredPinLifeTime);                
	SOAP_PTR_ALLOC(targetSpaceToken);
    	STRING_TO_SOAP_EMB(targetSpaceToken);
        	CONV2SOAP_OBJ(TRetentionPolicyInfo,targetFileRetentionPolicyInfo);
        	CONV2SOAP_OBJ(TTransferParameters,transferParameters);
        
	return res;                
}

/*
srmPrepareToGetResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmPrepareToGetResponse)
{
	GFAL_DECL_ALLOC(srmPrepareToGetResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	STRING_FROM_SOAP_EMB(requestToken);
        	CONV2GFAL_OBJ(ArrayOfTGetRequestFileStatus,arrayOfFileStatuses);
    	NUM_FROM_SOAP_PTR(remainingTotalRequestTime);
     
	return res;
}

/*
srmStatusOfGetRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmStatusOfGetRequestRequest)
{
	struct  srm22__srmStatusOfGetRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSourceSURLs);
        
	return res;                
}

/*
srmStatusOfGetRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmStatusOfGetRequestResponse)
{
	GFAL_DECL_ALLOC(srmStatusOfGetRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTGetRequestFileStatus,arrayOfFileStatuses);
    	NUM_FROM_SOAP_PTR(remainingTotalRequestTime);
     
	return res;
}

/*
srmBringOnlineRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmBringOnlineRequest)
{
	struct  srm22__srmBringOnlineRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfTGetFileRequest,arrayOfFileRequests);
        	SOAP_PTR_ALLOC(userRequestDescription);
    	STRING_TO_SOAP_EMB(userRequestDescription);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        	ENUM_TO_SOAP(TFileStorageType,desiredFileStorageType);
	SOAP_PTR_ALLOC(desiredTotalRequestTime);
    	NUM_TO_SOAP_PTR(desiredTotalRequestTime);                
	SOAP_PTR_ALLOC(desiredLifeTime);
    	NUM_TO_SOAP_PTR(desiredLifeTime);                
	SOAP_PTR_ALLOC(targetSpaceToken);
    	STRING_TO_SOAP_EMB(targetSpaceToken);
        	CONV2SOAP_OBJ(TRetentionPolicyInfo,targetFileRetentionPolicyInfo);
        	CONV2SOAP_OBJ(TTransferParameters,transferParameters);
        	SOAP_PTR_ALLOC(deferredStartTime);
    	NUM_TO_SOAP_PTR(deferredStartTime);                

	return res;                
}

/*
srmBringOnlineResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmBringOnlineResponse)
{
	GFAL_DECL_ALLOC(srmBringOnlineResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	STRING_FROM_SOAP_EMB(requestToken);
        	CONV2GFAL_OBJ(ArrayOfTBringOnlineRequestFileStatus,arrayOfFileStatuses);
    	NUM_FROM_SOAP_PTR(remainingTotalRequestTime);
	NUM_FROM_SOAP_PTR(remainingDeferredStartTime);
     
	return res;
}

/*
srmStatusOfBringOnlineRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmStatusOfBringOnlineRequestRequest)
{
	struct  srm22__srmStatusOfBringOnlineRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSourceSURLs);
        
	return res;                
}

/*
srmStatusOfBringOnlineRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmStatusOfBringOnlineRequestResponse)
{
	GFAL_DECL_ALLOC(srmStatusOfBringOnlineRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTBringOnlineRequestFileStatus,arrayOfFileStatuses);
    	NUM_FROM_SOAP_PTR(remainingTotalRequestTime);
	NUM_FROM_SOAP_PTR(remainingDeferredStartTime);
     
	return res;
}

/*
srmPrepareToPutRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmPrepareToPutRequest)
{
	struct  srm22__srmPrepareToPutRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfTPutFileRequest,arrayOfFileRequests);
        	SOAP_PTR_ALLOC(userRequestDescription);
    	STRING_TO_SOAP_EMB(userRequestDescription);
        	ENUM_TO_SOAP(TOverwriteMode,overwriteOption);
	CONV2SOAP_OBJ(ArrayOfTExtraInfo,storageSystemInfo);
        	SOAP_PTR_ALLOC(desiredTotalRequestTime);
    	NUM_TO_SOAP_PTR(desiredTotalRequestTime);                
	SOAP_PTR_ALLOC(desiredPinLifeTime);
    	NUM_TO_SOAP_PTR(desiredPinLifeTime);                
	SOAP_PTR_ALLOC(desiredFileLifeTime);
    	NUM_TO_SOAP_PTR(desiredFileLifeTime);                
	ENUM_TO_SOAP(TFileStorageType,desiredFileStorageType);
	SOAP_PTR_ALLOC(targetSpaceToken);
    	STRING_TO_SOAP_EMB(targetSpaceToken);
        	CONV2SOAP_OBJ(TRetentionPolicyInfo,targetFileRetentionPolicyInfo);
        	CONV2SOAP_OBJ(TTransferParameters,transferParameters);
        
	return res;                
}

/*
srmPrepareToPutResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmPrepareToPutResponse)
{
	GFAL_DECL_ALLOC(srmPrepareToPutResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	STRING_FROM_SOAP_EMB(requestToken);
        	CONV2GFAL_OBJ(ArrayOfTPutRequestFileStatus,arrayOfFileStatuses);
    	NUM_FROM_SOAP_PTR(remainingTotalRequestTime);
     
	return res;
}

/*
srmStatusOfPutRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmStatusOfPutRequestRequest)
{
	struct  srm22__srmStatusOfPutRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfTargetSURLs);
        
	return res;                
}

/*
srmStatusOfPutRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmStatusOfPutRequestResponse)
{
	GFAL_DECL_ALLOC(srmStatusOfPutRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTPutRequestFileStatus,arrayOfFileStatuses);
    	NUM_FROM_SOAP_PTR(remainingTotalRequestTime);
     
	return res;
}

/*
srmCopyRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmCopyRequest)
{
	struct  srm22__srmCopyRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfTCopyFileRequest,arrayOfFileRequests);
        	SOAP_PTR_ALLOC(userRequestDescription);
    	STRING_TO_SOAP_EMB(userRequestDescription);
        	ENUM_TO_SOAP(TOverwriteMode,overwriteOption);
	SOAP_PTR_ALLOC(desiredTotalRequestTime);
    	NUM_TO_SOAP_PTR(desiredTotalRequestTime);                
	SOAP_PTR_ALLOC(desiredTargetSURLLifeTime);
    	NUM_TO_SOAP_PTR(desiredTargetSURLLifeTime);                
	ENUM_TO_SOAP(TFileStorageType,targetFileStorageType);
	SOAP_PTR_ALLOC(targetSpaceToken);
    	STRING_TO_SOAP_EMB(targetSpaceToken);
        	CONV2SOAP_OBJ(TRetentionPolicyInfo,targetFileRetentionPolicyInfo);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,sourceStorageSystemInfo);
        	CONV2SOAP_OBJ(ArrayOfTExtraInfo,targetStorageSystemInfo);
        
	return res;                
}

/*
srmCopyResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmCopyResponse)
{
	GFAL_DECL_ALLOC(srmCopyResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	STRING_FROM_SOAP_EMB(requestToken);
        	CONV2GFAL_OBJ(ArrayOfTCopyRequestFileStatus,arrayOfFileStatuses);
    	NUM_FROM_SOAP_PTR(remainingTotalRequestTime);
     
	return res;
}

/*
srmStatusOfCopyRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmStatusOfCopyRequestRequest)
{
	struct  srm22__srmStatusOfCopyRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSourceSURLs);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfTargetSURLs);
        
	return res;                
}

/*
srmStatusOfCopyRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmStatusOfCopyRequestResponse)
{
	GFAL_DECL_ALLOC(srmStatusOfCopyRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTCopyRequestFileStatus,arrayOfFileStatuses);
    	NUM_FROM_SOAP_PTR(remainingTotalRequestTime);
     
	return res;
}

/*
srmReleaseFilesRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmReleaseFilesRequest)
{
	struct  srm22__srmReleaseFilesRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(requestToken);
    	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	SOAP_PTR_ALLOC(doRemove);
    	NUM_TO_SOAP_PTR(doRemove);                

	return res;                
}

/*
srmReleaseFilesResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmReleaseFilesResponse)
{
	GFAL_DECL_ALLOC(srmReleaseFilesResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
         
	return res;
}

/*
srmPutDoneRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmPutDoneRequest)
{
	struct  srm22__srmPutDoneRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        
	return res;                
}

/*
srmPutDoneResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmPutDoneResponse)
{
	GFAL_DECL_ALLOC(srmPutDoneResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
         
	return res;
}

/*
srmAbortRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmAbortRequestRequest)
{
	struct  srm22__srmAbortRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmAbortRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmAbortRequestResponse)
{
	GFAL_DECL_ALLOC(srmAbortRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
         
	return res;
}

/*
srmAbortFilesRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmAbortFilesRequest)
{
	struct  srm22__srmAbortFilesRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmAbortFilesResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmAbortFilesResponse)
{
	GFAL_DECL_ALLOC(srmAbortFilesResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
         
	return res;
}

/*
srmSuspendRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmSuspendRequestRequest)
{
	struct  srm22__srmSuspendRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmSuspendRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmSuspendRequestResponse)
{
	GFAL_DECL_ALLOC(srmSuspendRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
         
	return res;
}

/*
srmResumeRequestRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmResumeRequestRequest)
{
	struct  srm22__srmResumeRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmResumeRequestResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmResumeRequestResponse)
{
	GFAL_DECL_ALLOC(srmResumeRequestResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
         
	return res;
}

/*
srmGetRequestSummaryRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmGetRequestSummaryRequest)
{
	struct  srm22__srmGetRequestSummaryRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	CONV2SOAP_OBJ(ArrayOfString,arrayOfRequestTokens);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmGetRequestSummaryResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmGetRequestSummaryResponse)
{
	GFAL_DECL_ALLOC(srmGetRequestSummaryResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTRequestSummary,arrayOfRequestSummaries);
         
	return res;
}

/*
srmExtendFileLifeTimeRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmExtendFileLifeTimeRequest)
{
	struct  srm22__srmExtendFileLifeTimeRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	SOAP_PTR_ALLOC(requestToken);
    	STRING_TO_SOAP_EMB(requestToken);
        	CONV2SOAP_OBJ(ArrayOfAnyURI,arrayOfSURLs);
        	SOAP_PTR_ALLOC(newFileLifeTime);
    	NUM_TO_SOAP_PTR(newFileLifeTime);                
	SOAP_PTR_ALLOC(newPinLifeTime);
    	NUM_TO_SOAP_PTR(newPinLifeTime);                

	return res;                
}

/*
srmExtendFileLifeTimeResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmExtendFileLifeTimeResponse)
{
	GFAL_DECL_ALLOC(srmExtendFileLifeTimeResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTSURLLifetimeReturnStatus,arrayOfFileStatuses);
         
	return res;
}

/*
srmGetRequestTokensRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmGetRequestTokensRequest)
{
	struct  srm22__srmGetRequestTokensRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(userRequestDescription);
    	STRING_TO_SOAP_EMB(userRequestDescription);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmGetRequestTokensResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmGetRequestTokensResponse)
{
	GFAL_DECL_ALLOC(srmGetRequestTokensResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTRequestTokenReturn,arrayOfRequestTokens);
         
	return res;
}

/*
srmGetTransferProtocolsRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmGetTransferProtocolsRequest)
{
	struct  srm22__srmGetTransferProtocolsRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmGetTransferProtocolsResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmGetTransferProtocolsResponse)
{
	GFAL_DECL_ALLOC(srmGetTransferProtocolsResponse);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
    	CONV2GFAL_OBJ(ArrayOfTSupportedTransferProtocol,protocolInfo);
         
	return res;
}

/*
srmPingRequest    
is the 
main-input type.
*/           
DEF_GFALCONV_HEADERS_IN(srmPingRequest)
{
	struct  srm22__srmPingRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmPingResponse    
is the 
main-output type.
*/           
DEF_GFALCONV_HEADERS_OUT(srmPingResponse)
{
	GFAL_DECL_ALLOC(srmPingResponse);
	STRING_FROM_SOAP_EMB(versionInfo);
        	CONV2GFAL_OBJ(ArrayOfTExtraInfo,otherInfo);
         
	return res;
}
