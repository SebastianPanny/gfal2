#include "srm2_soap_conversion.h"
#include "CuTest.h"
#include "gfal_api.h"
#include "srm2Stub.h"


/*tests conducted with memory checker usually*/
#if _DUMA
#include "duma.h"
#endif

#define COPY_CHK(_name)	\
	tdata1->_name = _name;


/* Testing stuff for conversion methods.
 * Tests are build ground-up - first bidirectional simple types tests, then 
 * complex types.
 * GFAL types to smr2 and back conversions.
 */

#define INIT_SOAP	\
	struct soap soap; \
	soap_init (&soap);

#define SET_SOAP \
	struct soap soap; \
	memcpy(&soap,_soap,sizeof(soap));

#define END_SOAP		\
	soap_end(&soap);	\
	soap_done(&soap);	
	
#define NEW_DATA(_p) _p = malloc(sizeof(*_p));

#define SOAP_MAIN_ALLOC	\
	res = soap_malloc(&soap,sizeof(*res));	\
	CuAssertPtrNotNull(tc,res);	\
	memset(res, 0, sizeof(*res));

#define SOAP_PTR_ALLOC(_name)	\
	res->_name = soap_malloc(&soap,sizeof(*res->_name));	\
	CuAssertPtrNotNull(tc,res->_name);
	
#define SOAP_PTR_ARRAY_ALLOC(_name,_size)	\
	res->_name = soap_malloc(&soap,sizeof(*res->_name) * _elem->_size);	\
	CuAssertPtrNotNull(tc,res->_name);


#define STRING_TO_SOAP(src, dest) \
	 { \
		dest = soap_strdup(&soap, src); \
			CuAssertTrue(tc,!((src && !dest))); \
	 } 
#define STRING_TO_SOAP_EMB(_name) \
	 { \
		res->_name->value = soap_strdup(&soap, _name); \
			CuAssertTrue(tc,!(_name && !res->_name->value)); \
	 } 

#define NUM_TO_SOAP_EMB(f) \
	res->f->value = f

#define NUM_TO_SOAP_PTR(_n) 	\
	*(res->_n) = _n;




/* primitives for input preparation */
#define DUMMY_LFN "lfn:/test0/test1/test2/test3/test4/test5/testing.grid"
#define DUMMY_LFN1 "lfn:/test10/test11/test12/test13/test14/test15/testing.grid1"
#define DUMMY_LFN2 "lfn:/test20/test21/test22/test23/test24/test25/testing.grid2"
#define DUMMY_LFN3 "lfn:/test30/test31/test32/test33/test34/test35/testing.grid3"
#define DUMMY_LFN4 "lfn:/test40/test41/test42/test43/test44/test45/testing.grid4"
#define DUMMY_LFN5 "lfn:/test50/test51/test52/test53/test54/test55/testing.grid5"

#define DUMMY_LFNS {DUMMY_LFN1,DUMMY_LFN2,DUMMY_LFN3,DUMMY_LFN4,DUMMY_LFN5};
const TSURL dummy_lfns[] = DUMMY_LFNS;


const char STORAGE_INFO_DUMMY[]= "some storage system info";
const char SPACE_TOKEN_DUMMY[] = "Some space token equal 100";
const char VERBOSE_DESCRIPTION[] = "Some very verbose description";

#define NSIZE	5



 
TSURL* dummy_lfns_gen(int n, CuTest *tc) 
{
	TSURL* strs;
	int i;
	
	strs = malloc(sizeof(char*)*n);
	CuAssertPtrNotNull(tc,strs);
	for(i = 0; i<n;i++)
		{
		 strs[i] = strdup(dummy_lfns[i%sizeof(dummy_lfns)]);
		 CuAssertPtrNotNull(tc,strs[i]);
		}
	return strs;			
}; 
 
void TestTSURL(CuTest *tc) 
{
    INIT_SOAP;
    
    TSURL tdata1;
    struct srm2__TSURL* tdata2;
    
    tdata1 = strdup(DUMMY_LFN);

    //create srm2 from regular    
    tdata2 = conv2soap_TSURL(&soap,tdata1);
    free((char*)tdata1);	
	tdata1=NULL;
    
    CuAssertStrEquals(tc,tdata2->value,DUMMY_LFN);    
    //and back
	//check if equal
	
    tdata1 = conv2gfal_TSURL(tdata2);
    CuAssertStrEquals(tc, tdata1,DUMMY_LFN);    
    free((char*)tdata1);
    END_SOAP;
}
 
void TestArrayOfTSURL(CuTest *tc) 
{
    INIT_SOAP;
    
    int i;
    ArrayOfTSURL* tdata1;
    struct srm2__ArrayOfTSURL* tdata2;
    TSURL strings[] = DUMMY_LFNS;
    
    tdata1 = malloc(sizeof(*tdata1));
    CuAssertPtrNotNull(tc,tdata1);
    
    tdata1->__sizesurlArray = 5;
    tdata1->surlArray = dummy_lfns_gen(5,tc);
 
    //create srm2 from regular    
    tdata2 = conv2soap_ArrayOfTSURL(&soap,tdata1);
    SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURL,tdata1);
        
    for(i= 0;i<5;i++)
    CuAssertStrEquals(tc,strings[i],tdata2->surlArray[i]->value);    
    //and back
	//check if equal
	
    tdata1  = (ArrayOfTSURL*)conv2gfal_ArrayOfTSURL(tdata2);   
    
    for(i= 0;i<5;i++)
    CuAssertStrEquals(tc,strings[i],tdata1->surlArray[i]);        
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURL,tdata1);   

    END_SOAP;
}
 
//TSURLInfo 
/*typedef struct _TSURLInfo
{
	TSURL SURLOrStFN;
	TStorageSystemInfo storageSystemInfo;
} TSURLInfo;

struct srm2__TSURLInfo
{
	struct srm2__TSURL *SURLOrStFN;
	struct srm2__TStorageSystemInfo *storageSystemInfo;
};

*/
#ifdef INIT_CTX_PTR
#undef INIT_CTX_PTR
#define INIT_CTX_PTR	\
	gfal_ctx ctx;		\
    ctx.chkPtr = (void*)tc;   
#endif
void TestTSURLInfo(CuTest *tc) 
{
    INIT_SOAP;
    int i;
    TSURLInfo* tdata1;
    struct srm2__TSURLInfo* tdata2;
    INIT_CTX_PTR;
    
    //init gfal data
	tdata1 = gen_TSURLInfo(&ctx,DUMMY_LFN,STORAGE_INFO_DUMMY);
    //2soap
    tdata2 = conv2soap_TSURLInfo(&soap,tdata1);
	freeType_TSURLInfo(tdata1);	        //test    
    CuAssertStrEquals(tc,DUMMY_LFN,tdata2->SURLOrStFN->value);
    CuAssertStrEquals(tc,STORAGE_INFO_DUMMY,tdata2->storageSystemInfo->value);	
	//no 2gfal version   
 	END_SOAP;
}

/* ArrayOfTSURLInfo: 
typedef struct _ArrayOfTSURLInfo
{
	int __sizesurlInfoArray;	 sequence of elements <surlInfoArray> 
	TSURLInfo **surlInfoArray;
} ArrayOfTSURLInfo;


struct srm2__ArrayOfTSURLInfo
{
	int __sizesurlInfoArray;	
	struct srm2__TSURLInfo **surlInfoArray;
};

*/

ArrayOfTSURLInfo* gentest_ArrayOfTSURLInfo(CuTest* tc)
{
    int i;
    char k[3];
    char str1[100];
    char str2[100];
    
    ArrayOfTSURLInfo* tdata1;
    INIT_CTX_PTR;
    
    //gen data
    strcpy(str1,DUMMY_LFN);
    strcpy(str2,STORAGE_INFO_DUMMY);
    k[0]=k[1]=k[2]=0;
    
    tdata1 = malloc(sizeof(*tdata1));
    CuAssertPtrNotNull(tc,tdata1);
    tdata1->surlInfoArray = malloc(sizeof(TSURLInfo*)*NSIZE);
    CuAssertPtrNotNull(tc,tdata1->surlInfoArray);
  	for(i=0;i<NSIZE;i++)
		{
			k[0]=(char)i+'0';
			tdata1->surlInfoArray[i] = gen_TSURLInfo(&ctx,strcat(str1,k),strcat(str2,k));
		}
    tdata1->__sizesurlInfoArray = NSIZE;
	return tdata1;
	
}


