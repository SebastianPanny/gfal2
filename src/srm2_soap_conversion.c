#include "srm2_soap_conversion.h"
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
enum srm2__ ## _name* convEnum2soap_ ## _name(struct soap* soap, int _res) \
{ 	enum srm2__ ## _name* res;	\
	res = soap_malloc(soap,sizeof(*res));	\
	if(!res) return NULL;	\
	*res =(enum srm2__ ## _name)_res;	\
	return res;	\
}

#define COPY_CHK(_name)	\
	tdata1->_name = _name;

#define NEW_DATA(_p) _p = malloc(sizeof(*_p));
 
DEF_GFALCONVENUM_DEF(TStatusCode);
 
DEF_GFALCONVENUM_DEF(TSpaceType);
	
DEF_GFALCONVENUM_DEF(TFileStorageType);
	
DEF_GFALCONVENUM_DEF(TFileType);
	
DEF_GFALCONVENUM_DEF(TPermissionMode);

DEF_GFALCONVENUM_DEF(TPermissionType);

DEF_GFALCONVENUM_DEF(TRequestType);
	
DEF_GFALCONVENUM_DEF(TOverwriteMode);

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
		res->_name->value = soap_strdup(soap, _elem->_name); \
		if (_elem->_name && !res->_name->value) \
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
	res->f = strdup(_elem->f->value);	\
	if(!res->f) return NULL;


#define NUM_FROM_SOAP_EMB(f) \
	res->f = _elem->f->value


#define NUM_FROM_SOAP(f) \
	res->f = _elem->f->value

#define CONV2SOAP_OBJ(_type,_name)	\
res->_name=conv2soap_##_type(soap,(_elem->_name));	

#define CONV2GFAL_OBJ(_type,_name)	\
res->_name=conv2gfal_##_type(_elem->_name);	


#warning free objects if failure
#define GEN_ARRAY_CONV2SOAP(_typeName,_sizeName,_arrName) 	\
	int i;	\
	struct srm2__ArrayOf ## _typeName* res;	\
	SOAP_MAIN_ALLOC;	\
	res->_sizeName=_elem->_sizeName;	\
	res->_arrName = soap_malloc(soap,res->_sizeName*sizeof(*(res->_arrName)));	\
	if(_elem->_sizeName && !res->_arrName) return NULL;	\
	for(i=0;i<_elem->_sizeName;i++)	\
	{	\
		res->_arrName[i]=conv2soap_##_typeName(soap,_elem->_arrName[i]);	\
		if(!res->_arrName[i]) return NULL;	\
	};


#define GEN_ARRAY_CONV2SOAP2(_typeName,_sizeName,_arrName) 	\
	int i;	\
	struct srm2__ArrayOf ## _typeName* res;	\
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
	_name* res;	\
	if(!_elem) return NULL;	\
	res = (_name*) malloc (sizeof(_name));	\
	if(!res) return NULL;

// to gfal conversion
#define GEN_ARRAY_CONV2GFAL(_typeName,_sizeName,_arrName)	\
	ArrayOf##_typeName* ret;					\
	int i;										\
	if(!_elem) return NULL;						\
	ret = malloc(sizeof(ArrayOf##_typeName));	\
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
			ret->_arrName[i] = conv2gfal_##_typeName(_elem->_arrName[i]);	\
			if(!ret->_arrName[i])				\
			{									\
				ret->_sizeName = i+1;			\
				freegfalArray_ArrayOf##_typeName(ret);	\
				return NULL;					\
			}									\
		} 										\
		else ret->_arrName[i] = NULL;			\
	}											\
	ret->_sizeName = _elem->_sizeName;


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
{	for(;i>0;i--){if(ptrArr[i-1]) 				\
		free(ptrArr[i-1]);};}	

#define GFAL_FREEARRAY_CALL(_name,_i,_ptr)	freegfalArray_ArrayOf##_name(_ptr);

#define END_GFAL_FREE	\
	free((void*)_arg);	\
	_arg=NULL;	

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

	

//////////////////////////////
#define NUM_TO_SOAP_PTR(_n) 	\
	*(res->_n) = _elem->_n;

DEF_GFALCONV_HEADERS_IN(TDirOption)
{
/*//enum xsd__boolean *allLevelRecursive;
//	enum xsd__boolean isSourceADirectory;
//	int numOfLevels;	
*/
	//zwraca * na srm2
	
	// default init
	struct srm2__TDirOption* res;	
	//check
	if (!_elem) return NULL;
	//main alloc
	res = soap_malloc(soap, sizeof(*res));
	if (!res) return NULL;
	memset(res, 0, sizeof(TDirOption));
	
	//sub alloc
	res->allLevelRecursive = soap_malloc(soap,sizeof(*res->allLevelRecursive));
	//alloc check and possible counter check
	if(!res->allLevelRecursive) return NULL;
	
	//copying	
	*(res->allLevelRecursive)=_elem->allLevelRecursive;
	NUM_TO_SOAP(isSourceADirectory);
	NUM_TO_SOAP(numOfLevels);
		
	return res;
}


CONSTRUCTOR_DEC(TDirOption,short int _allLevelRecursive,short int _isSourceADirectory,int _numOfLevels)
{
    TDirOption* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
    
	tdata1->allLevelRecursive=_allLevelRecursive;
	tdata1->isSourceADirectory = _isSourceADirectory;
	tdata1->numOfLevels = _numOfLevels;
	return tdata1;
}


SRM2_GFAL_FREE_TYPE_DEC(TDirOption)
{
	if(!_arg)return;
	END_GFAL_FREE;	
}


DEF_GFALCONV_HEADERS_OUT(TDirOption)
{
	return 0;	
}


/* grabbed from fts_soapconv.c*/
/* Convert a list of strings to gSOAP's ArrayOf_USCORExsd_USCOREstring */
int to_soap_StringArray(struct soap *soap,
	struct srm2__ArrayOf_USCORExsd_USCOREstring *req,
	int nitems, const char** items)
{
	int i;
	req->__sizestringArray = nitems;
	req->stringArray = soap_malloc(soap, nitems * sizeof(*req->stringArray));
	if (nitems && !req->stringArray)
		return -1;
	for (i = 0; i < nitems; i++)
	{	req->stringArray[i] = soap_strdup(soap, items[i]);
		if (!req->stringArray[i])
			return -1;
	}
	return 0;
}


//int conv2soap_TSURLArray(struct soap *soap, struct srm2__ArrayOfTSURL *req,	
//	int nitems, const TSURL* items)		
//{										
//	int i;								
//	req->__sizesurlArray = nitems;	
//	req->surlArray = soap_malloc(soap, nitems * sizeof(*req->surlArray));	
//	if (nitems && !req->surlArray) return -1;						
//	for (i = 0; i < nitems; i++){		
//		
//		req->surlArray[i]->value = (char*) soap_strdup(soap, items[i]);	
//		if (!req->surlArray[i]) return -1;}	
//	return 0;	
//}
//CONV2SOAP_EMB_STRING_ARR(TSURL,TSURL,surl);

void glite_freeStringArray(int i, char** ptrArr)
{
	for(;i>0;i--)
	{
		free(ptrArr[i-1]);
		ptrArr[i-1]=NULL;
	};
	free(ptrArr);
	ptrArr = NULL;
}	

/* Convert gSOAP's ArrayOf_USCOREsoapenc_USCOREstring to char ** */
char **from_soap_StringArray(struct srm2__ArrayOf_USCORExsd_USCOREstring *resp, int *resultCount)
{
	char **result;
	int64_t i;

	/* The result may be empty */
	if (!resp)
	{
		if (resultCount)
			*resultCount = 0;
		return NULL;
	}

	result = malloc(resp->__sizestringArray * sizeof(*result));
	if (resp->__sizestringArray && !result)
	{
//		err_outofmemory(ctx);
		return NULL;
	}

	for (i = 0; i < resp->__sizestringArray; i++)
	{
		if (resp->stringArray[i])
		{
			result[i] = strdup(resp->stringArray[i]);
			if (!result[i])
			{
//				err_outofmemory(ctx);
				glite_freeStringArray(i, result);
				return NULL;
			}
		}
		else
			result[i] = NULL;
	}

	if (resultCount)
		*resultCount = resp->__sizestringArray;

	return result;
}


/*struct srm2__ArrayOfTRequestToken
{
	int __sizerequestTokenArray;	 sequence of elements <requestTokenArray> 
	struct srm2__TRequestToken **requestTokenArray;
};
*/
DEF_GFALCONV_HEADERS_OUT(ArrayOfTRequestToken)
{
#warning impl	
}

static void from_soap_time(struct timespec *ts, int64_t time)
{
	ts->tv_sec = time / 1000;
	ts->tv_nsec = (time - ts->tv_sec * 1000) * 1000000;
}
/*end of fts_soapconv.c*/

DEF_GFALCONV_HEADERS_PTRIN(TSURL)
{
	struct srm2__TSURL* res;
	//check
	SOAP_MAIN_ALLOC;	
	STRING_TO_SOAP(_elem,res->value);
	return res;	
}

DEF_GFALCONV_HEADERS_PTROUT(TSURL)
{
	TSURL ret;
	if(!_elem || !_elem->value) return NULL;
	STRING_FROM_SOAP(_elem->value,ret);
	return ret;
}

void freeType_TSURL(TSURL _arg)
{
	if(_arg)
	{
			END_GFAL_FREE;
	};
}

void freeArray_TSURL(int i, TSURL* ptrArr)
{	for(;i>0;i--)
	{
	if(ptrArr[i-1]) 
		free(&ptrArr[i-1]);
	};
}

TSURL get_TSURL_TSurl(TSURL a) {return a;}

DEF_GFALCONV_HEADERS_IN(ArrayOfTSURL)
{
	GEN_ARRAY_CONV2SOAP(TSURL,__sizesurlArray, surlArray);
	return res;		
}
CONSTRUCTOR_ARRAY_DEF(TSURL,surl);

DEF_GFALCONV_HEADERS_OUT(ArrayOfTSURL)
{
	GEN_ARRAY_CONV2GFAL(TSURL,__sizesurlArray, surlArray);
	return ret;		
}

GFAL_FREEARRAY_TYPE_DEF(TSURL,__sizesurlArray, surlArray);
getARRSURL_DEF(TSURL,surlArray);

DEF_GFALCONV_HEADERS_IN(TSURLInfo)
{
	// default init
	struct srm2__TSURLInfo* res;	
	//check
	SOAP_MAIN_ALLOC;
	
	//sub alloc
	SOAP_PTR_ALLOC(SURLOrStFN);
	SOAP_PTR_ALLOC(storageSystemInfo);
	
	//copying strngs
	STRING_TO_SOAP_EMB(SURLOrStFN);
	STRING_TO_SOAP_EMB(storageSystemInfo);	
	
	return res;	
};	

#define FREE_SUB_OBJ(_name) 	if(_arg->_name)free((void*)_arg->_name);

SRM2_GFAL_FREE_TYPE_DEC(TSURLInfo)
{
	if(!_arg)return;
	FREE_SUB_OBJ(SURLOrStFN);
	FREE_SUB_OBJ(storageSystemInfo);
	END_GFAL_FREE;	
}

getSURL_DEF(TSURLInfo,SURLOrStFN);

CONSTRUCTOR_DEC(TSURLInfo,TSURL t1, TStorageSystemInfo t2)
{
    TSURLInfo* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
    
    tdata1->SURLOrStFN =  strdup(t1);
    GCuAssertPtrNotNull(tdata1->SURLOrStFN);    
    tdata1->storageSystemInfo = strdup(t2);
    GCuAssertPtrNotNull(tdata1->storageSystemInfo);
	return tdata1;
}


DEF_GFALCONV_HEADERS_IN(ArrayOfTSURLInfo)
{ 
	GEN_ARRAY_CONV2SOAP(TSURLInfo,__sizesurlInfoArray, surlInfoArray);
	return res; 
}

CONSTRUCTOR_ARRAY_DEF(TSURLInfo,surlInfo);
GFAL_FREEARRAY_TYPE_DEF(TSURLInfo,__sizesurlInfoArray, surlInfoArray);	
getARRSURL_DEF(TSURLInfo,surlInfoArray);

/* TGetFileRequest: 
typedef struct _TGetFileRequest
{
	TDirOption* dirOption;
	TFileStorageType fileStorageType;
	TSURLInfo* fromSURLInfo;
	TLifeTimeInSeconds lifetime;
	TSpaceToken spaceToken;
} TGetFileRequest; 
*/
DEF_GFALCONV_HEADERS_IN(TGetFileRequest)
{ 
	struct srm2__TGetFileRequest* res;	
	//check
	SOAP_MAIN_ALLOC;
	
	//sub alloc	
//	SOAP_PTR_ALLOC(fileStorageType);
	SOAP_PTR_ALLOC(lifetime);
	SOAP_PTR_ALLOC(spaceToken);	
	//copying 
	CONV2SOAP_OBJ(TDirOption,dirOption);
	res->fileStorageType=convEnum2soap_TFileStorageType(soap,_elem->fileStorageType);	
	CONV2SOAP_OBJ(TSURLInfo,fromSURLInfo); //type caller convention
	NUM_TO_SOAP_EMB(lifetime);
	STRING_TO_SOAP_EMB(spaceToken);		

	return res;	
}
CONSTRUCTOR_DEC(TGetFileRequest,
	TDirOption* dirOption,
	TFileStorageType fileStorageType,
	TSURLInfo* fromSURLInfo,
	TLifeTimeInSeconds lifetime,
	TSpaceToken spaceToken)
{
    TGetFileRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
    
    GCuAssertPtrNotNull(dirOption);
    tdata1->dirOption = dirOption;
	tdata1->fileStorageType = fileStorageType;
	
	GCuAssertPtrNotNull(fromSURLInfo);
	tdata1->fromSURLInfo = fromSURLInfo;
	tdata1->lifetime = lifetime;
	tdata1->spaceToken = spaceToken;
	
	return tdata1;
		
}


SRM2_GFAL_FREE_TYPE_DEC(TGetFileRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(dirOption);
	freeType_TSURLInfo(_arg->fromSURLInfo);
	FREE_SUB_OBJ(spaceToken);
	END_GFAL_FREE;	
}

getSUBSURL_DEF(TGetFileRequest,TSURLInfo,fromSURLInfo);

/* ArrayOfTGetFileRequest: */
/*struct ArrayOfTGetFileRequest
{
	int __sizegetRequestArray;	 sequence of elements <getRequestArray> 
	struct TGetFileRequest **getRequestArray;
};*/
DEF_GFALCONV_HEADERS_IN(ArrayOfTGetFileRequest)
{ 
	GEN_ARRAY_CONV2SOAP(TGetFileRequest,__sizegetRequestArray, getRequestArray); 
	return res;
}

GFAL_FREEARRAY_TYPE_DEF(TGetFileRequest,__sizegetRequestArray, getRequestArray);	
getARRSURL_DEF(TGetFileRequest,getRequestArray);



/* TPutFileRequest: */
/*typedef struct _TPutFileRequest
{
	TFileStorageType fileStorageType;
	TSizeInBytes knownSizeOfThisFile;
	TLifeTimeInSeconds lifetime;
	TSpaceToken spaceToken;
	TSURLInfo* toSURLInfo;
} TPutFileRequest;

};

struct srm2__TPutFileRequest
{
	enum srm2__TFileStorageType *fileStorageType;
	struct srm2__TSizeInBytes *knownSizeOfThisFile;
	struct srm2__TLifeTimeInSeconds *lifetime;
	struct srm2__TSpaceToken *spaceToken;
	struct srm2__TSURLInfo *toSURLInfo;
};

*/

DEF_GFALCONV_HEADERS_IN(TPutFileRequest)
{ 
	 
	struct srm2__TPutFileRequest* res;	
	//check
	SOAP_MAIN_ALLOC;
	
	//sub alloc
//	SOAP_PTR_ALLOC(fileStorageType);
	SOAP_PTR_ALLOC(knownSizeOfThisFile);
	SOAP_PTR_ALLOC(lifetime);
	SOAP_PTR_ALLOC(spaceToken);
//	SOAP_PTR_ALLOC(toSURLInfo);	
	
	//copying 
	res->fileStorageType=convEnum2soap_TFileStorageType(soap,_elem->fileStorageType);	
	NUM_TO_SOAP_EMB(knownSizeOfThisFile);
	NUM_TO_SOAP_EMB(lifetime);
	STRING_TO_SOAP_EMB(spaceToken);	
	CONV2SOAP_OBJ(TSURLInfo,toSURLInfo); //type caller convention
	return res;	
}

CONSTRUCTOR_DEC(TPutFileRequest,
	TFileStorageType fileStorageType,
	TSizeInBytes knownSizeOfThisFile,
	TLifeTimeInSeconds lifetime,
	TSpaceToken spaceToken,
	TSURLInfo* toSURLInfo
	)
{
    TPutFileRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
    
	tdata1->fileStorageType = fileStorageType;
	tdata1->knownSizeOfThisFile = knownSizeOfThisFile;
	GCuAssertPtrNotNull(toSURLInfo);
	tdata1->toSURLInfo = toSURLInfo;
	tdata1->lifetime = lifetime;
	tdata1->spaceToken = spaceToken;
	return tdata1;
}


SRM2_GFAL_FREE_TYPE_DEC(TPutFileRequest)
{
	if(!_arg)return;
	freeType_TSURLInfo(_arg->toSURLInfo);
	FREE_SUB_OBJ(spaceToken);
	END_GFAL_FREE;	
}

getSUBSURL_DEF(TPutFileRequest,TSURLInfo,toSURLInfo);

/* ArrayOfTPutFileRequest: */
/*struct ArrayOfTPutFileRequest
{
	int __sizeputRequestArray;	 sequence of elements <putRequestArray> 
	struct TPutFileRequest **putRequestArray;
};*/

DEF_GFALCONV_HEADERS_IN(ArrayOfTPutFileRequest)
{ 
	GEN_ARRAY_CONV2SOAP(TPutFileRequest,__sizeputRequestArray, putRequestArray); 
	return res;
}

GFAL_FREEARRAY_TYPE_DEF(TPutFileRequest,__sizeputRequestArray, putRequestArray);
getARRSURL_DEF(TPutFileRequest,putRequestArray);

// srmReleaseFiles types: TUserID, TSURL, Boolean, TReturnStataus, TSURLReturnStatus

// srmPrepareToGet

/*typedef struct _srmPrepareToGetRequest
{
	TUserID userID;
	ArrayOfTGetFileRequest* arrayOfFileRequests;
	int nTransferProtocols;
	char** arrayOfTransferProtocols;
	char *userRequestDescription;
	TStorageSystemInfo storageSystemInfo;
	TLifeTimeInSeconds totalRetryTime;
} srmPrepareToGetRequest;


struct srm2__srmPrepareToGetRequest
{
	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTGetFileRequest *arrayOfFileRequests;
	struct srm2__ArrayOf_USCORExsd_USCOREstring *arrayOfTransferProtocols;
	char *userRequestDescription;
	struct srm2__TStorageSystemInfo *storageSystemInfo;
	struct srm2__TLifeTimeInSeconds *totalRetryTime;
};
*/
DEF_GFALCONV_HEADERS_IN(srmPrepareToGetRequest)
{ 
	struct srm2__srmPrepareToGetRequest* res;	
	//check
	SOAP_MAIN_ALLOC;
	
	//sub alloc
	SOAP_PTR_ALLOC(userID);
	SOAP_PTR_ARRAY_ALLOC(arrayOfTransferProtocols,nTransferProtocols);
	SOAP_PTR_ALLOC(storageSystemInfo);
	SOAP_PTR_ALLOC(totalRetryTime);
	
	//copying strngs
	//	TUSerID
	STRING_TO_SOAP_EMB(userID);	
	//	ArrayOfTPutFileRequest	
	CONV2SOAP_OBJ(ArrayOfTGetFileRequest,arrayOfFileRequests);
	//	arrayOfTransferProtocols
	to_soap_StringArray(soap,res->arrayOfTransferProtocols,_elem->nTransferProtocols, (const char**)_elem->arrayOfTransferProtocols);
	//	char* userRequestDescription	
	STRING_TO_SOAP(_elem->userRequestDescription,res->userRequestDescription);
	//	TStorageSystemInfo storageSystemInfo;
	STRING_TO_SOAP((char*)_elem->storageSystemInfo,res->storageSystemInfo->value);	
	//	TLifeTimeInSeconds totalRetryTime;	
	NUM_TO_SOAP_EMB(totalRetryTime);
	return res;	
}

CONSTRUCTOR_DEC(srmPrepareToGetRequest,
	TUserID userID,
	ArrayOfTGetFileRequest* arrayOfFileRequests,
	int nTransferProtocols,
	char** arrayOfTransferProtocols,
	char *userRequestDescription,
	TStorageSystemInfo storageSystemInfo,
	TLifeTimeInSeconds totalRetryTime)
{
    srmPrepareToGetRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);   
    
