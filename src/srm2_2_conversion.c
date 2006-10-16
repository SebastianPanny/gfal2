
        
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
enum srm22__##_name* convEnum2soap_srm22_##_name(struct soap* soap, int _res) \
{ 	enum srm22__##_name* res;	\
res = soap_malloc(soap,sizeof(*res));	\
if(!res) return NULL;	\
*res =(enum srm22__##_name)_res;	\
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
res->_name=conv2soap_srm22_##_type(soap,(_elem->_name));	

#define CONV2GFAL_OBJ(_type,_name)	\
res->_name=conv2gfal_srm22_##_type(_elem->_name);	


#define GEN_ARRAY_CONV2SOAP(_typeName,_sizeName,_arrName) 	\
DEF_GFALCONV_HEADERS_IN(ArrayOf##_typeName){    \
int i;	\
struct srm22__ArrayOf##_typeName* res;	\
SOAP_MAIN_ALLOC;	\
res->_sizeName=_elem->_sizeName;	\
res->_arrName = soap_malloc(soap,res->_sizeName*sizeof(*(res->_arrName)));	\
if(_elem->_sizeName && !res->_arrName) return NULL;	\
for(i=0;i<_elem->_sizeName;i++)	\
{	\
res->_arrName[i]=conv2soap_srm22_##_typeName(soap,_elem->_arrName[i]);	\
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
res->_arrName[i]=conv2soap_srm22_##_subTypeName(soap,_elem->_arrName[i]);	\
if(!res->_arrName[i]) return NULL;	\
};    \
return res;} 
	
	

#define GEN_ARRAY_CONV2SOAP2(_typeName,_sizeName,_arrName) 	\
int i;	\
struct srm22__ArrayOf##_typeName* res;	\
SOAP_MAIN_ALLOC;	\
res->_sizeName=_elem->_sizeName;	\
res->_arrName = soap_malloc(soap,res->_sizeName*sizeof(*res->_arrName));	\
if(_elem->_sizeName && !res->_arrName) return NULL;	\
for(i=0;i<_elem->_sizeName;i++)	\
{	\
res->_arrName[i]=conv2soap_srm22_##_typeName(soap,(_typeName)&_elem[i]);	\
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
    freeType_srm22_ArrayOf##_typeName(ret);	\
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
    ret = malloc(sizeof(srm22__ArrayOf##_typeName));	\
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
        ret->_arrName[i] = conv2gfal_ srm22_##_typeName##_##_typeNameIn(_elem->_arrNameIn[i]);	\
        if(!ret->_arrName[i])				\
        {									\
            ret->_sizeName = i+1;			\
            freeType_srm22_ArrayOf##_typeName(ret);	\
            return NULL;					\
        }									\
        } 										\
        else ret->_arrName[i] = NULL;			\
    }											\
    ret->_sizeName = _elem->_sizeNameIn;

	
	
#define GEN_ARRAY_CONV2GFAL_SIMPLE(_typeName,_subTypeName,_sizeName,_arrName)	\
DEF_GFALCONV_HEADERS_OUT(_typeName){            \
    srm22_##_typeName* ret;					\
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
    freeType_srm22_ArrayOf##_subTypeName(ret);	\
    return NULL;					\
    }									\
    } 										\
    else ret->_arrName[i] = NULL;			\
    }											\
    ret->_sizeName = _elem->_sizeName;        \
    return ret;}	
    



#define GFAL_FREEARRAY_TYPE_DEF(_typeName,_sizeName,_arrName)	\
void freeType_srm22_ArrayOf##_typeName(ArrayOf##_typeName* _elem) 			\
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
void freeArray_srm22_##_name(int i, _name** ptrArr) 	\
{	    for(;i>0;i--){if(ptrArr[i-1]) 				\
    free(ptrArr[i-1]);};}	

#define GFAL_FREEARRAY_CALL(_name,_i,_ptr)	freeType_ArrayOf##_name(_ptr);