void TestArrayOfTSURLInfo(CuTest *tc) 
{
    INIT_SOAP;
    int i;
    char k[2];
    char str1[100];
    char str2[100];
    
    ArrayOfTSURLInfo* tdata1;
    struct srm2__ArrayOfTSURLInfo* tdata2;
 
   
    tdata1 = gentest_ArrayOfTSURLInfo(tc);

    //create srm2 from regular    
    tdata2 = conv2soap_ArrayOfTSURLInfo(&soap,tdata1);
    
        
    for(i=0;i<NSIZE;i++)
    {
    CuAssertStrEquals(tc,(char*)tdata1->surlInfoArray[i]->SURLOrStFN,(char*)tdata2->surlInfoArray[i]->SURLOrStFN->value);
    CuAssertStrEquals(tc,(char*)tdata1->surlInfoArray[i]->storageSystemInfo,(char*)tdata2->surlInfoArray[i]->storageSystemInfo->value);
    }
    
    //and back
	//check if equal
	
//    tdata1  = (ArrayOfTSURL*)conv2gfal_ArrayOfTSURLInfo(tdata2);   
//    
//    for(i= 0;i<5;i++)
//    CuAssertStrEquals(tc,strings[i],tdata1->surlArray[i]);        
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTSURLInfo,tdata1);   

 
 	END_SOAP;
}


void TestTDirOption(CuTest* tc)
{
	INIT_SOAP;

    TDirOption* tdata1;
    struct srm2__TDirOption* tdata2;
    INIT_CTX_PTR;
    
    //init gfal data
	tdata1 = gen_TDirOption(&ctx,0,1,999);
    //2soap
    tdata2 = conv2soap_TDirOption(&soap,tdata1);
	freeType_TDirOption(tdata1);	        //test    

	CuAssertIntEquals(tc,0,*(tdata2->allLevelRecursive));
	CuAssertIntEquals(tc,1,tdata2->isSourceADirectory);
	CuAssertIntEquals(tc,999,tdata2->numOfLevels);
	//no 2gfal version   
	
	END_SOAP;
}	
/*

typedef struct _TGetFileRequest
{
	TDirOption* dirOption;
	TFileStorageType fileStorageType;
	TSURLInfo* fromSURLInfo;
	TLifeTimeInSeconds lifetime;
	TSpaceToken spaceToken;
} TGetFileRequest; 
DEF_GFALCONV_HEADERS_IN(TGetFileRequest);

 srm2:TGetFileRequest: 
struct srm2__TGetFileRequest
{
	struct srm2__TDirOption *dirOption;
	enum srm2__TFileStorageType fileStorageType;
	struct srm2__TSURLInfo *fromSURLInfo;
	struct srm2__TLifeTimeInSeconds *lifetime;
	struct srm2__TSpaceToken *spaceToken;
};

 srm2:ArrayOfTGetFileRequest: 
struct srm2__ArrayOfTGetFileRequest
{
	int __sizegetRequestArray;	 sequence of elements <getRequestArray> 
	struct srm2__TGetFileRequest **getRequestArray;
};
*/



ArrayOfTGetFileRequest* gentest_ArrayOfTGetFileRequest(CuTest* tc)
{
	
    ArrayOfTGetFileRequest* tdata1;
    char k[2];
    char str1[100];
    char str2[100];    
    int i;
    
    INIT_CTX_PTR;
    
    //create testdata 
    strcpy(str1,DUMMY_LFN);
    strcpy(str2,STORAGE_INFO_DUMMY);
    k[0]=k[1]=0;
    
    NEW_DATA(tdata1);
    CuAssertPtrNotNull(tc,tdata1);
    tdata1->getRequestArray = malloc(sizeof(TGetFileRequest*)*NSIZE);
    CuAssertPtrNotNull(tc,tdata1->getRequestArray);
  	for(i=0;i<NSIZE;i++)
		{
		k[0]=(char)i+'0';
		tdata1->getRequestArray[i] = gen_TGetFileRequest(&ctx,
			gen_TDirOption(&ctx,0,1,i),
			gfal_Volatile_,//TFileStorageType fileStorageType,
			gen_TSURLInfo(&ctx,strcat(str1,k),strcat(str2,k)),//TSURLInfo* fromSURLInfo,
			36000,//TLifeTimeInSeconds lifetime,
			strdup(SPACE_TOKEN_DUMMY)//TSpaceToken spaceToken
		);
		}
    tdata1->__sizegetRequestArray = NSIZE;
    return tdata1;
}


void TestTGetFileRequest(CuTest *tc) 
{
    INIT_SOAP;
    int i;
    TGetFileRequest* tdata1;
    struct srm2__TGetFileRequest* tdata2;
    INIT_CTX_PTR;
    
	tdata1 = gen_TGetFileRequest(&ctx,
		gen_TDirOption(&ctx,0,1,999),
		gfal_Volatile_,//TFileStorageType fileStorageType,
		gen_TSURLInfo(&ctx,DUMMY_LFN,STORAGE_INFO_DUMMY),//TSURLInfo* fromSURLInfo,
		36000,//TLifeTimeInSeconds lifetime,
		strdup(SPACE_TOKEN_DUMMY)//TSpaceToken spaceToken
		);
		
	tdata2 = conv2soap_TGetFileRequest(&soap,tdata1);
	//check if values equal

	CuAssertIntEquals(tc,tdata1->dirOption->allLevelRecursive,*(tdata2->dirOption->allLevelRecursive));
	CuAssertIntEquals(tc,tdata1->dirOption->isSourceADirectory,tdata2->dirOption->isSourceADirectory);
	CuAssertIntEquals(tc,tdata1->dirOption->numOfLevels,tdata2->dirOption->numOfLevels);

	//paranoid check of bizzare gsoap 2.6 generated struct layout
	CuAssertIntEquals(tc,0,*(tdata2->dirOption->allLevelRecursive));
	CuAssertIntEquals(tc,1,tdata2->dirOption->isSourceADirectory);
	CuAssertIntEquals(tc,999,tdata2->dirOption->numOfLevels);
	
	
	CuAssertIntEquals(tc,(int)gfal_Volatile_,*(tdata2->fileStorageType));
	CuAssertStrEquals(tc,DUMMY_LFN,tdata2->fromSURLInfo->SURLOrStFN->value);
    CuAssertStrEquals(tc,STORAGE_INFO_DUMMY,tdata2->fromSURLInfo->storageSystemInfo->value);	
    
    CuAssertIntEquals(tc,36000,tdata2->lifetime->value);
    CuAssertStrEquals(tc,(char*)SPACE_TOKEN_DUMMY,(char*)tdata2->spaceToken->value);
      
	//free source
	freeType_TGetFileRequest(tdata1);	    

 	END_SOAP;
}

/*
 * ArrayOfTGetFileRequest: 
typedef struct _ArrayOfTGetFileRequest
{
	int __sizegetRequestArray;	
	TGetFileRequest **getRequestArray;
} ArrayOfTGetFileRequest;
DEF_GFALCONV_HEADERS_IN(ArrayOfTGetFileRequest);

*/
void TestArrayOfTGetFileRequest(CuTest *tc) 
{
    INIT_SOAP;
    ArrayOfTGetFileRequest* tdata1;
    struct srm2__ArrayOfTGetFileRequest* tdata2;
    char k[2];
    char str1[100];
    char str2[100];    
    int i;

	tdata1 = gentest_ArrayOfTGetFileRequest(tc);
    //create srm2 from regular    
    tdata2 = conv2soap_ArrayOfTGetFileRequest(&soap,tdata1);  

	//test reset
	strcpy(str1,DUMMY_LFN);
    strcpy(str2,STORAGE_INFO_DUMMY);
    k[0]=k[1]=0;
	        
    for(i=0;i<NSIZE;i++)
    {
    	k[0]=(char)i+'0';
	    CuAssertStrEquals(tc,strcat(str1,k),(char*)tdata2->getRequestArray[i]->fromSURLInfo->SURLOrStFN->value);
	    CuAssertStrEquals(tc,strcat(str2,k),(char*)tdata2->getRequestArray[i]->fromSURLInfo->storageSystemInfo->value);
	    CuAssertIntEquals(tc,1,tdata2->getRequestArray[i]->dirOption->isSourceADirectory);
	    CuAssertIntEquals(tc,i,tdata2->getRequestArray[i]->dirOption->numOfLevels);    
    }    
    
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTGetFileRequest,tdata1);       
 
 	END_SOAP;
}

/*typedef struct _TPutFileRequest
{
	TFileStorageType fileStorageType;
	TSizeInBytes knownSizeOfThisFile;
	TLifeTimeInSeconds lifetime;
	TSpaceToken spaceToken;
	TSURLInfo* toSURLInfo;
} TPutFileRequest;

};*/