    COPY_CHK(userID);
	COPY_CHK(arrayOfFileRequests);
	COPY_CHK(nTransferProtocols);
	COPY_CHK(arrayOfTransferProtocols);
	COPY_CHK(userRequestDescription);
	COPY_CHK(storageSystemInfo);
	COPY_CHK(totalRetryTime);
	
	return tdata1;
	
}	


SRM2_GFAL_FREE_TYPE_DEC(srmPrepareToGetRequest)
{
	if(!_arg)return;

	FREE_SUB_OBJ(userID);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTGetFileRequest,_arg->arrayOfFileRequests);
	//free char array arrayOfTransferProtocols	
	glite_freeStringArray(_arg->nTransferProtocols,_arg->arrayOfTransferProtocols);
	FREE_SUB_OBJ(userRequestDescription);
	FREE_SUB_OBJ(storageSystemInfo);	
	END_GFAL_FREE;	
		
}

getSUBSURL_DEF(srmPrepareToGetRequest,ArrayOfTGetFileRequest,arrayOfFileRequests);

/*
 * typedef struct _TReturnStatus
{
	char *explanation;
	TStatusCode statusCode;
} TReturnStatus;

struct srm2__TReturnStatus
{
	char *explanation;
	enum srm2__TStatusCode statusCode;
};
*/