#define CONSTRUCTOR_ARRAY_DEF(_n,_Name)	\
CONSTRUCTOR_DEC(ArrayOf##_n,_n** _Name##Array ,int _size)	\
{												\
    srm22__ArrayOf##_n* tdata1;    					\
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
	return *_elem;
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
struct srm22__TRetentionPolicyInfo * conv2soap_srm22_TRetentionPolicyInfo(struct soap *soap, const srm22_TRetentionPolicyInfo * _elem)
{
	struct  srm22__TRetentionPolicyInfo* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);

	res->retentionPolicy = *((enum srm22_TRetentionPolicy*)(convEnum2soap_srm22_TRetentionPolicy(soap,_elem->retentionPolicy)));
	res->accessLatency=convEnum2soap_srm22_TAccessLatency(soap,_elem->accessLatency);	

	return res;                
}

//DEF_GFALCONV_HEADERS_OUT(TRetentionPolicyInfo)
srm22_TRetentionPolicyInfo * conv2gfal_srm22_TRetentionPolicyInfo(const struct srm22__TRetentionPolicyInfo* _elem)
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
struct srm22__TUserPermission * conv2soap_srm22_TUserPermission(struct soap *soap, const srm22_TUserPermission * _elem)
{
	struct  srm22__TUserPermission* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(userID);
        
	res->mode = *((enum srm22_TPermissionMode*)(convEnum2soap_srm22_TPermissionMode(soap,_elem->mode)));

	return res;                
}

//DEF_GFALCONV_HEADERS_OUT(TUserPermission)
srm22_TUserPermission * conv2gfal_srm22_TUserPermission(const struct srm22__TUserPermission* _elem)
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
struct srm22__TGroupPermission * conv2soap_srm22_TGroupPermission(struct soap *soap, const srm22_TGroupPermission * _elem)
{
	struct  srm22__TGroupPermission* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(groupID);
        
	res->mode = *((enum srm22_TPermissionMode*)(convEnum2soap_srm22_TPermissionMode(soap,_elem->mode)));

	return res;                
}

//DEF_GFALCONV_HEADERS_OUT(TGroupPermission)
srm22_TGroupPermission * conv2gfal_srm22_TGroupPermission(const struct srm22__TGroupPermission* _elem)
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

GEN_ARRAY_CONV2SOAP_SIMPLE(ArrayOfUnsignedLong,unsignedLong,__sizeunsignedLongArray, unsignedLongArray);
	
/*
    Array handling for:
    ArrayOfString    
    is the 
    input/output
*/

GEN_ARRAY_CONV2SOAP_SIMPLE(ArrayOfString,String,__sizestringArray, stringArray);
GEN_ARRAY_CONV2GFAL_SIMPLE(ArrayOfString,String,__sizestringArray, stringArray);    

/*
TReturnStatus    
is the 
deep-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(TReturnStatus)
srm22_TReturnStatus * conv2gfal_srm22_TReturnStatus(const struct srm22__TReturnStatus* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TSURLReturnStatus)
srm22_TSURLReturnStatus * conv2gfal_srm22_TSURLReturnStatus(const struct srm22__TSURLReturnStatus* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TSURLLifetimeReturnStatus)
srm22_TSURLLifetimeReturnStatus * conv2gfal_srm22_TSURLLifetimeReturnStatus(const struct srm22__TSURLLifetimeReturnStatus* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TMetaDataPathDetail)
srm22_TMetaDataPathDetail * conv2gfal_srm22_TMetaDataPathDetail(const struct srm22__TMetaDataPathDetail* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TMetaDataSpace)
srm22_TMetaDataSpace * conv2gfal_srm22_TMetaDataSpace(const struct srm22__TMetaDataSpace* _elem)
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

struct srm22__TDirOption * conv2soap_srm22_TDirOption(struct soap *soap, const srm22_TDirOption * _elem)
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
struct srm22__TExtraInfo * conv2soap_srm22_TExtraInfo(struct soap *soap, const srm22_TExtraInfo * _elem)
{
	struct  srm22__TExtraInfo* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(key);
        	SOAP_PTR_ALLOC(value);
    	STRING_TO_SOAP_EMB(value);
        
	return res;                
}

//DEF_GFALCONV_HEADERS_OUT(TExtraInfo)
srm22_TExtraInfo * conv2gfal_srm22_TExtraInfo(const struct srm22__TExtraInfo* _elem)
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
    complex-conversion
*/
srm22_ArrayOfString * conv2gfal_srm22__ArrayOfAnyURI_2_ArrayOfString(const struct srm22__ArrayOfAnyURI * _elem)
{	
	GFAL_DECL_ALLOC(ArrayOfString);
	//conv	
	return res;
}
	
struct srm22__ArrayOfAnyURI * conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(struct soap *soap, srm22_ArrayOfString * _elem)
{
	struct  srm22__ArrayOfAnyURI* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	//conv
	return res;
}

/*
TTransferParameters    
is the 
deep-input type.
*/           

struct srm22__TTransferParameters * conv2soap_srm22_TTransferParameters(struct soap *soap, const srm22_TTransferParameters * _elem)
{
	struct  srm22__TTransferParameters* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	res->accessPattern=convEnum2soap_srm22_TAccessPattern(soap,_elem->accessPattern);	
	res->connectionType=convEnum2soap_srm22_TConnectionType(soap,_elem->connectionType);	
	res->arrayOfClientNetworks=conv2soap_srm22_ArrayOfString(soap,(_elem->arrayOfClientNetworks));
	res->arrayOfTransferProtocols=conv2soap_srm22_ArrayOfString(soap,(_elem->arrayOfTransferProtocols));

	return res;                
}

/*
TGetFileRequest    
is the 
deep-input type.
*/           

struct srm22__TGetFileRequest * conv2soap_srm22_TGetFileRequest(struct soap *soap, const srm22_TGetFileRequest * _elem)
{
	struct  srm22__TGetFileRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(sourceSURL);
        	res->dirOption=conv2soap_srm22_TDirOption(soap,(_elem->dirOption));

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

struct srm22__TPutFileRequest * conv2soap_srm22_TPutFileRequest(struct soap *soap, const srm22_TPutFileRequest * _elem)
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

struct srm22__TCopyFileRequest * conv2soap_srm22_TCopyFileRequest(struct soap *soap, const srm22_TCopyFileRequest * _elem)
{
	struct  srm22__TCopyFileRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(sourceSURL);
        	STRING_TO_SOAP_EMB(targetSURL);
        	res->dirOption=conv2soap_srm22_TDirOption(soap,(_elem->dirOption));

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
//DEF_GFALCONV_HEADERS_OUT(TGetRequestFileStatus)
srm22_TGetRequestFileStatus * conv2gfal_srm22_TGetRequestFileStatus(const struct srm22__TGetRequestFileStatus* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TBringOnlineRequestFileStatus)
srm22_TBringOnlineRequestFileStatus * conv2gfal_srm22_TBringOnlineRequestFileStatus(const struct srm22__TBringOnlineRequestFileStatus* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TPutRequestFileStatus)
srm22_TPutRequestFileStatus * conv2gfal_srm22_TPutRequestFileStatus(const struct srm22__TPutRequestFileStatus* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TCopyRequestFileStatus)
srm22_TCopyRequestFileStatus * conv2gfal_srm22_TCopyRequestFileStatus(const struct srm22__TCopyRequestFileStatus* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TRequestSummary)
srm22_TRequestSummary * conv2gfal_srm22_TRequestSummary(const struct srm22__TRequestSummary* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TSURLPermissionReturn)
srm22_TSURLPermissionReturn * conv2gfal_srm22_TSURLPermissionReturn(const struct srm22__TSURLPermissionReturn* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TPermissionReturn)
srm22_TPermissionReturn * conv2gfal_srm22_TPermissionReturn(const struct srm22__TPermissionReturn* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TRequestTokenReturn)
srm22_TRequestTokenReturn * conv2gfal_srm22_TRequestTokenReturn(const struct srm22__TRequestTokenReturn* _elem)
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
//DEF_GFALCONV_HEADERS_OUT(TSupportedTransferProtocol)
srm22_TSupportedTransferProtocol * conv2gfal_srm22_TSupportedTransferProtocol(const struct srm22__TSupportedTransferProtocol* _elem)
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

struct srm22__srmReserveSpaceRequest * conv2soap_srm22_srmReserveSpaceRequest(struct soap *soap, const srm22_srmReserveSpaceRequest * _elem)
{
	struct  srm22__srmReserveSpaceRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	SOAP_PTR_ALLOC(userSpaceTokenDescription);
    	STRING_TO_SOAP_EMB(userSpaceTokenDescription);
        	res->retentionPolicyInfo=conv2soap_srm22_TRetentionPolicyInfo(soap,(_elem->retentionPolicyInfo));
	SOAP_PTR_ALLOC(desiredSizeOfTotalSpace);
    	NUM_TO_SOAP_PTR(desiredSizeOfTotalSpace);                
	NUM_TO_SOAP(desiredSizeOfGuaranteedSpace);
	SOAP_PTR_ALLOC(desiredLifetimeOfReservedSpace);
    	NUM_TO_SOAP_PTR(desiredLifetimeOfReservedSpace);                
	res->arrayOfExpectedFileSizes=conv2soap_srm22_ArrayOfUnsignedLong(soap,(_elem->arrayOfExpectedFileSizes));
	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));
	res->transferParameters=conv2soap_srm22_TTransferParameters(soap,(_elem->transferParameters));

	return res;                
}

/*
srmReserveSpaceResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmReserveSpaceResponse)
srm22_srmReserveSpaceResponse * conv2gfal_srm22_srmReserveSpaceResponse(const struct srm22__srmReserveSpaceResponse* _elem)
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

struct srm22__srmStatusOfReserveSpaceRequestRequest * conv2soap_srm22_srmStatusOfReserveSpaceRequestRequest(struct soap *soap, const srm22_srmStatusOfReserveSpaceRequestRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmStatusOfReserveSpaceRequestResponse)
srm22_srmStatusOfReserveSpaceRequestResponse * conv2gfal_srm22_srmStatusOfReserveSpaceRequestResponse(const struct srm22__srmStatusOfReserveSpaceRequestResponse* _elem)
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

struct srm22__srmReleaseSpaceRequest * conv2soap_srm22_srmReleaseSpaceRequest(struct soap *soap, const srm22_srmReleaseSpaceRequest * _elem)
{
	struct  srm22__srmReleaseSpaceRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(spaceToken);
        	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));
	SOAP_PTR_ALLOC(forceFileRelease);
    	NUM_TO_SOAP_PTR(forceFileRelease);                

	return res;                
}

/*
srmReleaseSpaceResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmReleaseSpaceResponse)
srm22_srmReleaseSpaceResponse * conv2gfal_srm22_srmReleaseSpaceResponse(const struct srm22__srmReleaseSpaceResponse* _elem)
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

struct srm22__srmUpdateSpaceRequest * conv2soap_srm22_srmUpdateSpaceRequest(struct soap *soap, const srm22_srmUpdateSpaceRequest * _elem)
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
	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));

	return res;                
}

/*
srmUpdateSpaceResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmUpdateSpaceResponse)
srm22_srmUpdateSpaceResponse * conv2gfal_srm22_srmUpdateSpaceResponse(const struct srm22__srmUpdateSpaceResponse* _elem)
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

struct srm22__srmStatusOfUpdateSpaceRequestRequest * conv2soap_srm22_srmStatusOfUpdateSpaceRequestRequest(struct soap *soap, const srm22_srmStatusOfUpdateSpaceRequestRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmStatusOfUpdateSpaceRequestResponse)
srm22_srmStatusOfUpdateSpaceRequestResponse * conv2gfal_srm22_srmStatusOfUpdateSpaceRequestResponse(const struct srm22__srmStatusOfUpdateSpaceRequestResponse* _elem)
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

struct srm22__srmGetSpaceMetaDataRequest * conv2soap_srm22_srmGetSpaceMetaDataRequest(struct soap *soap, const srm22_srmGetSpaceMetaDataRequest * _elem)
{
	struct  srm22__srmGetSpaceMetaDataRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	res->arrayOfSpaceTokens=conv2soap_srm22_ArrayOfString(soap,(_elem->arrayOfSpaceTokens));

	return res;                
}

/*
srmGetSpaceMetaDataResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmGetSpaceMetaDataResponse)
srm22_srmGetSpaceMetaDataResponse * conv2gfal_srm22_srmGetSpaceMetaDataResponse(const struct srm22__srmGetSpaceMetaDataResponse* _elem)
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

struct srm22__srmChangeSpaceForFilesRequest * conv2soap_srm22_srmChangeSpaceForFilesRequest(struct soap *soap, const srm22_srmChangeSpaceForFilesRequest * _elem)
{
	struct  srm22__srmChangeSpaceForFilesRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
	STRING_TO_SOAP_EMB(targetSpaceToken);
        	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));

	return res;                
}

/*
srmChangeSpaceForFilesResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmChangeSpaceForFilesResponse)
srm22_srmChangeSpaceForFilesResponse * conv2gfal_srm22_srmChangeSpaceForFilesResponse(const struct srm22__srmChangeSpaceForFilesResponse* _elem)
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

struct srm22__srmStatusOfChangeSpaceForFilesRequestRequest * conv2soap_srm22_srmStatusOfChangeSpaceForFilesRequestRequest(struct soap *soap, const srm22_srmStatusOfChangeSpaceForFilesRequestRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmStatusOfChangeSpaceForFilesRequestResponse)
srm22_srmStatusOfChangeSpaceForFilesRequestResponse * conv2gfal_srm22_srmStatusOfChangeSpaceForFilesRequestResponse(const struct srm22__srmStatusOfChangeSpaceForFilesRequestResponse* _elem)
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

struct srm22__srmExtendFileLifeTimeInSpaceRequest * conv2soap_srm22_srmExtendFileLifeTimeInSpaceRequest(struct soap *soap, const srm22_srmExtendFileLifeTimeInSpaceRequest * _elem)
{
	struct  srm22__srmExtendFileLifeTimeInSpaceRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(spaceToken);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
	SOAP_PTR_ALLOC(newLifeTime);
    	NUM_TO_SOAP_PTR(newLifeTime);                

	return res;                
}

/*
srmExtendFileLifeTimeInSpaceResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmExtendFileLifeTimeInSpaceResponse)
srm22_srmExtendFileLifeTimeInSpaceResponse * conv2gfal_srm22_srmExtendFileLifeTimeInSpaceResponse(const struct srm22__srmExtendFileLifeTimeInSpaceResponse* _elem)
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

struct srm22__srmPurgeFromSpaceRequest * conv2soap_srm22_srmPurgeFromSpaceRequest(struct soap *soap, const srm22_srmPurgeFromSpaceRequest * _elem)
{
	struct  srm22__srmPurgeFromSpaceRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
	STRING_TO_SOAP_EMB(spaceToken);
        	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));

	return res;                
}

/*
srmPurgeFromSpaceResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmPurgeFromSpaceResponse)
srm22_srmPurgeFromSpaceResponse * conv2gfal_srm22_srmPurgeFromSpaceResponse(const struct srm22__srmPurgeFromSpaceResponse* _elem)
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

struct srm22__srmGetSpaceTokensRequest * conv2soap_srm22_srmGetSpaceTokensRequest(struct soap *soap, const srm22_srmGetSpaceTokensRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmGetSpaceTokensResponse)
srm22_srmGetSpaceTokensResponse * conv2gfal_srm22_srmGetSpaceTokensResponse(const struct srm22__srmGetSpaceTokensResponse* _elem)
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

struct srm22__srmSetPermissionRequest * conv2soap_srm22_srmSetPermissionRequest(struct soap *soap, const srm22_srmSetPermissionRequest * _elem)
{
	struct  srm22__srmSetPermissionRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(SURL);
        
	res->permissionType = *((enum srm22_TPermissionType*)(convEnum2soap_srm22_TPermissionType(soap,_elem->permissionType)));
	res->ownerPermission=convEnum2soap_srm22_TPermissionMode(soap,_elem->ownerPermission);	
	res->arrayOfUserPermissions=conv2soap_srm22_ArrayOfTUserPermission(soap,(_elem->arrayOfUserPermissions));
	res->arrayOfGroupPermissions=conv2soap_srm22_ArrayOfTGroupPermission(soap,(_elem->arrayOfGroupPermissions));
	res->otherPermission=convEnum2soap_srm22_TPermissionMode(soap,_elem->otherPermission);	
	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));

	return res;                
}

/*
srmSetPermissionResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmSetPermissionResponse)
srm22_srmSetPermissionResponse * conv2gfal_srm22_srmSetPermissionResponse(const struct srm22__srmSetPermissionResponse* _elem)
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

struct srm22__srmCheckPermissionRequest * conv2soap_srm22_srmCheckPermissionRequest(struct soap *soap, const srm22_srmCheckPermissionRequest * _elem)
{
	struct  srm22__srmCheckPermissionRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));

	return res;                
}

/*
srmCheckPermissionResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmCheckPermissionResponse)
srm22_srmCheckPermissionResponse * conv2gfal_srm22_srmCheckPermissionResponse(const struct srm22__srmCheckPermissionResponse* _elem)
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

struct srm22__srmGetPermissionRequest * conv2soap_srm22_srmGetPermissionRequest(struct soap *soap, const srm22_srmGetPermissionRequest * _elem)
{
	struct  srm22__srmGetPermissionRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));

	return res;                
}

/*
srmGetPermissionResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmGetPermissionResponse)
srm22_srmGetPermissionResponse * conv2gfal_srm22_srmGetPermissionResponse(const struct srm22__srmGetPermissionResponse* _elem)
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

struct srm22__srmMkdirRequest * conv2soap_srm22_srmMkdirRequest(struct soap *soap, const srm22_srmMkdirRequest * _elem)
{
	struct  srm22__srmMkdirRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(directoryPath);
        	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));

	return res;                
}

/*
srmMkdirResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmMkdirResponse)
srm22_srmMkdirResponse * conv2gfal_srm22_srmMkdirResponse(const struct srm22__srmMkdirResponse* _elem)
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

struct srm22__srmRmdirRequest * conv2soap_srm22_srmRmdirRequest(struct soap *soap, const srm22_srmRmdirRequest * _elem)
{
	struct  srm22__srmRmdirRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(directoryPath);
        	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));
	SOAP_PTR_ALLOC(recursive);
    	NUM_TO_SOAP_PTR(recursive);                

	return res;                
}

/*
srmRmdirResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmRmdirResponse)
srm22_srmRmdirResponse * conv2gfal_srm22_srmRmdirResponse(const struct srm22__srmRmdirResponse* _elem)
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

struct srm22__srmRmRequest * conv2soap_srm22_srmRmRequest(struct soap *soap, const srm22_srmRmRequest * _elem)
{
	struct  srm22__srmRmRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));

	return res;                
}

/*
srmRmResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmRmResponse)
srm22_srmRmResponse * conv2gfal_srm22_srmRmResponse(const struct srm22__srmRmResponse* _elem)
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

struct srm22__srmLsRequest * conv2soap_srm22_srmLsRequest(struct soap *soap, const srm22_srmLsRequest * _elem)
{
	struct  srm22__srmLsRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));
	res->fileStorageType=convEnum2soap_srm22_TFileStorageType(soap,_elem->fileStorageType);	
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
//DEF_GFALCONV_HEADERS_OUT(srmLsResponse)
srm22_srmLsResponse * conv2gfal_srm22_srmLsResponse(const struct srm22__srmLsResponse* _elem)
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

struct srm22__srmStatusOfLsRequestRequest * conv2soap_srm22_srmStatusOfLsRequestRequest(struct soap *soap, const srm22_srmStatusOfLsRequestRequest * _elem)
{
	struct  srm22__srmStatusOfLsRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(offset);
    	NUM_TO_SOAP_PTR(offset);                
	SOAP_PTR_ALLOC(count);
    	NUM_TO_SOAP_PTR(count);                

	return res;                
}

/*
srmStatusOfLsRequestResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmStatusOfLsRequestResponse)
srm22_srmStatusOfLsRequestResponse * conv2gfal_srm22_srmStatusOfLsRequestResponse(const struct srm22__srmStatusOfLsRequestResponse* _elem)
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

struct srm22__srmMvRequest * conv2soap_srm22_srmMvRequest(struct soap *soap, const srm22_srmMvRequest * _elem)
{
	struct  srm22__srmMvRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	STRING_TO_SOAP_EMB(fromSURL);
        	STRING_TO_SOAP_EMB(toSURL);
        	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));

	return res;                
}

/*
srmMvResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmMvResponse)
srm22_srmMvResponse * conv2gfal_srm22_srmMvResponse(const struct srm22__srmMvResponse* _elem)
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

struct srm22__srmPrepareToGetRequest * conv2soap_srm22_srmPrepareToGetRequest(struct soap *soap, const srm22_srmPrepareToGetRequest * _elem)
{
	struct  srm22__srmPrepareToGetRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	res->arrayOfFileRequests=conv2soap_srm22_ArrayOfTGetFileRequest(soap,(_elem->arrayOfFileRequests));
	SOAP_PTR_ALLOC(userRequestDescription);
    	STRING_TO_SOAP_EMB(userRequestDescription);
        	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));
	res->desiredFileStorageType=convEnum2soap_srm22_TFileStorageType(soap,_elem->desiredFileStorageType);	
	SOAP_PTR_ALLOC(desiredTotalRequestTime);
    	NUM_TO_SOAP_PTR(desiredTotalRequestTime);                
	SOAP_PTR_ALLOC(desiredPinLifeTime);
    	NUM_TO_SOAP_PTR(desiredPinLifeTime);                
	SOAP_PTR_ALLOC(targetSpaceToken);
    	STRING_TO_SOAP_EMB(targetSpaceToken);
        	res->targetFileRetentionPolicyInfo=conv2soap_srm22_TRetentionPolicyInfo(soap,(_elem->targetFileRetentionPolicyInfo));
	res->transferParameters=conv2soap_srm22_TTransferParameters(soap,(_elem->transferParameters));

	return res;                
}

/*
srmPrepareToGetResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmPrepareToGetResponse)
srm22_srmPrepareToGetResponse * conv2gfal_srm22_srmPrepareToGetResponse(const struct srm22__srmPrepareToGetResponse* _elem)
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

struct srm22__srmStatusOfGetRequestRequest * conv2soap_srm22_srmStatusOfGetRequestRequest(struct soap *soap, const srm22_srmStatusOfGetRequestRequest * _elem)
{
	struct  srm22__srmStatusOfGetRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSourceSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSourceSURLs);

	return res;                
}

/*
srmStatusOfGetRequestResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmStatusOfGetRequestResponse)
srm22_srmStatusOfGetRequestResponse * conv2gfal_srm22_srmStatusOfGetRequestResponse(const struct srm22__srmStatusOfGetRequestResponse* _elem)
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

struct srm22__srmBringOnlineRequest * conv2soap_srm22_srmBringOnlineRequest(struct soap *soap, const srm22_srmBringOnlineRequest * _elem)
{
	struct  srm22__srmBringOnlineRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	res->arrayOfFileRequests=conv2soap_srm22_ArrayOfTGetFileRequest(soap,(_elem->arrayOfFileRequests));
	SOAP_PTR_ALLOC(userRequestDescription);
    	STRING_TO_SOAP_EMB(userRequestDescription);
        	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));
	res->desiredFileStorageType=convEnum2soap_srm22_TFileStorageType(soap,_elem->desiredFileStorageType);	
	SOAP_PTR_ALLOC(desiredTotalRequestTime);
    	NUM_TO_SOAP_PTR(desiredTotalRequestTime);                
	SOAP_PTR_ALLOC(desiredLifeTime);
    	NUM_TO_SOAP_PTR(desiredLifeTime);                
	SOAP_PTR_ALLOC(targetSpaceToken);
    	STRING_TO_SOAP_EMB(targetSpaceToken);
        	res->targetFileRetentionPolicyInfo=conv2soap_srm22_TRetentionPolicyInfo(soap,(_elem->targetFileRetentionPolicyInfo));
	res->transferParameters=conv2soap_srm22_TTransferParameters(soap,(_elem->transferParameters));
	SOAP_PTR_ALLOC(deferredStartTime);
    	NUM_TO_SOAP_PTR(deferredStartTime);                

	return res;                
}

/*
srmBringOnlineResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmBringOnlineResponse)
srm22_srmBringOnlineResponse * conv2gfal_srm22_srmBringOnlineResponse(const struct srm22__srmBringOnlineResponse* _elem)
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

struct srm22__srmStatusOfBringOnlineRequestRequest * conv2soap_srm22_srmStatusOfBringOnlineRequestRequest(struct soap *soap, const srm22_srmStatusOfBringOnlineRequestRequest * _elem)
{
	struct  srm22__srmStatusOfBringOnlineRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSourceSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSourceSURLs);

	return res;                
}

/*
srmStatusOfBringOnlineRequestResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmStatusOfBringOnlineRequestResponse)
srm22_srmStatusOfBringOnlineRequestResponse * conv2gfal_srm22_srmStatusOfBringOnlineRequestResponse(const struct srm22__srmStatusOfBringOnlineRequestResponse* _elem)
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

struct srm22__srmPrepareToPutRequest * conv2soap_srm22_srmPrepareToPutRequest(struct soap *soap, const srm22_srmPrepareToPutRequest * _elem)
{
	struct  srm22__srmPrepareToPutRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	res->arrayOfFileRequests=conv2soap_srm22_ArrayOfTPutFileRequest(soap,(_elem->arrayOfFileRequests));
	SOAP_PTR_ALLOC(userRequestDescription);
    	STRING_TO_SOAP_EMB(userRequestDescription);
        	res->overwriteOption=convEnum2soap_srm22_TOverwriteMode(soap,_elem->overwriteOption);	
	res->storageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->storageSystemInfo));
	SOAP_PTR_ALLOC(desiredTotalRequestTime);
    	NUM_TO_SOAP_PTR(desiredTotalRequestTime);                
	SOAP_PTR_ALLOC(desiredPinLifeTime);
    	NUM_TO_SOAP_PTR(desiredPinLifeTime);                
	SOAP_PTR_ALLOC(desiredFileLifeTime);
    	NUM_TO_SOAP_PTR(desiredFileLifeTime);                
	res->desiredFileStorageType=convEnum2soap_srm22_TFileStorageType(soap,_elem->desiredFileStorageType);	
	SOAP_PTR_ALLOC(targetSpaceToken);
    	STRING_TO_SOAP_EMB(targetSpaceToken);
        	res->targetFileRetentionPolicyInfo=conv2soap_srm22_TRetentionPolicyInfo(soap,(_elem->targetFileRetentionPolicyInfo));
	res->transferParameters=conv2soap_srm22_TTransferParameters(soap,(_elem->transferParameters));

	return res;                
}

/*
srmPrepareToPutResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmPrepareToPutResponse)
srm22_srmPrepareToPutResponse * conv2gfal_srm22_srmPrepareToPutResponse(const struct srm22__srmPrepareToPutResponse* _elem)
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

struct srm22__srmStatusOfPutRequestRequest * conv2soap_srm22_srmStatusOfPutRequestRequest(struct soap *soap, const srm22_srmStatusOfPutRequestRequest * _elem)
{
	struct  srm22__srmStatusOfPutRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfTargetSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfTargetSURLs);

	return res;                
}

/*
srmStatusOfPutRequestResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmStatusOfPutRequestResponse)
srm22_srmStatusOfPutRequestResponse * conv2gfal_srm22_srmStatusOfPutRequestResponse(const struct srm22__srmStatusOfPutRequestResponse* _elem)
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

struct srm22__srmCopyRequest * conv2soap_srm22_srmCopyRequest(struct soap *soap, const srm22_srmCopyRequest * _elem)
{
	struct  srm22__srmCopyRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	res->arrayOfFileRequests=conv2soap_srm22_ArrayOfTCopyFileRequest(soap,(_elem->arrayOfFileRequests));
	SOAP_PTR_ALLOC(userRequestDescription);
    	STRING_TO_SOAP_EMB(userRequestDescription);
        	res->overwriteOption=convEnum2soap_srm22_TOverwriteMode(soap,_elem->overwriteOption);	
	SOAP_PTR_ALLOC(desiredTotalRequestTime);
    	NUM_TO_SOAP_PTR(desiredTotalRequestTime);                
	SOAP_PTR_ALLOC(desiredTargetSURLLifeTime);
    	NUM_TO_SOAP_PTR(desiredTargetSURLLifeTime);                
	res->targetFileStorageType=convEnum2soap_srm22_TFileStorageType(soap,_elem->targetFileStorageType);	
	SOAP_PTR_ALLOC(targetSpaceToken);
    	STRING_TO_SOAP_EMB(targetSpaceToken);
        	res->targetFileRetentionPolicyInfo=conv2soap_srm22_TRetentionPolicyInfo(soap,(_elem->targetFileRetentionPolicyInfo));
	res->sourceStorageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->sourceStorageSystemInfo));
	res->targetStorageSystemInfo=conv2soap_srm22_ArrayOfTExtraInfo(soap,(_elem->targetStorageSystemInfo));

	return res;                
}

/*
srmCopyResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmCopyResponse)
srm22_srmCopyResponse * conv2gfal_srm22_srmCopyResponse(const struct srm22__srmCopyResponse* _elem)
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

struct srm22__srmStatusOfCopyRequestRequest * conv2soap_srm22_srmStatusOfCopyRequestRequest(struct soap *soap, const srm22_srmStatusOfCopyRequestRequest * _elem)
{
	struct  srm22__srmStatusOfCopyRequestRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSourceSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSourceSURLs);
		res->arrayOfTargetSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfTargetSURLs);

	return res;                
}

/*
srmStatusOfCopyRequestResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmStatusOfCopyRequestResponse)
srm22_srmStatusOfCopyRequestResponse * conv2gfal_srm22_srmStatusOfCopyRequestResponse(const struct srm22__srmStatusOfCopyRequestResponse* _elem)
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

struct srm22__srmReleaseFilesRequest * conv2soap_srm22_srmReleaseFilesRequest(struct soap *soap, const srm22_srmReleaseFilesRequest * _elem)
{
	struct  srm22__srmReleaseFilesRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(requestToken);
    	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
	SOAP_PTR_ALLOC(doRemove);
    	NUM_TO_SOAP_PTR(doRemove);                

	return res;                
}

/*
srmReleaseFilesResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmReleaseFilesResponse)
srm22_srmReleaseFilesResponse * conv2gfal_srm22_srmReleaseFilesResponse(const struct srm22__srmReleaseFilesResponse* _elem)
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

struct srm22__srmPutDoneRequest * conv2soap_srm22_srmPutDoneRequest(struct soap *soap, const srm22_srmPutDoneRequest * _elem)
{
	struct  srm22__srmPutDoneRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);

	return res;                
}

/*
srmPutDoneResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmPutDoneResponse)
srm22_srmPutDoneResponse * conv2gfal_srm22_srmPutDoneResponse(const struct srm22__srmPutDoneResponse* _elem)
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

struct srm22__srmAbortRequestRequest * conv2soap_srm22_srmAbortRequestRequest(struct soap *soap, const srm22_srmAbortRequestRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmAbortRequestResponse)
srm22_srmAbortRequestResponse * conv2gfal_srm22_srmAbortRequestResponse(const struct srm22__srmAbortRequestResponse* _elem)
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

struct srm22__srmAbortFilesRequest * conv2soap_srm22_srmAbortFilesRequest(struct soap *soap, const srm22_srmAbortFilesRequest * _elem)
{
	struct  srm22__srmAbortFilesRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	STRING_TO_SOAP_EMB(requestToken);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmAbortFilesResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmAbortFilesResponse)
srm22_srmAbortFilesResponse * conv2gfal_srm22_srmAbortFilesResponse(const struct srm22__srmAbortFilesResponse* _elem)
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

struct srm22__srmSuspendRequestRequest * conv2soap_srm22_srmSuspendRequestRequest(struct soap *soap, const srm22_srmSuspendRequestRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmSuspendRequestResponse)
srm22_srmSuspendRequestResponse * conv2gfal_srm22_srmSuspendRequestResponse(const struct srm22__srmSuspendRequestResponse* _elem)
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

struct srm22__srmResumeRequestRequest * conv2soap_srm22_srmResumeRequestRequest(struct soap *soap, const srm22_srmResumeRequestRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmResumeRequestResponse)
srm22_srmResumeRequestResponse * conv2gfal_srm22_srmResumeRequestResponse(const struct srm22__srmResumeRequestResponse* _elem)
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

struct srm22__srmGetRequestSummaryRequest * conv2soap_srm22_srmGetRequestSummaryRequest(struct soap *soap, const srm22_srmGetRequestSummaryRequest * _elem)
{
	struct  srm22__srmGetRequestSummaryRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	res->arrayOfRequestTokens=conv2soap_srm22_ArrayOfString(soap,(_elem->arrayOfRequestTokens));
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        
	return res;                
}

/*
srmGetRequestSummaryResponse    
is the 
main-output type.
*/           
//DEF_GFALCONV_HEADERS_OUT(srmGetRequestSummaryResponse)
srm22_srmGetRequestSummaryResponse * conv2gfal_srm22_srmGetRequestSummaryResponse(const struct srm22__srmGetRequestSummaryResponse* _elem)
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

struct srm22__srmExtendFileLifeTimeRequest * conv2soap_srm22_srmExtendFileLifeTimeRequest(struct soap *soap, const srm22_srmExtendFileLifeTimeRequest * _elem)
{
	struct  srm22__srmExtendFileLifeTimeRequest* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	SOAP_PTR_ALLOC(authorizationID);
    	STRING_TO_SOAP_EMB(authorizationID);
        	SOAP_PTR_ALLOC(requestToken);
    	STRING_TO_SOAP_EMB(requestToken);
        		res->arrayOfSURLs = conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(soap,_elem->arrayOfSURLs);
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
//DEF_GFALCONV_HEADERS_OUT(srmExtendFileLifeTimeResponse)
srm22_srmExtendFileLifeTimeResponse * conv2gfal_srm22_srmExtendFileLifeTimeResponse(const struct srm22__srmExtendFileLifeTimeResponse* _elem)
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

struct srm22__srmGetRequestTokensRequest * conv2soap_srm22_srmGetRequestTokensRequest(struct soap *soap, const srm22_srmGetRequestTokensRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmGetRequestTokensResponse)
srm22_srmGetRequestTokensResponse * conv2gfal_srm22_srmGetRequestTokensResponse(const struct srm22__srmGetRequestTokensResponse* _elem)
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

struct srm22__srmGetTransferProtocolsRequest * conv2soap_srm22_srmGetTransferProtocolsRequest(struct soap *soap, const srm22_srmGetTransferProtocolsRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmGetTransferProtocolsResponse)
srm22_srmGetTransferProtocolsResponse * conv2gfal_srm22_srmGetTransferProtocolsResponse(const struct srm22__srmGetTransferProtocolsResponse* _elem)
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

struct srm22__srmPingRequest * conv2soap_srm22_srmPingRequest(struct soap *soap, const srm22_srmPingRequest * _elem)
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
//DEF_GFALCONV_HEADERS_OUT(srmPingResponse)
srm22_srmPingResponse * conv2gfal_srm22_srmPingResponse(const struct srm22__srmPingResponse* _elem)
{
	GFAL_DECL_ALLOC(srmPingResponse);
	STRING_FROM_SOAP_EMB(versionInfo);
        	CONV2GFAL_OBJ(ArrayOfTExtraInfo,otherInfo);
         
	return res;
}