void TestTPutFileRequest(CuTest *tc) 
{
	INIT_SOAP;
    int i;
    TPutFileRequest* tdata1;
    struct srm2__TPutFileRequest* tdata2;
    INIT_CTX_PTR;
    
    tdata1 = gen_TPutFileRequest(&ctx,
		gfal_Volatile_,//TFileStorageType fileStorageType,
		100000001,//TSizeInBytes knownSizeOfThisFile,
		31536000,//TLifeTimeInSeconds lifetime,
		strdup(SPACE_TOKEN_DUMMY),//TSpaceToken spaceToken				
		gen_TSURLInfo(&ctx,DUMMY_LFN,STORAGE_INFO_DUMMY)//TSURLInfo* fromSURLInfo,
		);
		
	tdata2 = conv2soap_TPutFileRequest(&soap,tdata1);
	//check if values equal

	CuAssertIntEquals(tc,(int)gfal_Volatile_,*(tdata2->fileStorageType));
	CuAssertIntEquals(tc,100000001,tdata2->knownSizeOfThisFile->value);
	
	CuAssertStrEquals(tc,DUMMY_LFN,tdata2->toSURLInfo->SURLOrStFN->value);
    CuAssertStrEquals(tc,STORAGE_INFO_DUMMY,tdata2->toSURLInfo->storageSystemInfo->value);	
    
    CuAssertIntEquals(tc,31536000,tdata2->lifetime->value);
    CuAssertStrEquals(tc,(char*)SPACE_TOKEN_DUMMY,(char*)tdata2->spaceToken->value);
      
	//free source
	freeType_TPutFileRequest(tdata1);	    

 	END_SOAP;
 
}

ArrayOfTPutFileRequest* gentest_ArrayOfTPutFileRequest(CuTest* tc)
{
	
	ArrayOfTPutFileRequest* tdata1;
    char k[2];
    char str1[100];
    char str2[100];    
    int i;
	INIT_CTX_PTR;    
    //create testdata 
    strcpy(str1,DUMMY_LFN);
    strcpy(str2,STORAGE_INFO_DUMMY);
    k[0]=k[1]=0;
    
    tdata1 = malloc(sizeof(*tdata1));
    CuAssertPtrNotNull(tc,tdata1);
    tdata1->putRequestArray = malloc(sizeof(TPutFileRequest)*NSIZE);
    CuAssertPtrNotNull(tc,tdata1->putRequestArray);
  	for(i=0;i<NSIZE;i++)
		{
		k[0]=(char)i+'0';			
		tdata1->putRequestArray[i] = gen_TPutFileRequest(&ctx,
		gfal_Volatile_,//TFileStorageType fileStorageType,
		100000001,//TSizeInBytes knownSizeOfThisFile,
		31536000,//TLifeTimeInSeconds lifetime,
		strdup(SPACE_TOKEN_DUMMY),//TSpaceToken spaceToken				
		gen_TSURLInfo(&ctx,strcat(str1,k),strcat(str2,k))//TSURLInfo* fromSURLInfo,
		);
		
		}
    tdata1->__sizeputRequestArray = NSIZE;
    return tdata1;
}


void TestArrayOfTPutFileRequest(CuTest *tc) 
{
    INIT_SOAP;
    ArrayOfTPutFileRequest* tdata1;
    struct srm2__ArrayOfTPutFileRequest* tdata2;
    char k[2];
    char str1[100];
    char str2[100];    
    int i;
    
    //create testdata 
    tdata1 = gentest_ArrayOfTPutFileRequest(tc);
    //create srm2 from regular    
    tdata2 = conv2soap_ArrayOfTPutFileRequest(&soap,tdata1);  

	//test reset
	strcpy(str1,DUMMY_LFN);
    strcpy(str2,STORAGE_INFO_DUMMY);
    k[0]=k[1]=0;
	        
    for(i=0;i<NSIZE;i++)
    {
    	k[0]=(char)i+'0';
	    CuAssertStrEquals(tc,strcat(str1,k),(char*)tdata2->putRequestArray[i]->toSURLInfo->SURLOrStFN->value);
	    CuAssertStrEquals(tc,strcat(str2,k),(char*)tdata2->putRequestArray[i]->toSURLInfo->storageSystemInfo->value);
	    
	    CuAssertIntEquals(tc,(int)gfal_Volatile_,*(tdata2->putRequestArray[i]->fileStorageType));
		CuAssertIntEquals(tc,100000001,tdata2->putRequestArray[i]->knownSizeOfThisFile->value);
	    CuAssertIntEquals(tc,31536000,tdata2->putRequestArray[i]->lifetime->value);
	    CuAssertStrEquals(tc,(char*)SPACE_TOKEN_DUMMY,(char*)tdata2->putRequestArray[i]->spaceToken->value);	    
    }    
    
	SRM2_CALL_GFAL_FREEARRAY_TYPE(ArrayOfTPutFileRequest,tdata1);        
 	END_SOAP;
}

/*
 * typedef struct _srmPrepareToPutRequest
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
DEF_GFALCONV_HEADERS_IN(srmPrepareToPutRequest);
 * 
 * struct srm2__srmPrepareToPutRequest
{
	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTPutFileRequest *arrayOfFileRequests;
	struct srm2__ArrayOf_USCORExsd_USCOREstring *arrayOfTransferProtocols;
	char *userRequestDescription;
	enum srm2__TOverwriteMode *overwriteOption;
	struct srm2__TStorageSystemInfo *storageSystemInfo;
	struct srm2__TLifeTimeInSeconds *totalRetryTime;
};
*/



const char DUMMY_USERID[] =  "SOME_VO_USERIDXYZ";


void TestsrmPrepareToPutRequest(CuTest *tc) 
{
    INIT_SOAP;
    srmPrepareToPutRequest* tdata1;
    struct srm2__srmPrepareToPutRequest* tdata2;
	INIT_CTX_PTR;    
    char k[2];
    char str1[100];
    char str2[100];    
    int i;
    
    char** protocols  = (char**)malloc(3*sizeof(char*));
    protocols[0]=strdup("srm");    
    protocols[1]=strdup("lfn");    
    protocols[2]=strdup("rfio");   
    
    
    tdata1 = gen_srmPrepareToPutRequest(&ctx,
    	strdup(DUMMY_USERID),
    	gentest_ArrayOfTPutFileRequest(tc),
    	3,
    	protocols,
    	strdup("some verbose description"),
    	gfal_WhenFilesAreDifferent,
    	strdup(STORAGE_INFO_DUMMY),
    	999);
    	
    tdata2  = conv2soap_srmPrepareToPutRequest(&soap,tdata1);
    
    //check
    	//test reset
	strcpy(str1,DUMMY_LFN);
    strcpy(str2,STORAGE_INFO_DUMMY);
    k[0]=k[1]=0;
	        
    for(i=0;i<NSIZE;i++)
    {
    	k[0]=(char)i+'0';
    	//array checks
	    CuAssertStrEquals(tc,strcat(str1,k),(char*)tdata2->arrayOfFileRequests->putRequestArray[i]->toSURLInfo->SURLOrStFN->value);
	    CuAssertStrEquals(tc,strcat(str2,k),(char*)tdata2->arrayOfFileRequests->putRequestArray[i]->toSURLInfo->storageSystemInfo->value);
	    
	    CuAssertIntEquals(tc,(int)gfal_Volatile_,*(tdata2->arrayOfFileRequests->putRequestArray[i]->fileStorageType));
		CuAssertIntEquals(tc,100000001,tdata2->arrayOfFileRequests->putRequestArray[i]->knownSizeOfThisFile->value);
	    CuAssertIntEquals(tc,31536000,tdata2->arrayOfFileRequests->putRequestArray[i]->lifetime->value);
	    CuAssertStrEquals(tc,(char*)SPACE_TOKEN_DUMMY,(char*)tdata2->arrayOfFileRequests->putRequestArray[i]->spaceToken->value);	    
    }    
    //more checks
    #warning do more checks

	freeType_srmPrepareToPutRequest(tdata1);	     

 	END_SOAP;
} 

/* *********************** out parameters tests **************************** */