DEF_GFALCONV_HEADERS_OUT(TReturnStatus)
{
	GFAL_DECL_ALLOC(TReturnStatus);
	//copying
	STRING_FROM_SOAP(_elem->explanation,res->explanation);
	res->statusCode = _elem->statusCode;
	return res;
}
SRM2_GFAL_FREE_TYPE_DEC(TReturnStatus)
{
	if(!_arg)return;
	FREE_SUB_OBJ(explanation);
	END_GFAL_FREE;	
}



// srmPrepareToGetResponse: 
/*struct srmPrepareToGetResponse
{
	struct TRequestToken *requestToken;
	struct TReturnStatus *returnStatus;
	struct ArrayOfTGetRequestFileStatus *arrayOfFileStatuses;
};*/
DEF_GFALCONV_HEADERS_OUT(srmPrepareToGetResponse)
{ 
	GFAL_DECL_ALLOC(srmPrepareToGetResponse);
	//copying
	STRING_FROM_SOAP_EMB(requestToken);
	res->returnStatus = conv2gfal_TReturnStatus(_elem->returnStatus);
	CONV2GFAL_OBJ(ArrayOfTGetRequestFileStatus,arrayOfFileStatuses);
	return res;
}

#warning impl of free
//=============================
// srmPrepareToPut
/* srmPrepareToPutRequest: */
/*typedef struct _srmPrepareToPutRequest
{
	TUserID userID;
	ArrayOfTPutFileRequest* arrayOfFileRequests;
	int nTransferProtocols;
	char** arrayOfTransferProtocols;
	char *userRequestDescription;
	TOverwriteMode overwriteOption;
	TStorageSystemInfo storageSystemInfo; --char*
	TLifeTimeInSeconds totalRetryTime; --long64
} srmPrepareToPutRequest;
*/