/*typedef struct _TReturnStatus
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

struct srm2__TReturnStatus* gentest_srm2__TReturnStatus(CuTest* tc,
	struct soap* _soap,
	char* explanation,
	TStatusCode statusCode)
{
	struct srm2__TReturnStatus* res;
	SET_SOAP;
    SOAP_MAIN_ALLOC;
	
	//generate TReturnStatus
	STRING_TO_SOAP(explanation,res->explanation);
	res->statusCode = *convEnum2soap_TStatusCode(&soap,statusCode);
    return res;
}



static const char DUMMY_EXPLANATION[]="Some dummy explanation...";
void TestTReturnStatus(CuTest *tc) 
{
    INIT_SOAP;
    struct srm2__TReturnStatus* tdata1;
    TReturnStatus* tdata2;
    

	/*typedef enum TStatusCode_ {SRM_SUCCESS = 0, SRM_FAILURE = 1, SRM_AUTHENTICATION_FAILURE = 2, SRM_UNAUTHORIZED_ACCESS = 3, SRM_INVALID_REQUEST = 4, SRM_INVALID_PATH = 5, SRM_FILE_LIFETIME_EXPIRED = 6, SRM_SPACE_LIFETIME_EXPIRED = 7, SRM_EXCEED_ALLOCATION = 8, SRM_NO_USER_SPACE = 9, SRM_NO_FREE_SPACE = 10, SRM_DUPLICATION_ERROR = 11, SRM_NON_EMPTY_DIRECTORY = 12, SRM_TOO_MANY_RESULTS = 13, SRM_INTERNAL_ERROR = 14, SRM_FATAL_INTERNAL_ERROR = 15, SRM_NOT_SUPPORTED = 16, SRM_REQUEST_QUEUED = 17, SRM_REQUEST_INPROGRESS = 18, SRM_REQUEST_SUSPENDED = 19, SRM_ABORTED = 20, SRM_RELEASED = 21, SRM_FILE_PINNED = 22, SRM_FILE_IN_CACHE = 23, SRM_SPACE_AVAILABLE = 24, SRM_LOWER_SPACE_GRANTED = 25, SRM_DONE = 26, SRM_CUSTOM_STATUS = 27}
		TStatusCode;
	*/    
    tdata1 = gentest_srm2__TReturnStatus(tc,&soap,    
    	(char*)DUMMY_EXPLANATION,
    	SRM_AUTHENTICATION_FAILURE);
    	
    tdata2  = conv2gfal_TReturnStatus(tdata1);
    
    //check
    CuAssertStrEquals(tc,DUMMY_EXPLANATION,tdata2->explanation);
    CuAssertIntEquals(tc,(int)SRM_AUTHENTICATION_FAILURE,tdata2->statusCode);

	freeType_TReturnStatus(tdata2);   
 
 	END_SOAP;
}


/*typedef struct _TPutRequestFileStatus
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


/*	NUM_TO_SOAP(estimatedProcessingTime);
	NUM_TO_SOAP(estimatedWaitTimeOnQueue);
	NUM_TO_SOAP(fileSize);
	NUM_TO_SOAP(remainingPinTime);
	STRING_TO_SOAP_EMB(siteURL);
	res->status = conv2soap_TReturnStatus(_elem->status);
	STRING_TO_SOAP_EMB(transferURL);
	
		
	struct srm2__TPutRequestFileStatus
{
	struct srm2__TLifeTimeInSeconds *estimatedProcessingTime;
	struct srm2__TLifeTimeInSeconds *estimatedWaitTimeOnQueue;
	struct srm2__TSizeInBytes *fileSize;
	struct srm2__TLifeTimeInSeconds *remainingPinTime;
	struct srm2__TSURL *siteURL;
	struct srm2__TReturnStatus *status;
	struct srm2__TTURL *transferURL;
};
	
*/
struct srm2__TPutRequestFileStatus* gentest_TPutRequestFileStatus(CuTest* tc,struct soap* _soap,
	TLifeTimeInSeconds estimatedProcessingTime,
	TLifeTimeInSeconds estimatedWaitTimeOnQueue,
	TSizeInBytes fileSize,
	TLifeTimeInSeconds remainingPinTime,
	TSURL siteURL,
	struct srm2__TReturnStatus* status,
	TTURL transferURL)
{
	
	struct srm2__TPutRequestFileStatus* res;
	SET_SOAP;
    SOAP_MAIN_ALLOC;
	
	SOAP_PTR_ALLOC(estimatedProcessingTime);
	SOAP_PTR_ALLOC(estimatedWaitTimeOnQueue);
	SOAP_PTR_ALLOC(fileSize);
	SOAP_PTR_ALLOC(remainingPinTime);

    SOAP_PTR_ALLOC(siteURL);
    SOAP_PTR_ALLOC(transferURL);
    
	//copying 
	NUM_TO_SOAP_EMB(estimatedProcessingTime);
	NUM_TO_SOAP_EMB(estimatedWaitTimeOnQueue);
	NUM_TO_SOAP_EMB(fileSize);
	NUM_TO_SOAP_EMB(remainingPinTime);
	STRING_TO_SOAP(siteURL,res->siteURL->value);
	STRING_TO_SOAP(transferURL,res->transferURL->value);
	res->status =  status;
	
    return res;	
	
}	
	
static const char SOME_SITE_URL[]="http://innerspace.org";
static const char SOME_DUMMY_TURL[]="on/1/2/3/4/5/6/end";

static const long long	DUMMY_ESTIMATEDPROCESSINGTIME = 222222222;
static const long long	DUMMY_ESTIMATEDWAITTIMEONQUEUE=333333333;
static const long long	DUMMY_FILESIZE = 1234567890;
static const long long	DUMMY_REMAININGPINTIME = 10000;


void TestTPutRequestFileStatus(CuTest *tc) 
{
    INIT_SOAP;
    struct srm2__TPutRequestFileStatus* tdata1;
    TPutRequestFileStatus* tdata2;
    

	/*typedef enum TStatusCode_ {SRM_SUCCESS = 0, SRM_FAILURE = 1, SRM_AUTHENTICATION_FAILURE = 2, SRM_UNAUTHORIZED_ACCESS = 3, SRM_INVALID_REQUEST = 4, SRM_INVALID_PATH = 5, SRM_FILE_LIFETIME_EXPIRED = 6, SRM_SPACE_LIFETIME_EXPIRED = 7, SRM_EXCEED_ALLOCATION = 8, SRM_NO_USER_SPACE = 9, SRM_NO_FREE_SPACE = 10, SRM_DUPLICATION_ERROR = 11, SRM_NON_EMPTY_DIRECTORY = 12, SRM_TOO_MANY_RESULTS = 13, SRM_INTERNAL_ERROR = 14, SRM_FATAL_INTERNAL_ERROR = 15, SRM_NOT_SUPPORTED = 16, SRM_REQUEST_QUEUED = 17, SRM_REQUEST_INPROGRESS = 18, SRM_REQUEST_SUSPENDED = 19, SRM_ABORTED = 20, SRM_RELEASED = 21, SRM_FILE_PINNED = 22, SRM_FILE_IN_CACHE = 23, SRM_SPACE_AVAILABLE = 24, SRM_LOWER_SPACE_GRANTED = 25, SRM_DONE = 26, SRM_CUSTOM_STATUS = 27}
		TStatusCode;
	*/    
    tdata1 = gentest_TPutRequestFileStatus(tc,&soap,
	DUMMY_ESTIMATEDPROCESSINGTIME,//estimatedProcessingTime,
	DUMMY_ESTIMATEDWAITTIMEONQUEUE,// estimatedWaitTimeOnQueue,
	DUMMY_FILESIZE,			// fileSize,
	DUMMY_REMAININGPINTIME,	//remainingPinTime,
	SOME_SITE_URL,			//TSURL siteURL,
	gentest_srm2__TReturnStatus(tc,&soap,(char*)DUMMY_EXPLANATION,SRM_AUTHENTICATION_FAILURE), //status
	SOME_DUMMY_TURL			//transferURL
	);

    tdata2  = conv2gfal_TPutRequestFileStatus(tdata1);
    
    //check    
    CuAssertStrEquals(tc,(char*)SOME_SITE_URL,tdata2->siteURL);
    CuAssertStrEquals(tc,(char*)SOME_DUMMY_TURL,tdata2->transferURL);
    
    CuAssertIntEquals(tc,DUMMY_ESTIMATEDPROCESSINGTIME,tdata2->estimatedProcessingTime);
    CuAssertIntEquals(tc,DUMMY_ESTIMATEDWAITTIMEONQUEUE,tdata2->estimatedWaitTimeOnQueue);
    CuAssertIntEquals(tc,DUMMY_FILESIZE,tdata2->fileSize);
    CuAssertIntEquals(tc,DUMMY_REMAININGPINTIME,tdata2->remainingPinTime);
        
    CuAssertStrEquals(tc,DUMMY_EXPLANATION,tdata2->status->explanation);
    CuAssertIntEquals(tc,(int)SRM_AUTHENTICATION_FAILURE,tdata2->status->statusCode);

	freeType_TPutRequestFileStatus(tdata2);   
 	END_SOAP;
}


/*struct srm2__ArrayOfTPutRequestFileStatus
{
	int __sizeputStatusArray;	 sequence of elements <putStatusArray> 
	struct srm2__TPutRequestFileStatus **putStatusArray;
};
*/

struct srm2__ArrayOfTPutRequestFileStatus* gentest_srm2__ArrayOfTPutRequestFileStatus(CuTest* tc,struct soap* _soap)
{
	SET_SOAP;
	struct srm2__ArrayOfTPutRequestFileStatus* tdata1;
    char k[2];
    char str1[100];
    char str2[100];    
    char str3[100];
    int i;
    
    //create testdata 
    strcpy(str1,SOME_SITE_URL);
    strcpy(str2,SOME_DUMMY_TURL);
	strcpy(str3,DUMMY_EXPLANATION);
    k[0]=k[1]=0;
    
    tdata1 = soap_malloc(&soap,sizeof(*tdata1));
    CuAssertPtrNotNull(tc,tdata1);
    tdata1->putStatusArray = soap_malloc(&soap,sizeof(TPutRequestFileStatus)*NSIZE);
    CuAssertPtrNotNull(tc,tdata1->putStatusArray);
  	for(i=0;i<NSIZE;i++)
		{
		k[0]=(char)i+'0';
		tdata1->putStatusArray[i] = gentest_TPutRequestFileStatus(tc,&soap,
			DUMMY_ESTIMATEDPROCESSINGTIME+i,//estimatedProcessingTime,
			DUMMY_ESTIMATEDWAITTIMEONQUEUE+i,// estimatedWaitTimeOnQueue,
			DUMMY_FILESIZE+i,			// fileSize,
			DUMMY_REMAININGPINTIME+i,	//remainingPinTime,
			strcat(str1,k),			//TSURL siteURL,
			gentest_srm2__TReturnStatus(tc,&soap,strcat(str3,k),SRM_AUTHENTICATION_FAILURE), //status
			strcat(str2,k)			//transferURL
			);
		
		}
    tdata1->__sizeputStatusArray = NSIZE;
    return tdata1;
	
}
	


void TestArrayOfTPutRequestFileStatus(CuTest *tc) 
{
    INIT_SOAP;
    struct srm2__ArrayOfTPutRequestFileStatus* tdata1;
    ArrayOfTPutRequestFileStatus* tdata2;
    int i;
    char k[2];
    char str1[100];
    char str2[100];    
    char str3[100];    
       
    
    //create testdata 
    tdata1 = gentest_srm2__ArrayOfTPutRequestFileStatus(tc,&soap);
    //create srm2 from regular    
    tdata2 = conv2gfal_ArrayOfTPutRequestFileStatus(tdata1);  

	//test reset
	strcpy(str1,SOME_SITE_URL);
    strcpy(str2,SOME_DUMMY_TURL);
	strcpy(str3,DUMMY_EXPLANATION);
	
    k[0]=k[1]=0;
	 

	CuAssertIntEquals(tc,NSIZE,tdata2->__sizeputStatusArray);
    for(i=0;i<NSIZE;i++)
    {
    	k[0]=(char)i+'0';
	    
 		CuAssertIntEquals(tc,DUMMY_ESTIMATEDPROCESSINGTIME+i,tdata2->putStatusArray[i]->estimatedProcessingTime);
		CuAssertIntEquals(tc,DUMMY_ESTIMATEDWAITTIMEONQUEUE+i,tdata2->putStatusArray[i]->estimatedWaitTimeOnQueue);
		CuAssertIntEquals(tc,DUMMY_FILESIZE+i,tdata2->putStatusArray[i]->fileSize);
		CuAssertIntEquals(tc,DUMMY_REMAININGPINTIME+i,tdata2->putStatusArray[i]->remainingPinTime);

		CuAssertStrEquals(tc,strcat(str1,k),tdata2->putStatusArray[i]->siteURL);		
		CuAssertStrEquals(tc,strcat(str3,k),tdata2->putStatusArray[i]->status->explanation);
	    CuAssertIntEquals(tc,(int)SRM_AUTHENTICATION_FAILURE,tdata2->putStatusArray[i]->status->statusCode);
		
		CuAssertStrEquals(tc,strcat(str2,k),tdata2->putStatusArray[i]->transferURL);
    }    
   
	freegfalArray_ArrayOfTPutRequestFileStatus(tdata2);            

 
 	END_SOAP;
}

/*
 * struct srm2__srmPrepareToPutResponse
{
	struct srm2__TRequestToken *requestToken;
	struct srm2__TReturnStatus *returnStatus;
	struct srm2__ArrayOfTPutRequestFileStatus *arrayOfFileStatuses;
};
 * 
 * 
 * typedef struct _srmPrepareToPutResponse
{
	TRequestToken requestToken;
	TReturnStatus* returnStatus;
	ArrayOfTPutRequestFileStatus* arrayOfFileStatuses;
} srmPrepareToPutResponse;
*/

static const char SOME_DUMMY_TOKEN[] = "USER TOKEN 1112222333";
struct srm2__srmPrepareToPutResponse* gentest_srm2__srmPrepareToPutResponse(CuTest* tc,struct soap* _soap,
	char* requestToken,
	struct srm2__TReturnStatus* returnStatus,
	struct srm2__ArrayOfTPutRequestFileStatus* arrayOfFileStatuses
	)
{
	SET_SOAP;
	
	struct srm2__srmPrepareToPutResponse* res;
    SOAP_MAIN_ALLOC;
    SOAP_PTR_ALLOC(requestToken);
    	
	STRING_TO_SOAP(requestToken,res->requestToken->value);
	res->arrayOfFileStatuses = gentest_srm2__ArrayOfTPutRequestFileStatus(tc,&soap);	
	res->returnStatus = gentest_srm2__TReturnStatus(tc,&soap,returnStatus->explanation,returnStatus->statusCode);
	return res;	
	
}	
	
void TestsrmPrepareToPutResponse(CuTest *tc) 
{
    INIT_SOAP;
    
    struct srm2__srmPrepareToPutResponse* tdata1;
    srmPrepareToPutResponse* tdata2;
    int i;
    char k[2];
    char str1[100];
    char str2[100];    
    char str3[100];    
       
    
    //create testdata 
    tdata1 = gentest_srm2__srmPrepareToPutResponse(tc,&soap,
    	(char*)SOME_DUMMY_TOKEN,
    	gentest_srm2__TReturnStatus(tc,&soap,(char*)DUMMY_EXPLANATION,SRM_AUTHENTICATION_FAILURE),
    	gentest_srm2__ArrayOfTPutRequestFileStatus(tc,&soap)
    	);
   	
    
    //create srm2 from regular    
    tdata2 = conv2gfal_srmPrepareToPutResponse(tdata1);  

	//test reset
	strcpy(str1,SOME_SITE_URL);
    strcpy(str2,SOME_DUMMY_TURL);
	strcpy(str3,DUMMY_EXPLANATION);

	CuAssertStrEquals(tc,DUMMY_EXPLANATION,tdata2->returnStatus->explanation);
    CuAssertIntEquals(tc,(int)SRM_AUTHENTICATION_FAILURE,tdata2->returnStatus->statusCode);
    CuAssertStrEquals(tc,SOME_DUMMY_TOKEN,tdata2->requestToken);
	
    k[0]=k[1]=0;

	CuAssertIntEquals(tc,NSIZE,tdata2->arrayOfFileStatuses->__sizeputStatusArray);
    for(i=0;i<NSIZE;i++)
    {
    	k[0]=(char)i+'0';
	    
 		CuAssertIntEquals(tc,DUMMY_ESTIMATEDPROCESSINGTIME+i,tdata2->arrayOfFileStatuses->putStatusArray[i]->estimatedProcessingTime);
		CuAssertIntEquals(tc,DUMMY_ESTIMATEDWAITTIMEONQUEUE+i,tdata2->arrayOfFileStatuses->putStatusArray[i]->estimatedWaitTimeOnQueue);
		CuAssertIntEquals(tc,DUMMY_FILESIZE+i,tdata2->arrayOfFileStatuses->putStatusArray[i]->fileSize);
		CuAssertIntEquals(tc,DUMMY_REMAININGPINTIME+i,tdata2->arrayOfFileStatuses->putStatusArray[i]->remainingPinTime);

		CuAssertStrEquals(tc,strcat(str1,k),tdata2->arrayOfFileStatuses->putStatusArray[i]->siteURL);		
		CuAssertStrEquals(tc,strcat(str3,k),tdata2->arrayOfFileStatuses->putStatusArray[i]->status->explanation);
	    CuAssertIntEquals(tc,(int)SRM_AUTHENTICATION_FAILURE,tdata2->arrayOfFileStatuses->putStatusArray[i]->status->statusCode);
		
		CuAssertStrEquals(tc,strcat(str2,k),tdata2->arrayOfFileStatuses->putStatusArray[i]->transferURL);
    }    
  
   	freeType_srmPrepareToPutResponse(tdata2); 
 	END_SOAP;
}





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
*/

void TestPermission(CuTest* tc)
{
	
	
}
	