DEF_GFALCONV_HEADERS_IN(srmPrepareToPutRequest)
{ 
	struct srm2__srmPrepareToPutRequest* res;	
	//check
	SOAP_MAIN_ALLOC;
	
	//sub alloc
	SOAP_PTR_ALLOC(userID);
	SOAP_PTR_ALLOC(arrayOfTransferProtocols);
	SOAP_PTR_ALLOC(storageSystemInfo);
	SOAP_PTR_ALLOC(totalRetryTime);

	
	//copying strngs
	//	TUSerID
	STRING_TO_SOAP_EMB(userID);
	
	//	ArrayOfTPutFileRequest	
	CONV2SOAP_OBJ(ArrayOfTPutFileRequest,arrayOfFileRequests);
	//	arrayOfTransferProtocols
	to_soap_StringArray(soap,res->arrayOfTransferProtocols,_elem->nTransferProtocols, (const char**)_elem->arrayOfTransferProtocols);
	//	char* userRequestDescription	
	STRING_TO_SOAP(_elem->userRequestDescription,res->userRequestDescription);
	//	TOverwriteMode overwriteOption;
	res->overwriteOption=convEnum2soap_TOverwriteMode(soap,_elem->overwriteOption);	
	//	TStorageSystemInfo storageSystemInfo;
	STRING_TO_SOAP((char*)_elem->storageSystemInfo,res->storageSystemInfo->value);	
	//	TLifeTimeInSeconds totalRetryTime;	
	NUM_TO_SOAP_EMB(totalRetryTime);
	return res;	

}


CONSTRUCTOR_DEC(srmPrepareToPutRequest,
	TUserID userID,
	ArrayOfTPutFileRequest* arrayOfFileRequests,
	int nTransferProtocols,
	char** arrayOfTransferProtocols,
	char *userRequestDescription,
	TOverwriteMode overwriteOption,
	TStorageSystemInfo storageSystemInfo,
	TLifeTimeInSeconds totalRetryTime)
{
		
	srmPrepareToPutRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
    
    COPY_CHK(userID);
    COPY_CHK(arrayOfFileRequests);
    COPY_CHK(nTransferProtocols);
    COPY_CHK(arrayOfTransferProtocols);
    COPY_CHK(userRequestDescription);
    COPY_CHK(overwriteOption);
    COPY_CHK(storageSystemInfo);
    COPY_CHK(totalRetryTime);
    
    return tdata1;
}


SRM2_GFAL_FREE_TYPE_DEC(srmPrepareToPutRequest)
{
	if(!_arg)return;

	FREE_SUB_OBJ(userID);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTPutFileRequest,_arg->arrayOfFileRequests);
	//free char array arrayOfTransferProtocols	
	glite_freeStringArray(_arg->nTransferProtocols,_arg->arrayOfTransferProtocols);
	FREE_SUB_OBJ(userRequestDescription);
	FREE_SUB_OBJ(storageSystemInfo);	
	END_GFAL_FREE;	
}

getSUBSURL_DEF(srmPrepareToPutRequest,ArrayOfTPutFileRequest,arrayOfFileRequests);

/* TPutRequestFileStatus: 
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

*/
/* srm2:TPutRequestFileStatus: */
/*struct srm2__TPutRequestFileStatus
{
	struct srm2__TLifeTimeInSeconds *estimatedProcessingTime;
	struct srm2__TLifeTimeInSeconds *estimatedWaitTimeOnQueue;
	struct srm2__TSizeInBytes *fileSize;
	struct srm2__TLifeTimeInSeconds *remainingPinTime;
	struct srm2__TSURL *siteURL;
	struct srm2__TReturnStatus *status;
	struct srm2__TTURL *transferURL;
};*/	

DEF_GFALCONV_HEADERS_OUT(TPutRequestFileStatus)
{	
	GFAL_DECL_ALLOC(TPutRequestFileStatus);
	//copying
	NUM_FROM_SOAP(estimatedProcessingTime);
	NUM_FROM_SOAP(estimatedWaitTimeOnQueue);
	NUM_FROM_SOAP(fileSize);
	NUM_FROM_SOAP(remainingPinTime);
	STRING_FROM_SOAP_EMB(siteURL);
	res->status = conv2gfal_TReturnStatus(_elem->status);
	STRING_FROM_SOAP_EMB(transferURL);
	
	return res;

}

SRM2_GFAL_FREE_TYPE_DEC(TPutRequestFileStatus)
{
	if(!_arg)return;

	FREE_SUB_OBJ(siteURL);
	freeType_TReturnStatus(_arg->status);
	FREE_SUB_OBJ(transferURL);
	END_GFAL_FREE;	
}


GFAL_FREEARRAY_TYPE_DEF(TPutRequestFileStatus,__sizeputStatusArray, putStatusArray);


DEF_GFALCONV_HEADERS_OUT(ArrayOfTPutRequestFileStatus)
{	
	GEN_ARRAY_CONV2GFAL(TPutRequestFileStatus,__sizeputStatusArray,putStatusArray);
	return ret;
}	

// srmPrepareToPutResponse: 
/*typedef struct _srmPrepareToPutResponse
{
	TRequestToken requestToken;
	TReturnStatus* returnStatus;
	ArrayOfTPutRequestFileStatus* arrayOfFileStatuses;
} srmPrepareToPutResponse;

*/


DEF_GFALCONV_HEADERS_OUT(srmPrepareToPutResponse)
{ 
	GFAL_DECL_ALLOC(srmPrepareToPutResponse);
	//copying
	STRING_FROM_SOAP_EMB(requestToken);
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
	CONV2GFAL_OBJ(ArrayOfTPutRequestFileStatus,arrayOfFileStatuses);
	return res;
}

SRM2_GFAL_FREE_TYPE_DEC(srmPrepareToPutResponse)
{
	if(!_arg)return;

	FREE_SUB_OBJ(requestToken);
	freeType_TReturnStatus(_arg->returnStatus);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTPutRequestFileStatus,_arg->arrayOfFileStatuses);
	END_GFAL_FREE;	
}