/*struct srm2__srmPrepareToGetRequest
{
	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTGetFileRequest *arrayOfFileRequests;
	struct srm2__ArrayOf_USCORExsd_USCOREstring *arrayOfTransferProtocols;
	char *userRequestDescription;
	struct srm2__TStorageSystemInfo *storageSystemInfo;
	struct srm2__TLifeTimeInSeconds *totalRetryTime;
};
*/
void TestsrmPrepareToGetRequest(CuTest *tc) 
{
    INIT_SOAP;
    srmPrepareToGetRequest* tdata1;
    struct srm2__srmPrepareToGetRequest* tdata2;
	INIT_CTX_PTR;
    
    int i;
    char k[2];
    char str1[100];
    char str2[100];    
    char str3[100];    
    
    char** protocols  = (char**)malloc(3*sizeof(char*));
    protocols[0]=strdup("srm");    
    protocols[1]=strdup("lfn");    
    protocols[2]=strdup("rfio");   

    tdata1 = gen_srmPrepareToGetRequest(&ctx,
    	strdup(DUMMY_USERID),
    	gentest_ArrayOfTGetFileRequest(tc),
    	3,
    	protocols,
    	strdup(VERBOSE_DESCRIPTION),
    	strdup(STORAGE_INFO_DUMMY),
    	999);
    	
    tdata2  = conv2soap_srmPrepareToGetRequest(&soap,tdata1);

    
    //check
	//test reset
	strcpy(str1,DUMMY_LFN);
    strcpy(str2,STORAGE_INFO_DUMMY);
    k[0]=k[1]=0;
/*	        struct srm2__TGetFileRequest
{
	struct srm2__TDirOption *dirOption;
	enum srm2__TFileStorageType *fileStorageType;
	struct srm2__TSURLInfo *fromSURLInfo;
	struct srm2__TLifeTimeInSeconds *lifetime;
	struct srm2__TSpaceToken *spaceToken;
};
	        */
	        
	CuAssertStrEquals(tc,DUMMY_USERID,tdata2->userID->value);
	CuAssertStrEquals(tc,VERBOSE_DESCRIPTION,tdata2->userRequestDescription);
	CuAssertStrEquals(tc, STORAGE_INFO_DUMMY,tdata2->storageSystemInfo->value);
	CuAssertIntEquals(tc,999,tdata2->totalRetryTime->value);
	
	
    for(i=0;i<NSIZE;i++)
    {
    	k[0]=(char)i+'0';
    	//array checks
	    CuAssertStrEquals(tc,strcat(str1,k),(char*)tdata2->arrayOfFileRequests->getRequestArray[i]->fromSURLInfo->SURLOrStFN->value);
	    CuAssertStrEquals(tc,strcat(str2,k),(char*)tdata2->arrayOfFileRequests->getRequestArray[i]->fromSURLInfo->storageSystemInfo->value);
	    
	    CuAssertIntEquals(tc,(int)gfal_Volatile_,*(tdata2->arrayOfFileRequests->getRequestArray[i]->fileStorageType));
	    CuAssertIntEquals(tc,36000,tdata2->arrayOfFileRequests->getRequestArray[i]->lifetime->value);
	    CuAssertStrEquals(tc,(char*)SPACE_TOKEN_DUMMY,(char*)tdata2->arrayOfFileRequests->getRequestArray[i]->spaceToken->value);	    
    }    

    

	freeType_srmPrepareToGetRequest(tdata1);	     
    
    
 
 	END_SOAP;
}

void TestsrmPrepareToGetResponse(CuTest *tc) 
{
    INIT_SOAP;
    
    
    
 
 	END_SOAP;
}

void TestsrmPutDoneRequest(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}

void TestsrmPutDoneResponse(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}


void TestsrmAbortFilesRequest(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}

void TestsrmAbortFilesResponse(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}

void TestsrmStatusOfGetRequestRequest(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}

void TestsrmStatusOfGetRequestResponse(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}

void TestsrmStatusOfPutRequestRequest(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}

void TestsrmStatusOfPutRequestResponse(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}


void TestTSURLReturnStatus(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}

void TestArrayOfTSURLReturnStatus(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}

void TestTGetRequestFileStatus(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}

void TestArrayOfTGetRequestFileStatus(CuTest *tc) 
{
    INIT_SOAP;
 
 	END_SOAP;
}



/*typedef struct _srmLsRequest
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
*/	


void TestsrmLsRequest(CuTest* tc)
{
	INIT_SOAP;
 
    srmLsRequest* tdata1;
    struct srm2__srmLsRequest* tdata2;
	INIT_CTX_PTR;	
    
    int i;
    char k[2];
    char str1[100];
    char str2[100];    
    char str3[100];    
    

    tdata1 = gen_srmLsRequest(&ctx,
    	strdup(DUMMY_USERID),
    	gentest_ArrayOfTSURLInfo(tc),
    	gfal_Durable_,
    	0,
    	1,
    	100,//offset
    	33, //numOfLevels
    	10); //count
    	
    tdata2  = conv2soap_srmLsRequest(&soap,tdata1);
    
    //test and free
    
    /*	struct srm2__TUserID *userID;
	struct srm2__ArrayOfTSURLInfo *paths;
	enum srm2__TFileStorageType *fileStorageType;
	enum xsd__boolean *fullDetailedList;
	enum xsd__boolean *allLevelRecursive;
	int *numOfLevels;
	int *offset;
	int *count;
    */
    CuAssertStrEquals(tc,DUMMY_USERID,tdata2->userID->value);;

//	CuAssertIntEquals(tc,,(paths);
    //gen data
    strcpy(str1,DUMMY_LFN);
    strcpy(str2,STORAGE_INFO_DUMMY);
    k[0]=k[1]=0;
    
  	for(i=0;i<tdata2->paths->__sizesurlInfoArray;i++)
		{
			k[0]=(char)i+'0';
			CuAssertStrEquals(tc,strcat(str1,k),tdata2->paths->surlInfoArray[i]->SURLOrStFN->value);
			CuAssertStrEquals(tc,strcat(str2,k),(char*)tdata2->paths->surlInfoArray[i]->storageSystemInfo->value);		   
		}
	
	CuAssertIntEquals(tc,gfal_Durable_,*tdata2->fileStorageType);
	CuAssertIntEquals(tc,0,*tdata2->fullDetailedList);
	CuAssertIntEquals(tc,1,*tdata2->allLevelRecursive);
	CuAssertIntEquals(tc,100,*tdata2->numOfLevels);
	CuAssertIntEquals(tc,33,*tdata2->offset);
	CuAssertIntEquals(tc,10,*tdata2->count);    
 
 	freeType_srmLsRequest(tdata1);	     
	END_SOAP;
	
}

/*struct srm2__srmLsResponse
{
	struct srm2__ArrayOfTMetaDataPathDetail *details;
	struct srm2__TReturnStatus *returnStatus;
};
*/


/* * struct srm2__TMetaDataPathDetail
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
*/


/* srm2:TOwnerPermission: */
/*struct srm2__TOwnerPermission
{
	enum srm2__TPermissionMode mode;
};
*/
	
struct srm2__TOwnerPermission* gentest_srm2__TOwnerPermission(CuTest* tc, struct soap* _soap,enum srm2__TPermissionMode mode)
{
	SET_SOAP;
	struct srm2__TOwnerPermission* res;
    SOAP_MAIN_ALLOC;
    res->mode=mode;
    return res;	
}

/*struct srm2__TUserPermission
{
    enum srm2__TPermissionMode           mode                           1:1;
    struct srm2__TUserID*                userID                         1:1;
};
*/
struct srm2__TUserPermission* gentest_srm2__TUserPermission(CuTest* tc,struct soap* _soap,enum srm2__TPermissionMode mode, TUserID userID)
{	
	SET_SOAP;
	struct srm2__TUserPermission* res;
    SOAP_MAIN_ALLOC;
    SOAP_PTR_ALLOC(userID);

    res->mode=mode;    	
	STRING_TO_SOAP(userID,res->userID->value);
	return res;
}

struct srm2__TGroupPermission* gentest_srm2__TGroupPermission(CuTest* tc,struct soap* _soap,enum srm2__TPermissionMode mode, TGroupID groupID)
{	
	SET_SOAP;
	struct srm2__TGroupPermission* res;
    SOAP_MAIN_ALLOC;
    SOAP_PTR_ALLOC(groupID);

    res->mode=mode;    	
	STRING_TO_SOAP(groupID,res->groupID->value);
	return res;
}



/* srm2:ArrayOfTUserPermission: */
/*struct srm2__ArrayOfTUserPermission
{
	int __sizeuserPermissionArray;	 sequence of elements <userPermissionArray> 
	struct srm2__TUserPermission **userPermissionArray;
};
*/