// srmPutDone
/* srmPutDoneRequest: */
/*	struct srm2__TRequestToken *requestToken;
	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTSURL *arrayOfSiteURLs;
*/
DEF_GFALCONV_HEADERS_IN(srmPutDoneRequest)
{ 
	struct srm2__srmPutDoneRequest* res;
	//check
	SOAP_MAIN_ALLOC;
	
	//sub alloc
	SOAP_PTR_ALLOC(userID);
	SOAP_PTR_ALLOC(requestToken);	
	
	//copying strngs
	//	TUSerID
	STRING_TO_SOAP_EMB(userID);	
	//	ArrayOfTPutFileRequest	
	CONV2SOAP_OBJ(ArrayOfTSURL,arrayOfSiteURLs);
	//	char* requestToken
	STRING_TO_SOAP_EMB(requestToken);
	return res;		
}

SRM2_GFAL_FREE_TYPE_DEC(srmPutDoneRequest)
{
	if(!_arg)return;

	FREE_SUB_OBJ(userID);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURL,_arg->arrayOfSiteURLs);
	FREE_SUB_OBJ(requestToken);	
	END_GFAL_FREE;	
}

getSUBSURL_DEF(srmPutDoneRequest,ArrayOfTSURL,arrayOfSiteURLs);

// srmPutDoneResponse: 
/*struct srmPutDoneResponse
{
	struct TReturnStatus *returnStatus;
	struct ArrayOfTSURLReturnStatus *arrayOfFileStatuses;
};*/
DEF_GFALCONV_HEADERS_OUT(srmPutDoneResponse)
{ 
	GFAL_DECL_ALLOC(srmPutDoneResponse);
	//copying
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
	return res;
}

SRM2_GFAL_FREE_TYPE_DEC(srmPutDoneResponse)
{
	if(!_arg)return;	
	freeType_TReturnStatus(_arg->returnStatus);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURLReturnStatus,_arg->arrayOfFileStatuses);
	END_GFAL_FREE;	
}


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
// srmAbortFilesRequest: 
/*struct srmAbortFilesRequest
{
	struct TRequestToken *requestToken;
	struct ArrayOfTSURL *arrayOfSiteURLs;
	struct TUserID *userID;
};*/
DEF_GFALCONV_HEADERS_IN(srmAbortFilesRequest)
{ 
	struct srm2__srmAbortFilesRequest* res;
	//check
	SOAP_MAIN_ALLOC;
	
	//sub alloc
	SOAP_PTR_ALLOC(userID);
	SOAP_PTR_ALLOC(requestToken);	
	
	//copying strngs
	//	TUSerID
	STRING_TO_SOAP_EMB(userID);
	
	//	ArrayOfTPutFileRequest	
	CONV2SOAP_OBJ(ArrayOfTSURL,arrayOfSiteURLs);

	//	char* requestToken
	STRING_TO_SOAP_EMB(requestToken);
	return res;	
}



// srmAbortFilesResponse: 
/*struct srmAbortFilesResponse
{
	struct TReturnStatus *returnStatus;
	struct ArrayOfTSURLReturnStatus *arrayOfFileStatuses;
};*/
DEF_GFALCONV_HEADERS_OUT(srmAbortFilesResponse)
{ 
	GFAL_DECL_ALLOC(srmAbortFilesResponse);
	//copying
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
	return res;

}


// srmStatusOfGetRequest

// srmStatusOfGetRequestRequest: 
/*struct srmStatusOfGetRequestRequest
{
	struct TRequestToken *requestToken;
	struct TUserID *userID;
	struct ArrayOfTSURL *arrayOfFromSURLs;
};*/
DEF_GFALCONV_HEADERS_IN(srmStatusOfGetRequestRequest)
{ 
	struct srm2__srmStatusOfGetRequestRequest* res;
	//check
	SOAP_MAIN_ALLOC;
	
	//sub alloc
	SOAP_PTR_ALLOC(userID);
	SOAP_PTR_ALLOC(requestToken);		
	//copying strngs
	//	TUSerID
	STRING_TO_SOAP_EMB(userID);	
	//	ArrayOfTPutFileRequest	
	CONV2SOAP_OBJ(ArrayOfTSURL,arrayOfFromSURLs);

	//	char* requestToken
	STRING_TO_SOAP_EMB(requestToken);
	return res;	
}

SRM2_GFAL_FREE_TYPE_DEC(srmStatusOfGetRequestRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(userID);
	FREE_SUB_OBJ(requestToken);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURL,_arg->arrayOfFromSURLs);	
	END_GFAL_FREE;
}
getSUBSURL_DEF(srmStatusOfGetRequestRequest,ArrayOfTSURL,arrayOfFromSURLs);

// srmStatusOfGetRequestResponse: 
/*struct srmStatusOfGetRequestResponse
{
	struct TReturnStatus *returnStatus;
	struct ArrayOfTGetRequestFileStatus *arrayOfFileStatuses;
};*/

DEF_GFALCONV_HEADERS_OUT(srmStatusOfGetRequestResponse)
{ 
	GFAL_DECL_ALLOC(srmStatusOfGetRequestResponse);
	//copying
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
	CONV2GFAL_OBJ(ArrayOfTGetRequestFileStatus,arrayOfFileStatuses);
	return res;		
}


// srmStatusOfPutRequest
// srmStatusOfPutRequestRequest: 
/*struct srm2__srmStatusOfPutRequest
{
	struct srm2__srmStatusOfPutRequestRequest *srmStatusOfPutRequestRequest;
};
*/
/*struct srmStatusOfPutRequestRequest
{
	TRequestToken requestToken;
	TUserID userID;
	ArrayOfTStrings arrayOfToSURLs;
};
struct srm2__srmStatusOfPutRequestRequest
{
	struct srm2__TRequestToken *requestToken;
	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTSURL *arrayOfToSURLs;
};
*/

DEF_GFALCONV_HEADERS_IN(srmStatusOfPutRequestRequest)
{ 
	struct srm2__srmStatusOfPutRequestRequest* res;
	//check
	SOAP_MAIN_ALLOC;
	
	//sub alloc
	SOAP_PTR_ALLOC(userID);
	SOAP_PTR_ALLOC(requestToken);	
	
	//copying strngs
	//	TUSerID
	STRING_TO_SOAP_EMB(userID);	
	//	ArrayOfTPutFileRequest	
	CONV2SOAP_OBJ(ArrayOfTSURL,arrayOfToSURLs);
	//	char* requestToken
	STRING_TO_SOAP_EMB(requestToken);
	return res;	

}
SRM2_GFAL_FREE_TYPE_DEC(srmStatusOfPutRequestRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(userID);
	FREE_SUB_OBJ(requestToken);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURL,_arg->arrayOfToSURLs);	
	END_GFAL_FREE;
}
getSUBSURL_DEF(srmStatusOfPutRequestRequest,ArrayOfTSURL,arrayOfToSURLs);

// srmStatusOfPutRequestResponse: 
/*struct srmStatusOfPutRequestResponse
{
	struct TReturnStatus *returnStatus;
	struct ArrayOfTPutRequestFileStatus *arrayOfFileStatuses;
};*/
DEF_GFALCONV_HEADERS_OUT(srmStatusOfPutRequestResponse)
{ 
	GFAL_DECL_ALLOC(srmStatusOfPutRequestResponse);
	//copying
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
	CONV2GFAL_OBJ(ArrayOfTPutRequestFileStatus,arrayOfFileStatuses);
	return res;		
}

SRM2_GFAL_FREE_TYPE_DEC(srmStatusOfPutRequestResponse)
{
	if(!_arg)return;
	freeType_TReturnStatus(_arg->returnStatus);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTPutRequestFileStatus,_arg->arrayOfFileStatuses);	
	END_GFAL_FREE;	
}

// srmGetRequestSummaryRequest: 
/*struct srmGetRequestSummaryRequest
{
	struct ArrayOfTRequestToken *arrayOfRequestTokens;
	struct TUserID *userID;
};*/
DEF_GFALCONV_HEADERS_IN(srmGetRequestSummaryRequest)
{ 
		#warning impl
}


/*typedef struct _TRequestSummary
{
	enum xsd__boolean isSuspended;
	int numOfFinishedRequests;
	int numOfProgressingRequests;
	int numOfQueuedRequests;
	struct TRequestToken *requestToken;
	enum TRequestType *requestType;
	int totalFilesInThisRequest;
} TRequestSummary;
*/
DEF_GFALCONV_HEADERS_OUT(TRequestSummary)
{
		#warning impl	
}

/* ArrayOfTRequestSummary: */
/*typedef struct _ArrayOfTRequestSummary
{
	int __sizesummaryArray;	 sequence of elements <summaryArray> 
	TRequestSummary **summaryArray;
} ArrayOfTRequestSummary;
*/
DEF_GFALCONV_HEADERS_OUT(ArrayOfTRequestSummary)
{
	#warning impl	
}


// srmGetRequestSummaryResponse: 
/*struct srmGetRequestSummaryResponse
{
	struct ArrayOfTRequestSummary *arrayOfRequestSummaries;
	struct TReturnStatus *returnStatus;
};*/
DEF_GFALCONV_HEADERS_OUT(srmGetRequestSummaryResponse)
{ 
	GFAL_DECL_ALLOC(srmGetRequestSummaryResponse);
	//copying
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
	CONV2GFAL_OBJ(ArrayOfTRequestSummary,arrayOfRequestSummaries);
	return res;		
}



// srmGetRequestID
/* srmGetRequestIDRequest: */
/*struct srmGetRequestIDRequest
{
	char *userRequestDescription;
	struct TUserID *userID;
};*/
DEF_GFALCONV_HEADERS_IN(srmGetRequestIDRequest)
{ 
	return 0;
}


/* srmGetRequestIDResponse: */
/*struct srmGetRequestIDResponse
{
	struct ArrayOfTRequestToken *arrayOfRequestTokens;
	struct TReturnStatus *returnStatus;
};*/
DEF_GFALCONV_HEADERS_OUT(srmGetRequestIDResponse)
{ 
	GFAL_DECL_ALLOC(srmGetRequestIDResponse);
	//copying
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);

//	CONV2GFAL_OBJ(ArrayOfTRequestToken,arrayOfRequestTokens);
	return res;		
}

/*typedef struct _TSURLReturnStatus
{
	TReturnStatus* status;
	TSURL  surl;
} TSURLReturnStatus; 
*/
DEF_GFALCONV_HEADERS_OUT(TSURLReturnStatus)
{
	GFAL_DECL_ALLOC(TSURLReturnStatus);
	//copying
	STRING_FROM_SOAP_EMB(surl);
	res->status = conv2gfal_TReturnStatus(_elem->status);	
	return res;
}

GFAL_FREEARRAY_TYPE_DEF(TSURLReturnStatus,__sizesurlReturnStatusArray, surlReturnStatusArray);

SRM2_GFAL_FREE_TYPE_DEC(TSURLReturnStatus)
{
	if(!_arg)return;
	freeType_TReturnStatus(_arg->status);
	FREE_SUB_OBJ(surl);
	END_GFAL_FREE;	
}


/*typedef struct _ArrayOfTSURLReturnStatus
{
	int __sizesurlReturnStatusArray;	 sequence of elements <surlReturnStatusArray> 
	TSURLReturnStatus **surlReturnStatusArray;
} ArrayOfTSURLReturnStatus;
*/
DEF_GFALCONV_HEADERS_OUT(ArrayOfTSURLReturnStatus)
{
	GEN_ARRAY_CONV2GFAL(TSURLReturnStatus,__sizesurlReturnStatusArray,surlReturnStatusArray);
	return ret;
}

/*typedef struct _TGetRequestFileStatus
{
	TLifeTimeInSeconds estimatedProcessingTime;
	TLifeTimeInSeconds estimatedWaitTimeOnQueue;
	TSizeInBytes fileSize;
	TSURL fromSURLInfo;
	TLifeTimeInSeconds remainingPinTime;
	TReturnStatus* status;
	TTURL transferURL;
} TGetRequestFileStatus;
*/
DEF_GFALCONV_HEADERS_OUT(TGetRequestFileStatus)
{
	GFAL_DECL_ALLOC(TGetRequestFileStatus);
	//copying
	NUM_FROM_SOAP(estimatedProcessingTime);	
	NUM_FROM_SOAP(estimatedWaitTimeOnQueue);
	NUM_FROM_SOAP(fileSize);
	STRING_FROM_SOAP_EMB(fromSURLInfo);
	NUM_FROM_SOAP(remainingPinTime);
	CONV2GFAL_OBJ(TReturnStatus,status);
	STRING_FROM_SOAP_EMB(transferURL);	
	return res;		
}
GFAL_FREEARRAY_TYPE_DEF(TGetRequestFileStatus,__sizegetStatusArray, getStatusArray);

SRM2_GFAL_FREE_TYPE_DEC(TGetRequestFileStatus)
{
	if(!_arg)return;

	FREE_SUB_OBJ(fromSURLInfo);
	freeType_TReturnStatus(_arg->status);	
	FREE_SUB_OBJ(transferURL);
	END_GFAL_FREE;	
}



/* ArrayOfTGetRequestFileStatus: */
/*typedef struct _ArrayOfTGetRequestFileStatus
{
	int __sizegetStatusArray;	 sequence of elements <getStatusArray> 
	TGetRequestFileStatus **getStatusArray;
} ArrayOfTGetRequestFileStatus;
*/

DEF_GFALCONV_HEADERS_OUT(ArrayOfTGetRequestFileStatus)
{
	GEN_ARRAY_CONV2GFAL(TGetRequestFileStatus,__sizegetStatusArray,getStatusArray);
}

//permissions
/*
 * typedef struct _TPermission 
{
	TPermissionMode mode;
	char* principal;
} TPermission;
DEF_GFALCONV_HEADERS_OUT(TPermission);
SRM2_GFAL_FREE_TYPE_DEC(TPermission);
 * 
*/

SRM2_GFAL_FREE_TYPE_DEC(TPermission)
{
	if(!_arg)return;
	FREE_SUB_OBJ(principal);
	END_GFAL_FREE;	
}


//principal+enum wrappers
DEF_GFALCONV_HEADERS_OUT_EXT(TPermission,srm2__TUserPermission)
{ 
	GFAL_DECL_ALLOC(TPermission);
	//copying
	STRING_FROM_SOAP(_elem->userID->value,res->principal);
	res->mode=_elem->mode;
	return res;
}

DEF_GFALCONV_HEADERS_OUT_EXT(TPermission,srm2__TGroupPermission)
{
	GFAL_DECL_ALLOC(TPermission);
	//copying
	STRING_FROM_SOAP(_elem->groupID->value,res->principal);
	res->mode=_elem->mode;
	return res;	
}



/*//////////////////////////////////
typedef struct _ArrayOfTPermission;
{
	int __sizepermissionArray;	
	TPermission **permissionArray;
} ArrayOfTPermission;

GFAL_FREEARRAY_TYPE_DEC(ArrayOfTPermission);
*/
GFAL_FREEARRAY_TYPE_DEF(TPermission,__sizepermissionArray, permissionArray);