#define GEN_TEST_ARR_INIT(_typeName,N,_fieldName,...) \
	int i;	\
	struct srm2__ArrayOf ## _typeName* res;	\
	SOAP_MAIN_ALLOC;	\
	res->__size##_fieldName##Array=N;	\
	res->_fieldName##Array = soap_malloc(&soap,res->__size##_fieldName##Array*sizeof(_typeName));	\
	if(N && !res->_fieldName##Array) return NULL;	\
	for(i=0;i<N;i++)						\
	{										\
		k[0]=(char)i+'0';					\
		res->_fieldName##Array[i]=gentest_srm2__##_typeName(tc,&soap, __VA_ARGS__);	\
		CuAssertPtrNotNull(tc,res->_fieldName##Array[i]);	\
	};										\
	return res;	
	

#define BIGARRSIZE 10
struct srm2__ArrayOfTUserPermission* gentest_srm2__ArrayOfTUserPermission(CuTest* tc, struct soap* _soap)
{
	char str1[100];
	char str2[100];
	char k[3];
	memset(k,0,3);	
	strcpy(str1,DUMMY_USERID);
	SET_SOAP;
	GEN_TEST_ARR_INIT(TUserPermission,BIGARRSIZE,userPermission, i%7, strcat(str1,k));
}

#define DUMMY_GROUPID "Some_dummy_groupid"

struct srm2__ArrayOfTGroupPermission* gentest_srm2__ArrayOfTGroupPermission(CuTest* tc, struct soap* _soap)
{
	char str1[100];
	char str2[100];
	char k[3];	
	memset(k,0,3);	
	strcpy(str1,DUMMY_GROUPID);
	SET_SOAP;
	GEN_TEST_ARR_INIT(TGroupPermission,BIGARRSIZE, groupPermission,i%7, strcat(str1,k));	
}

/*struct srm2__TOtherPermission
{
	enum srm2__TPermissionMode mode;
};
*/
struct srm2__TOtherPermission* gentest_srm2__TOtherPermission(CuTest* tc, struct soap* _soap, enum srm2__TPermissionMode mode)
{
	SET_SOAP;
	struct srm2__TOtherPermission* res;
    SOAP_MAIN_ALLOC;
    res->mode=mode;
    return res;			
}

/*
struct srm2__TMetaDataPathDetail
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
*/
struct srm2__TMetaDataPathDetail* gentest_srm2__TMetaDataPathDetail(CuTest* tc, struct soap* _soap, int _genSub,
	char* path,
	struct srm2__TReturnStatus *status,
	long size,
	struct srm2__TOwnerPermission *ownerPermission,
	struct srm2__ArrayOfTUserPermission *userPermissions,
	struct srm2__ArrayOfTGroupPermission *groupPermissions,
	struct srm2__TOtherPermission *otherPermission,
	char* createdAtTime,
	char* lastModificationTime,
	TUserID owner,
	enum srm2__TFileStorageType fileStorageType,
	enum srm2__TFileType type,
	long long lifetimeAssigned,
	long long  lifetimeLeft,
	char* checkSumType,
	char* checkSumValue,
	TSURL originalSURL,
	struct srm2__ArrayOfTMetaDataPathDetail *subPaths
	)
{
	struct srm2__TMetaDataPathDetail* res;
	SET_SOAP;

    SOAP_MAIN_ALLOC;
    SOAP_PTR_ALLOC(size);
    SOAP_PTR_ALLOC(createdAtTime);
	SOAP_PTR_ALLOC(lastModificationTime);
	SOAP_PTR_ALLOC(owner);
	SOAP_PTR_ALLOC(fileStorageType);
	SOAP_PTR_ALLOC(type);
	SOAP_PTR_ALLOC(lifetimeAssigned);
	SOAP_PTR_ALLOC(lifetimeLeft);
	SOAP_PTR_ALLOC(checkSumType);
	SOAP_PTR_ALLOC(checkSumValue);
	SOAP_PTR_ALLOC(originalSURL);
    
    STRING_TO_SOAP(path,res->path);
	res->status=status;	
	NUM_TO_SOAP_EMB(size);
	res->ownerPermission=ownerPermission;
	res->userPermissions=userPermissions;
	res->groupPermissions=groupPermissions;
	res->otherPermission=otherPermission;
	STRING_TO_SOAP_EMB(createdAtTime);
	STRING_TO_SOAP_EMB(lastModificationTime);
	STRING_TO_SOAP_EMB(owner);
	NUM_TO_SOAP_PTR(fileStorageType);
	NUM_TO_SOAP_PTR(type);
	NUM_TO_SOAP_EMB(lifetimeAssigned);
	NUM_TO_SOAP_EMB(lifetimeLeft);
	STRING_TO_SOAP_EMB(checkSumType);
	STRING_TO_SOAP_EMB(checkSumValue);
	STRING_TO_SOAP_EMB(originalSURL);
	res->subPaths=subPaths;
    return res;			
}	



/*struct srm2__ArrayOfTMetaDataPathDetail
{
	int __sizepathDetailArray;	 sequence of elements <pathDetailArray> 
	struct srm2__TMetaDataPathDetail **pathDetailArray;
};
*/

#define DUMMY_TIME "2000-01-"
#define DUMMY_CHECKSUM "XXXXYYYY-AAAAA-"
#define DUMMY_CHECKSUMVAL	"AASOME_CHECKBB"


struct srm2__ArrayOfTMetaDataPathDetail* gentest_srm2__ArrayOfTMetaDataPathDetail(CuTest* tc,struct soap* _soap, int* _genSub)
{
	char path[100];
	char str1[100];
	char str2[100];
	char str3[100];
	char str4[100];
	
	
	SET_SOAP;
	
	char k[3];
	memset(k,0,3);	
	strcpy(path,DUMMY_LFN);
	strcpy(str1,DUMMY_EXPLANATION);
	strcpy(str2,DUMMY_TIME);
	strcpy(str3,DUMMY_CHECKSUM);
	strcpy(str4,DUMMY_LFN1);
	

	if(!(*_genSub))return NULL;
	else (*_genSub)--;
	

	GEN_TEST_ARR_INIT(TMetaDataPathDetail,BIGARRSIZE,pathDetail,	
	*_genSub,
	strcat(path,k),
	gentest_srm2__TReturnStatus(tc,&soap,strcat(str1,k),SRM_AUTHENTICATION_FAILURE),
	(*_genSub)+100000+i,//size
	gentest_srm2__TOwnerPermission(tc,&soap,i%7),//	struct srm2__TOwnerPermission *ownerPermission,
	gentest_srm2__ArrayOfTUserPermission(tc,&soap),		//	struct srm2__ArrayOfTUserPermission *userPermissions,
	gentest_srm2__ArrayOfTGroupPermission(tc,&soap),		//	struct srm2__ArrayOfTGroupPermission *groupPermissions,
	gentest_srm2__TOtherPermission(tc,&soap,i%3+1),		//	struct srm2__TOtherPermission *otherPermission,
	strcat(str2,k),//	char* createdAtTime,
	strcat(str2,k),		//	char* lastModificationTime,
	DUMMY_USERID,		//	TUserID owner,
	i%2,		//	enum srm2__TFileStorageType fileStorageType,
	i%2,		//	enum srm2__TFileType type,
	1000+i,		//	long long lifetimeAssigned,
	10000+i,		//	long long  lifetimeLeft,
	DUMMY_CHECKSUM,		//	char* checkSumType,
	strcat(str3,k),		//	char* checkSumValue,
	strcat(str4,k),		//	TSURL originalSURL,
	gentest_srm2__ArrayOfTMetaDataPathDetail(tc,&soap,_genSub)		//	struct srm2__ArrayOfTMetaDataPathDetail *subPaths
	);


}


void TestTMetaDataPathDetail(CuTest* tc)
{
	INIT_SOAP;
	struct srm2__TMetaDataPathDetail* tdata1;
    TMetaDataPathDetail* tdata2;
    int i=0;
    char k[3];
	char path[100];
    char str1[100];
    char str2[100];    
    char str3[100];    
    char str4[100];    
    char str5[100];
    char str6[100];
    int* _genSub;
    int gSub=2;
    _genSub=&gSub;
    
	memset(k,0,3);	
	strcpy(path,DUMMY_LFN);
	strcpy(str1,DUMMY_EXPLANATION);
	strcpy(str2,DUMMY_TIME);
	strcpy(str3,DUMMY_CHECKSUM);
	strcpy(str4,DUMMY_LFN1);
	strcpy(str5,DUMMY_USERID);
	strcpy(str6,DUMMY_CHECKSUM);	
	
    k[0]='0';	
    //create testdata 
    tdata1 = gentest_srm2__TMetaDataPathDetail(tc,&soap,
	*_genSub,
	strcat(path,k),
	gentest_srm2__TReturnStatus(tc,&soap,strcat(str1,k),SRM_AUTHENTICATION_FAILURE),
	*_genSub+100000+i,//size
	gentest_srm2__TOwnerPermission(tc,&soap,i%7),//	struct srm2__TOwnerPermission *ownerPermission,
	gentest_srm2__ArrayOfTUserPermission(tc,&soap),		//	struct srm2__ArrayOfTUserPermission *userPermissions,
	gentest_srm2__ArrayOfTGroupPermission(tc,&soap),		//	struct srm2__ArrayOfTGroupPermission *groupPermissions,
	gentest_srm2__TOtherPermission(tc,&soap,i%3+1),		//	struct srm2__TOtherPermission *otherPermission,
	strcat(str2,k),//	char* createdAtTime,
	strcat(str2,k),		//	char* lastModificationTime,
	DUMMY_USERID,		//	TUserID owner,
	i%2,		//	enum srm2__TFileStorageType fileStorageType,
	i%2,		//	enum srm2__TFileType type,
	1000+i,		//	long long lifetimeAssigned,
	10000+i,		//	long long  lifetimeLeft,
	DUMMY_CHECKSUM,		//	char* checkSumType,
	strcat(str3,k),		//	char* checkSumValue,
	strcat(str4,k),		//	TSURL originalSURL,
	gentest_srm2__ArrayOfTMetaDataPathDetail(tc,&soap,_genSub)		//	struct srm2__ArrayOfTMetaDataPathDetail *subPaths
	);
   	
   	//convert
   	tdata2 = conv2gfal_TMetaDataPathDetail(tdata1);   	
   	//test data
	/*
struct srm2__TMetaDataPathDetail
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
*/
	
	CuAssertStrEquals(tc,path,tdata2->path);
	
	CuAssertIntEquals(tc,0,tdata2->ownerPermission);
	CuAssertIntEquals(tc,1,tdata2->otherPermission);
	
	CuAssertIntEquals(tc,i%2,tdata2->fileStorageType);
	CuAssertIntEquals(tc,i%2,tdata2->type);
	CuAssertIntEquals(tc,1000+i,tdata2->lifetimeAssigned);		//	long long lifetimeAssigned,
	CuAssertIntEquals(tc,10000+i,tdata2->lifetimeLeft);		//	long long  lifetimeLeft,
	
	CuAssertStrEquals(tc,str2,tdata2->createdAtTime);					//		struct srm2__TGMTTime *createdAtTime;
	CuAssertStrEquals(tc,str2,tdata2->lastModificationTime);				//	struct srm2__TGMTTime *lastModificationTime;
	CuAssertStrEquals(tc,str5,tdata2->owner);					//	struct srm2__TUserID *owner;	

	//check arrays too
	#warning todo: check arrays too
	
	CuAssertStrEquals(tc,str6, tdata2->checkSumType);
	CuAssertStrEquals(tc,str3,tdata2->checkSumValue);
	CuAssertStrEquals(tc,str4,tdata2->originalSURL);
	
	//free heap tdata2
	freeType_TMetaDataPathDetail(tdata2);
 
	END_SOAP;
}


/*
 * struct srm2__srmLsResponse
{
	struct srm2__ArrayOfTMetaDataPathDetail *details;
	struct srm2__TReturnStatus *returnStatus;
};
*/
struct srm2__srmLsResponse* gentest_srmLsResponse(CuTest* tc,struct soap* _soap,
	struct srm2__ArrayOfTMetaDataPathDetail *details,
	struct srm2__TReturnStatus *returnStatus
	)
{
	SET_SOAP;	
	struct srm2__srmLsResponse* res;
    SOAP_MAIN_ALLOC;
	res->returnStatus = returnStatus;
	res->details = details;	
	return res;		
}

void TestsrmLsResponse(CuTest* tc)
{
	INIT_SOAP;
     
    struct srm2__srmLsResponse* tdata1;
    srmLsResponse* tdata2;
    int i;
    char k[2];
    char str1[100];
    char str2[100];    
    char str3[100];    
    int genSubDirs =1;
       
    
    //create testdata 
    tdata1 = gentest_srmLsResponse(tc,&soap,
	    gentest_srm2__ArrayOfTMetaDataPathDetail(tc,&soap, &genSubDirs),
    	gentest_srm2__TReturnStatus(tc,&soap,(char*)DUMMY_EXPLANATION,SRM_AUTHENTICATION_FAILURE)
    	);
   	
    //create srm2 from regular    
    tdata2 = conv2gfal_srmLsResponse(tdata1);  
    
    //check all the data...
    
    
    
	freeType_srmLsResponse(tdata2);
	END_SOAP;		
}


/*typedef struct _srmCheckPermissionRequest
{
	ArrayOfTSURLInfo *arrayOfSiteURLs;
	TUserID userID;
	short int checkInLocalCacheOnly;	
} srmCheckPermissionRequest;
*/

void TestsrmCheckPermissionRequest(CuTest* tc)
{
	INIT_SOAP;
    srmCheckPermissionRequest* tdata1;     
    struct srm2__srmCheckPermissionRequest* tdata2;
	INIT_CTX_PTR;
    int i;
    char k[2];
    char str1[100];
    char str2[100];    
    char str3[100];    
    int genSubDirs =1;
  
    //create testdata 
    tdata1 = gen_srmCheckPermissionRequest(&ctx,		    
		    strdup(DUMMY_USERID),
		    gentest_ArrayOfTSURLInfo(tc),
		    1 //TRUE
    	);
   	
    //create srm2 from regular    
    tdata2 = conv2soap_srmCheckPermissionRequest(&soap,tdata1);
    
    //check all the data...    
	for(i=0;i<NSIZE;i++)
    {
    CuAssertStrEquals(tc,(char*)tdata1->arrayOfSiteURLs->surlInfoArray[i]->SURLOrStFN,(char*)tdata2->arrayOfSiteURLs->surlInfoArray[i]->SURLOrStFN->value);
    CuAssertStrEquals(tc,(char*)tdata1->arrayOfSiteURLs->surlInfoArray[i]->storageSystemInfo,(char*)tdata2->arrayOfSiteURLs->surlInfoArray[i]->storageSystemInfo->value);
    }
    
	freeType_srmCheckPermissionRequest(tdata1);
	END_SOAP;		
}

#define GENTEST_CALL_DEF(_typeName,...)	\
struct srm2__##_typeName* gentest_srm2__##_typeName(CuTest* tc, struct soap* _soap, ##__VA_ARGS__)

GENTEST_CALL_DEF(TSURLPermissionReturn,	
	struct srm2__TReturnStatus *status,
	char* surl,
	enum srm2__TPermissionMode userPermission)
{
	
	struct srm2__TSURLPermissionReturn* res;
	SET_SOAP;

    SOAP_MAIN_ALLOC;    
    SOAP_PTR_ALLOC(userPermission);
    SOAP_PTR_ALLOC(surl);
    
    STRING_TO_SOAP(surl,res->surl->value);
	res->status=status;	
	*res->userPermission=userPermission;
	return res;		

}
/*struct srm2__TSURLPermissionReturn
{
	struct srm2__TReturnStatus *status;
	struct srm2__TSURL *surl;
	enum srm2__TPermissionMode *userPermission;
};
*/

GENTEST_CALL_DEF(ArrayOfTSURLPermissionReturn)
{
	char path[100];
	char str1[100];
	char str2[100];
	char str3[100];
	char str4[100];
	
	SET_SOAP;
	char k[3];
	memset(k,0,3);	
	strcpy(path,DUMMY_LFN);
	strcpy(str1,DUMMY_EXPLANATION);
	strcpy(str2,DUMMY_TIME);
	strcpy(str3,DUMMY_CHECKSUM);
	strcpy(str4,DUMMY_LFN1);

	GEN_TEST_ARR_INIT(TSURLPermissionReturn,BIGARRSIZE,surlPermission,	
		gentest_srm2__TReturnStatus(tc,&soap,strcat(str1,k),SRM_AUTHENTICATION_FAILURE),
		strcat(str1,k),
		i%7);
}
/*struct srm2__srmCheckPermissionResponse
{
	struct srm2__ArrayOfTSURLPermissionReturn *arrayOfPermissions;
	struct srm2__TReturnStatus *returnStatus;
};
*/
GENTEST_CALL_DEF(srmCheckPermissionResponse,
	struct srm2__TReturnStatus* returnStatus,
	struct srm2__ArrayOfTSURLPermissionReturn* arrayOfPermissions
	)
{
	SET_SOAP;
	
	struct srm2__srmCheckPermissionResponse* res;
    SOAP_MAIN_ALLOC;

	res->returnStatus = returnStatus;
	res->arrayOfPermissions = arrayOfPermissions;	
	return res;		
}	


void TestsrmCheckPermissionResponse(CuTest* tc)
{
	INIT_SOAP;
     
    struct srm2__srmCheckPermissionResponse* tdata1;
    srmCheckPermissionResponse* tdata2;
    int i;
    char k[2];
    char str1[100];
    char str2[100];    
    char str3[100];    
      
    //create testdata 
    tdata1 = gentest_srm2__srmCheckPermissionResponse(tc,&soap,
       	gentest_srm2__TReturnStatus(tc,&soap,(char*)DUMMY_EXPLANATION,SRM_AUTHENTICATION_FAILURE),
	    gentest_srm2__ArrayOfTSURLPermissionReturn(tc,&soap)
    	);   	
    //create srm2 from regular    
    tdata2 = conv2gfal_srmCheckPermissionResponse(tdata1);  
    
    //check all the data... 
    
    
       
    
	freeType_srmCheckPermissionResponse(tdata2);
	END_SOAP;		
}