DEF_GFALCONV_HEADERS_OUT_EXT(ArrayOfTPermission,srm2__ArrayOfTUserPermission)
{
	GEN_ARRAY_CONV2GFAL_EXT(TPermission,__sizepermissionArray,permissionArray,srm2__TUserPermission,__sizeuserPermissionArray,userPermissionArray);
	return ret;	
}
DEF_GFALCONV_HEADERS_OUT_EXT(ArrayOfTPermission,srm2__ArrayOfTGroupPermission)
{
	GEN_ARRAY_CONV2GFAL_EXT(TPermission,__sizepermissionArray,permissionArray,srm2__TGroupPermission,__sizegroupPermissionArray,groupPermissionArray);
	return ret;
}

//metadatapathdetail

/*
 * 
 * 		char *path;
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
	
 * struct srm2__TMetaDataPathDetail
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
 * 
	
*/	//sub alloc

#define ENUM_FROM_SOAP(_n) res->_n = *(_elem->_n);

DEF_GFALCONV_HEADERS_OUT(TMetaDataPathDetail)
{
	GFAL_DECL_ALLOC(TMetaDataPathDetail);

	STRING_FROM_SOAP(_elem->path,res->path);
	STRING_FROM_SOAP_EMB(owner);
	STRING_FROM_SOAP_EMB(checkSumType);
	STRING_FROM_SOAP_EMB(checkSumValue);
	STRING_FROM_SOAP_EMB(originalSURL);	
	
	CONV2GFAL_OBJ(TReturnStatus,status)
	
	res->ownerPermission = _elem->ownerPermission->mode;
	CONV2GFAL_OBJ(ArrayOfTPermission_srm2__ArrayOfTUserPermission,userPermissions);
	CONV2GFAL_OBJ(ArrayOfTPermission_srm2__ArrayOfTGroupPermission,groupPermissions);
	res->otherPermission = _elem->otherPermission->mode;
	
	NUM_FROM_SOAP_EMB(size);
	NUM_FROM_SOAP_EMB(createdAtTime);
	NUM_FROM_SOAP_EMB(lastModificationTime);
	ENUM_FROM_SOAP(fileStorageType);
	ENUM_FROM_SOAP(type);
	NUM_FROM_SOAP_EMB(lifetimeAssigned);
	NUM_FROM_SOAP_EMB(lifetimeLeft);
	
	CONV2GFAL_OBJ(ArrayOfTMetaDataPathDetail,subPaths);
	
	return res;	
}

SRM2_GFAL_FREE_TYPE_DEC(TMetaDataPathDetail)
{
	if(!_arg)return;

	FREE_SUB_OBJ(path);
	freeType_TReturnStatus(_arg->status);

	freegfalArray_ArrayOfTPermission(_arg->userPermissions);
	freegfalArray_ArrayOfTPermission(_arg->groupPermissions);
	
	FREE_SUB_OBJ(owner);
	FREE_SUB_OBJ(checkSumType);
	FREE_SUB_OBJ(checkSumValue);
	FREE_SUB_OBJ(originalSURL);
	freegfalArray_ArrayOfTMetaDataPathDetail(_arg->subPaths);
	END_GFAL_FREE;	
}


DEF_GFALCONV_HEADERS_OUT(ArrayOfTMetaDataPathDetail)
{
	GEN_ARRAY_CONV2GFAL(TMetaDataPathDetail,__sizepathDetailArray,pathDetailArray);
	return ret;
}

GFAL_FREEARRAY_TYPE_DEF(TMetaDataPathDetail,__sizepathDetailArray, pathDetailArray);

//srmls
/*typedef struct _srmLsRequest


/*
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

DEF_GFALCONV_HEADERS_IN(srmLsRequest);
SRM2_GFAL_FREE_TYPE_DEC(srmLsRequest);

struct srm2__srmLsRequest
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


struct srm2__srmLsRequest
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

/*
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
*/
DEF_GFALCONV_HEADERS_IN(srmLsRequest)
{
	struct srm2__srmLsRequest* res;	
	//check
	SOAP_MAIN_ALLOC;
	
	SOAP_PTR_ALLOC(userID);
	SOAP_PTR_ALLOC(fullDetailedList);
	SOAP_PTR_ALLOC(allLevelRecursive);
	SOAP_PTR_ALLOC(numOfLevels);
	SOAP_PTR_ALLOC(offset);
	SOAP_PTR_ALLOC(count);
	
	//copying strngs
	//	TUSerID
	STRING_TO_SOAP_EMB(userID);	
	//	ArrayOfTSURLInfo	
	CONV2SOAP_OBJ(ArrayOfTSURLInfo,paths);
	res->fileStorageType=convEnum2soap_TFileStorageType(soap,_elem->fileStorageType);	
	NUM_TO_SOAP_PTR(fullDetailedList);
	NUM_TO_SOAP_PTR(allLevelRecursive);
	NUM_TO_SOAP_PTR(numOfLevels);
	NUM_TO_SOAP_PTR(offset);
	NUM_TO_SOAP_PTR(count);	
	return res;	
}

CONSTRUCTOR_DEC(srmLsRequest,
	TUserID userID,
	ArrayOfTSURLInfo *paths,
	TFileStorageType fileStorageType,
	short int fullDetailedList,	//boolean
	short int allLevelRecursive,	//boolean
	int numOfLevels,
	int offset,
	int count
)
{
    srmLsRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
    
    COPY_CHK(userID);
	COPY_CHK(paths);
	COPY_CHK(fileStorageType);
	COPY_CHK(fullDetailedList);
	COPY_CHK(allLevelRecursive);
	COPY_CHK(numOfLevels);
	COPY_CHK(offset)
	COPY_CHK(count);
	
	return tdata1;	
}



SRM2_GFAL_FREE_TYPE_DEC(srmLsRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(userID);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURLInfo,_arg->paths);
	END_GFAL_FREE;
}

getSURL_DEC(srmLsRequest)
{
	if(!_arg)return NULL;
	return get_ArrayOfTSURLInfo_TSurl(_arg->paths);
}


/*
 * getSURL_DEF(srmLsRequest,paths->surlInfoArray[0]
 * 	char* get_##_n##_TSurl(_n* arg){	\
		if(!arg) return NULL;		\
		return get_ArrayOfTSURLInfo_TSurl(paths);
		
	};
*/
/*
 * /*
 srm2:srmLsResponse: 
struct srm2__srmLsResponse
{
	struct srm2__ArrayOfTMetaDataPathDetail *details;
	struct srm2__TReturnStatus *returnStatus;
};
*
 * 
 * typedef struct _srmLsResponse
{
	ArrayOfTMetaDataPathDetail *details;
	TReturnStatus *returnStatus;
} srmLsResponse;

DEF_GFALCONV_HEADERS_OUT(srmLsResponse);
SRM2_GFAL_FREE_TYPE_DEC(srmLsResponse);

*/

DEF_GFALCONV_HEADERS_OUT(srmLsResponse)
{
	GFAL_DECL_ALLOC(srmLsResponse);
	//copying	
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
	CONV2GFAL_OBJ(ArrayOfTMetaDataPathDetail,details);
	return res;
}

SRM2_GFAL_FREE_TYPE_DEC(srmLsResponse)
{
	if(!_arg)return;

	freeType_TReturnStatus(_arg->returnStatus);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTMetaDataPathDetail,_arg->details);	
	END_GFAL_FREE;
}


//srmCheckPermission


/*
typedef struct _srmCheckPermissionRequest
{
	ArrayOfTSURLInfo *arrayOfSiteURLs;
	TUserID userID;
	short int checkInLocalCacheOnly;	
} srmCheckPermissionRequest;

struct srm2__srmCheckPermissionRequest
{
	struct srm2__ArrayOfTSURLInfo *arrayOfSiteURLs;
	struct srm2__TUserID *userID;
	enum xsd__boolean *checkInLocalCacheOnly;
};

*/

DEF_GFALCONV_HEADERS_IN(srmCheckPermissionRequest)
{
	struct srm2__srmCheckPermissionRequest* res;	
	//check
	SOAP_MAIN_ALLOC;
	
	SOAP_PTR_ALLOC(userID);
	SOAP_PTR_ALLOC(checkInLocalCacheOnly);

	//copying strngs
	//	TUSerID
	STRING_TO_SOAP_EMB(userID);	
	//	ArrayOfTSURLInfo	
	CONV2SOAP_OBJ(ArrayOfTSURLInfo,arrayOfSiteURLs);
	NUM_TO_SOAP_PTR(checkInLocalCacheOnly);	
	return res;	
}
CONSTRUCTOR_DEC(srmCheckPermissionRequest,	
	TUserID userID,
	ArrayOfTSURLInfo *arrayOfSiteURLs,
	short int checkInLocalCacheOnly	
)
{
    srmCheckPermissionRequest* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
    
    COPY_CHK(userID);
	COPY_CHK(arrayOfSiteURLs);
	COPY_CHK(checkInLocalCacheOnly);	
	return tdata1;	
}


SRM2_GFAL_FREE_TYPE_DEC(srmCheckPermissionRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(userID);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURLInfo,_arg->arrayOfSiteURLs);	
	END_GFAL_FREE;
}

getSURL_DEC(srmCheckPermissionRequest)
{
	if(!_arg)return NULL;
	return get_ArrayOfTSURLInfo_TSurl(_arg->arrayOfSiteURLs);
}



/*typedef struct _TSURLPermissionReturn
{
	TReturnStatus *status;
	TSURL surl;
	enum TPermissionMode userPermission;
} TSURLPermissionReturn;
*/

/*	GFAL_DECL_ALLOC(TMetaDataPathDetail);

	STRING_FROM_SOAP(_elem->path,res->path);
	STRING_FROM_SOAP_EMB(owner);
	STRING_FROM_SOAP_EMB(checkSumType);
	STRING_FROM_SOAP_EMB(checkSumValue);
	STRING_FROM_SOAP_EMB(originalSURL);	
	
	CONV2GFAL_OBJ(TReturnStatus,status)
	
	res->ownerPermission = _elem->ownerPermission->mode;
	CONV2GFAL_OBJ(ArrayOfTPermission_srm2__ArrayOfTUserPermission,userPermissions);
	CONV2GFAL_OBJ(ArrayOfTPermission_srm2__ArrayOfTGroupPermission,groupPermissions);
	res->otherPermission = _elem->otherPermission->mode;
	
	NUM_FROM_SOAP_EMB(size);
*/
DEF_GFALCONV_HEADERS_OUT(TSURLPermissionReturn)
{
	GFAL_DECL_ALLOC(TSURLPermissionReturn);
	//copying	
	STRING_FROM_SOAP_EMB(surl);
	CONV2GFAL_OBJ(TReturnStatus,status);
	res->userPermission = *_elem->userPermission;
	return res;
}

SRM2_GFAL_FREE_TYPE_DEC(TSURLPermissionReturn)
{
	if(!_arg)return;

	freeType_TReturnStatus(_arg->status);
	FREE_SUB_OBJ(surl);
	END_GFAL_FREE;	
}

/*
 srm2:ArrayOfTSURLPermissionReturn: 
struct srm2__ArrayOfTSURLPermissionReturn
{
	int __sizesurlPermissionArray;	 sequence of elements <surlPermissionArray> 
	struct srm2__TSURLPermissionReturn **surlPermissionArray;
};
*/

DEF_GFALCONV_HEADERS_OUT(ArrayOfTSURLPermissionReturn)
{
	GEN_ARRAY_CONV2GFAL(TSURLPermissionReturn,__sizesurlPermissionArray,surlPermissionArray);
	return ret;
}

GFAL_FREEARRAY_TYPE_DEF(TSURLPermissionReturn,__sizesurlPermissionArray, surlPermissionArray);


/* srm2:srmCheckPermissionResponse: 
struct srm2__srmCheckPermissionResponse
{
	struct srm2__ArrayOfTSURLPermissionReturn *arrayOfPermissions;
	struct srm2__TReturnStatus *returnStatus;
};
*/

/*typedef struct _srmCheckPermissionResponse
{
	ArrayOfTSURLPermissionReturn *arrayOfPermissions;
	TReturnStatus *returnStatus;
} srmCheckPermissionResponse;
*/


DEF_GFALCONV_HEADERS_OUT(srmCheckPermissionResponse)
{
	GFAL_DECL_ALLOC(srmCheckPermissionResponse);
	//copying	
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
	CONV2GFAL_OBJ(ArrayOfTSURLPermissionReturn,arrayOfPermissions);
	return res;
}

SRM2_GFAL_FREE_TYPE_DEC(srmCheckPermissionResponse)
{
	if(!_arg)return;
	freeType_TReturnStatus(_arg->returnStatus);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURLPermissionReturn,_arg->arrayOfPermissions);	
	END_GFAL_FREE;	
}

/*struct srm2__srmReleaseFilesRequest
{
	struct srm2__TRequestToken *requestToken;
	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTSURL *siteURLs;
	enum xsd__boolean *keepSpace;
};
*/
DEF_GFALCONV_HEADERS_IN(srmReleaseFilesRequest)
{
	struct srm2__srmReleaseFilesRequest* res;	
	//check
	SOAP_MAIN_ALLOC;
	
	SOAP_PTR_ALLOC(userID);
	SOAP_PTR_ALLOC(keepSpace);
	SOAP_PTR_ALLOC(requestToken);

	//copying strngs
	//	TUSerID
	STRING_TO_SOAP_EMB(userID);	
	STRING_TO_SOAP_EMB(requestToken);	
	//	ArrayOfTSURLInfo	
	CONV2SOAP_OBJ(ArrayOfTSURL,siteURLs);
	NUM_TO_SOAP_PTR(keepSpace);	
	return res;	
}

SRM2_GFAL_FREE_TYPE_DEC(srmReleaseFilesRequest)
{
	if(!_arg)return;
	FREE_SUB_OBJ(userID);	
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURL,_arg->siteURLs);	
	END_GFAL_FREE;
}

getSUBSURL_DEF(srmReleaseFilesRequest,ArrayOfTSURL,siteURLs);

/*srm2:srmReleaseFilesResponse: 
struct srm2__srmReleaseFilesResponse
{
	struct srm2__TReturnStatus *returnStatus;
	struct srm2__ArrayOfTSURLReturnStatus *arrayOfFileStatuses;
};
*/
DEF_GFALCONV_HEADERS_OUT(srmReleaseFilesResponse)
{ 
	GFAL_DECL_ALLOC(srmReleaseFilesResponse);
	//copying
	CONV2GFAL_OBJ(TReturnStatus,returnStatus);
	CONV2GFAL_OBJ(ArrayOfTSURLReturnStatus,arrayOfFileStatuses);
	return res;		
}

SRM2_GFAL_FREE_TYPE_DEC(srmReleaseFilesResponse)
{
	if(!_arg)return;
	freeType_TReturnStatus(_arg->returnStatus);
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURLReturnStatus,_arg->arrayOfFileStatuses);	
	END_GFAL_FREE;	
}

