<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet 
    xmlns:wsdd="http://xml.apache.org/axis/wsdd/"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:wsdl="http://schemas.xmlsoap.org/wsdl/"
    xmlns:soap="http://schemas.xmlsoap.org/soap/encoding/"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:gsoap="http://www.cern.ch/gsoaptrans"
    version='2.0'>
    <xsl:output method="text" version="2.0" encoding="UTF-8"/>
    

<xsl:template name="ifce_h_header">
        <xsl:param name="lsrmVer"/>
        <xsl:param name="lsrmPre"/>


#ifndef <xsl:value-of select="$lsrmVer"/>_IFCE_H_
#define <xsl:value-of select="$lsrmVer"/>_IFCE_H_

#include "gfal_base.h"

#undef GFAL_WRAP_DEC_SURL
#define GFAL_WRAP_DEC_SURL(_typeName)	\
    int <xsl:value-of select="$lsrmPre"/>##_typeName(<xsl:value-of select="$lsrmPre"/>##_typeName##Request* elem,<xsl:value-of select="$lsrmPre"/>##_typeName##Response* res, gfal_ctx* ctx)

#undef GFAL_WRAP_DEC
#define GFAL_WRAP_DEC(_typeName)	\
int <xsl:value-of select="$lsrmPre"/>##_typeName(<xsl:value-of select="$lsrmPre"/>##_typeName##Request* elem,<xsl:value-of select="$lsrmPre"/>##_typeName##Response* res, char* srm_endpoint, gfal_ctx* ctx)
    
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
typedef struct _<xsl:value-of select="$lsrmPre"/>##ArrayOf##_name 	\
{ 										\
int _sizeN;							\
<xsl:value-of select="$lsrmPre"/>##_name** arrN;				\
} <xsl:value-of select="$lsrmPre"/>ArrayOf##_name;				

//could be the same but for clarity let's distinguish
#undef DEF_GFAL_ARRAY
#define DEF_GFAL_ARRAY(_typeName,fieldName)	\
typedef struct _<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName	\
{									\
int __size##fieldName;	\
<xsl:value-of select="$lsrmPre"/>##_typeName** fieldName;	\
} <xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName;

#undef INIT_CTX_PTR
#define INIT_CTX_PTR	\
gfal_ctx ctx;		\
strcpy((char*)&amp;ctx.user, &quot;&quot;);

// generates new struct out of fields args.
#undef  CONSTRUCTOR_DEC
#define CONSTRUCTOR_DEC(_n,...) \
<xsl:value-of select="$lsrmPre"/>##_n* <xsl:value-of select="$lsrmPre"/>gen_##_n(gfal_ctx* ctx,##__VA_ARGS__)

#undef CONSTRUCTOR_ARRAY_DEC
#define CONSTRUCTOR_ARRAY_DEC(_type,_Name)	\
CONSTRUCTOR_DEC(ArrayOf##_type,<xsl:value-of select="$lsrmPre"/>##_type** _Name ,int _size);

// free dynamic objects' fields
#define GFAL_FREEARRAY_TYPE_DEC(_typeName)	\
void freeType_<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName(<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName* _elem);

#define CALL_GFAL_FREEARRAY_TYPE(_typeName,ptrName);	\
freeType_<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName(ptrName);

#define GFAL_FREE_TYPE_DEC(_typeName) void freeType_<xsl:value-of select="$lsrmPre"/>##_typeName(<xsl:value-of select="$lsrmPre"/>##_typeName* _arg)

#define GFAL_FREEARRAY_DEC(_name)	\
void freeArray_<xsl:value-of select="$lsrmPre"/>##_name(int i, _name** ptrArr)

//getter for single surl to get the endpoint
#undef getSURL_DEC
#define getSURL_DEC(_typeName)	char* get_<xsl:value-of select="$lsrmPre"/>##_typeName##_TSurl(<xsl:value-of select="$lsrmPre"/>##_typeName* _arg)


#define STRUCT_DEC(_n,...)		\
typedef struct _<xsl:value-of select="$lsrmPre"/>##_n {	\
__VA_ARGS__ }				\
<xsl:value-of select="$lsrmPre"/>##_n;

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
#define TYPEDEFCHAR(_name) typedef const char* <xsl:value-of select="$lsrmPre"/>##_name;
#define TYPEDEFSUB(_name1,_name2) typedef <xsl:value-of select="$lsrmPre"/>##_name1 <xsl:value-of select="$lsrmPre"/>##_name2;

#define TYPEDENUM_DEC(_n,...)	\
enum <xsl:value-of select="$lsrmPre"/>##_n {__VA_ARGS__};		\
typedef enum <xsl:value-of select="$lsrmPre"/>##_n <xsl:value-of select="$lsrmPre"/>##_n;
//predef types

STRUCT_DEC(ArrayOfString,	
int __sizestringArray;
char **stringArray;
);   

void freeType_<xsl:value-of select="$lsrmPre"/>ArrayOfString(<xsl:value-of select="$lsrmPre"/>ArrayOfString* _elem);	
CONSTRUCTOR_DEC(ArrayOfString,char** stringArray ,int _size);
getSURL_DEC(ArrayOfString);	
	
STRUCT_DEC(ArrayOfUnsignedLong,	
int __sizeunsignedLongArray;
ULONG64 **unsignedLongArray;
);
void freeType_<xsl:value-of select="$lsrmPre"/>ArrayOfUnsignedLong(<xsl:value-of select="$lsrmPre"/>ArrayOfUnsignedLong* _elem);	
CONSTRUCTOR_DEC(ArrayOfUnsignedLong,ULONG64** unsignedLongArray ,int _size);	
	
/// typedefs
TYPEDENUM_DEC(boolean ,<xsl:value-of select="$lsrmPre"/>false_ = 0, <xsl:value-of select="$lsrmPre"/>true_ = 1);    
        
</xsl:template>    

    
    
<xsl:template name="conv_h_header">
    <xsl:param name="lsrmVer"/>
    <xsl:param name="lsrmPre"/>    

#ifndef <xsl:value-of select="$lsrmPre"/>SOAP_CONVERSION_H_
#define <xsl:value-of select="$lsrmPre"/>SOAP_CONVERSION_H_

#include "<xsl:value-of select="$lsrmVer"/>Stub.h"
#include "<xsl:value-of select="$lsrmVer"/>_ifce.h"

/*
* Conversion methods for gsoap to gfal types
* Types are defined in gfal_api.h.
* For arrays we always use compound type with ptr and sizeArr.
* 
* &gt;&gt;Convention&gt;&gt;:
* Embedded structures if hold pointers, such as arrays wrappers 
* are hold as pointers for unification of freeing.
*/


#define DEF_GFALCONV_HEADERS_IN(_name)	\
struct <xsl:value-of select="$lsrmPre"/>_##_name * conv2soap_<xsl:value-of select="$lsrmPre"/>##_name(struct soap *soap, const <xsl:value-of select="$lsrmPre"/>##_name * _elem)

	
#define DEF_GFALCONV_HEADERS_OUT(_name)	\
<xsl:value-of select="$lsrmPre"/>##_name * conv2gfal_<xsl:value-of select="$lsrmPre"/>##_name(const struct <xsl:value-of select="$lsrmPre"/>_##_name * _elem)


#define DEF_GFALCONV_HEADERS(_name)	\
DEF_GFALCONV_HEADERS_IN(_name); \
DEF_GFALCONV_HEADERS_OUT(_name);


#define DEF_GFALCONVENUM_DEC(_name)	\
enum <xsl:value-of select="$lsrmPre"/>_ ##_name* convEnum2soap_<xsl:value-of select="$lsrmPre"/>##_name(struct soap* soap, int _res);

//in
ULONG64 conv2soap_<xsl:value-of select="$lsrmPre"/>unsignedLong(struct soap *soap, ULONG64 _elem);
char*  conv2soap_<xsl:value-of select="$lsrmPre"/>String(struct soap *soap, char* _elem);
//out
ULONG64 conv2gfal_<xsl:value-of select="$lsrmPre"/>unsignedLong(ULONG64* _elem);
char* conv2gfal_<xsl:value-of select="$lsrmPre"/>String(char* _elem);	
</xsl:template>    

    
<xsl:template name="ifce_c_header">
    <xsl:param name="lsrmVer"/>
    <xsl:param name="lsrmPre"/>


#include &lt;sys/types.h&gt;
#include &lt;errno.h&gt;
#include &lt;stdio.h&gt;
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "<xsl:value-of select="$lsrmVer"/>H.h"

#if _DUMA
#include "duma.h"
#endif

#include "<xsl:value-of select="$lsrmVer"/>SoapBinding+.nsmap"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif
#include "<xsl:value-of select="$lsrmVer"/>C.c"
#include "<xsl:value-of select="$lsrmVer"/>Client.c"

#include "<xsl:value-of select="$lsrmVer"/>_conversion.h"

#define COPY_CHK(_name)	\
tdata1-&gt;_name = _name;

	
#define getSURL_DEF(_n,_ret)		\
TSURL get_##_n##_TSurl(_n* _arg){	\
if(!_arg) return NULL;		\
return (char*)_arg->_ret;			\
};

#define getARRSURL_DEF(_typeName,_arrName)	\
getSURL_DEC(<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName)	\
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
#define FREE_SUB_OBJ(_name) 	if(_arg-&gt;_name)free((void*)_arg-&gt;_name);		
	
#define END_GFAL_FREE	\
free((void*)_arg);	\
_arg=NULL;	


	
#define GFAL_FREEARRAY_TYPE_DEF(_typeName,_sizeName,_arrName)	\
void freeType_<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName(<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName* _elem) 			\
{	int i;													\
if(!_elem) return;										\
for(i=0;i&lt;_elem-&gt;_sizeName;i++)							\
    {													\
    if(_elem->_arrName[i]) 								\
    freeType_<xsl:value-of select="$lsrmPre"/>##_typeName(_elem-&gt;_arrName[i]);		\
    };													\
    free((void*)_elem-&gt;_arrName);							\
    free((void*)_elem);										\
    _elem = NULL;											\
}        
static int
<xsl:value-of select="$lsrmPre"/>init (struct soap *soap, const char *surl, char *srm_endpoint,
int srm_endpointsz, char *errbuf, int errbufsz)
{
int flags;
char *sfn;

if (parsesurl (surl, srm_endpoint, srm_endpointsz, &amp;sfn, errbuf, errbufsz) &lt; 0)
return (-1);

soap_init (soap);


#ifdef GFAL_SECURE
flags = CGSI_OPT_DISABLE_NAME_CHECK;
soap_register_plugin_arg (soap, client_cgsi_plugin, &amp;flags);
#endif
return (0);
}



#define INTERNAL_SOAP_BODY_DECL	\
int ret;\
struct soap soap;


#define DEF_INOUT(_typeName)						\
struct <xsl:value-of select="$lsrmPre"/>_##_typeName##Response_ _res;	\
struct <xsl:value-of select="$lsrmPre"/>_##_typeName##Request* _req;


#define STANDARD_SURL_CHECK_INIT(_surl)	\
if (<xsl:value-of select="$lsrmPre"/>init (&amp;soap, _surl, srm_endpoint, sizeof(srm_endpoint), ctx->errbuf, GFAL_ERRSIZE) &lt; 0)	\
return (-1);	\
soap.send_timeout = ctx->soap_timeout;	\
soap.recv_timeout = ctx->soap_timeout;	


#define STANDARD_CLEANUP_RETURN(_n)	\
soap_end (&amp;soap);	\
soap_done (&amp;soap);	\
return (_n);

#define HANDLE_SOAP_ERROR	\
if (soap.error == SOAP_EOF) {	\
	gfal_errmsg(&amp;ctx->errbuf, GFAL_ERRSIZE, "connection fails or timeout");	\
	ctx->errnom = soap.error;	\
	STANDARD_CLEANUP_RETURN(-1);	\
	}	\
if(ret == SOAP_FAULT || ret == SOAP_CLI_FAULT)	\
	gfal_errmsg(&amp;ctx->errbuf, GFAL_ERRSIZE, soap.fault-&gt;faultstring);	\
STANDARD_CLEANUP_RETURN(-1);


// all manipulation methods are defined for it like gen_ and freeType_.

#define CALL_SOAP(_call) soap_call_<xsl:value-of select="$lsrmPre"/>_##_call(&amp;soap,"_call", #_call, _req, &amp;_res)

//	ArrayofString predefined
char* get_<xsl:value-of select="$lsrmPre"/>ArrayOfString_TSurl(<xsl:value-of select="$lsrmPre"/>ArrayOfString* _arg)
	{
		if(!_arg || !_arg->stringArray[0])return NULL;
		return _arg->stringArray[0];
	}
</xsl:template>

    
<xsl:template name="conv_c_header">
        <xsl:param name="lsrmVer"/>
        <xsl:param name="lsrmPre"/>
        
#include "<xsl:value-of select="$lsrmVer"/>_conversion.h"
#warning pass ctx to all functions and make wrapper for tc form the context or just set ctx error and return with null 

/*
* Conversion routines for gfal&lt;-&gt;SOAP interoperability
*/

// helpers
#define SOAP_MAIN_ALLOC	\
if (!_elem) return NULL;	\
res = soap_malloc(soap,sizeof(*res));	\
if(!res) return NULL;	\
memset(res, 0, sizeof(*res));

#define SOAP_PTR_ALLOC(_name)	\
res-&gt;_name = soap_malloc(soap,sizeof(*res-&gt;_name));	\
if(!res-&gt;_name) return NULL;

#define SOAP_PTR_ARRAY_ALLOC(_name,_size)	\
res-&gt;_name = soap_malloc(soap,sizeof(*res-&gt;_name) * _elem-&gt;_size);	\
if(!res-&gt;_name) return NULL;

#define GFAL_PTR_ALLOC(_name)	\
res-&gt;_name = malloc(sizeof(*(res-&gt;_name)));	\
if(!res-&gt;_name) return NULL;	

// enum handling
#define DEF_GFALCONVENUM_DEF(_name)	\
enum <xsl:value-of select="$lsrmPre"/>_##_name* convEnum2soap_<xsl:value-of select="$lsrmPre"/>##_name(struct soap* soap, int _res) \
{ 	enum <xsl:value-of select="$lsrmPre"/>_##_name* res;	\
res = soap_malloc(soap,sizeof(*res));	\
if(!res) return NULL;	\
*res =(enum <xsl:value-of select="$lsrmPre"/>_##_name)_res;	\
return res;	\
}


#define NEW_DATA(_p) _p = malloc(sizeof(*_p));


#define getSURL_DEF(_n,_ret)		\
TSURL get_##_n##_TSurl(_n* _arg){	\
if(!_arg) return NULL;		\
return (char*)_arg-&gt;_ret;			\
};

#define getARRSURL_DEF(_typeName,_arrName)	\
getSURL_DEC(ArrayOf##_typeName)	\
{						\
if(!_arg || (_arg-&gt;_arrName[0])) return NULL;			\
return get_##_typeName##_TSurl(_arg-&gt;_arrName[0]);		\
}

#define getSUBSURL_DEF(_typeName,_subType,_subName)					\
getSURL_DEC(_typeName)										\
{															\
if(!_arg || (_arg-&gt;_subName)) return NULL;			\
return get_##_subType##_TSurl(_arg-&gt;_subName);	\
}


#define STRING_TO_SOAP(src, dest) 		\
{ 									\
dest = soap_strdup(soap, src); 	\
if (src &amp;&amp; !dest) 				\
return NULL; 				\
} 

#define STRING_TO_SOAP_EMB(_name) \
{ \
	res-&gt;_name = soap_strdup(soap, _elem-&gt;_name); \
	if (_elem-&gt;_name &amp;&amp; !res-&gt;_name) \
	return NULL; \
} 


#define NUM_TO_SOAP(f) \
res-&gt;f = _elem-&gt;f

#define NUM_TO_SOAP_EMB(f) \
res-&gt;f-&gt;value = _elem-&gt;f

#define STRING_FROM_SOAP(src, dest) \
dest = strdup(src);	\
if(!dest) return NULL;

#define STRING_FROM_SOAP_EMB(f) \
res-&gt;f = strdup(_elem-&gt;f);	\
if(!res-&gt;f) return NULL;


#define NUM_FROM_SOAP_EMB(f) \
res-&gt;f = _elem-&gt;f-&gt;value


#define NUM_FROM_SOAP(f) \
res-&gt;f = _elem-&gt;f;

#define NUM_FROM_SOAP_PTR(f) \
res-&gt;f = *(_elem-&gt;f);
    
    
#define CONV2SOAP_OBJ(_type,_name)	\
res-&gt;_name=conv2soap_<xsl:value-of select="$lsrmPre"/>##_type(soap,(_elem-&gt;_name));	

#define CONV2GFAL_OBJ(_type,_name)	\
res-&gt;_name=conv2gfal_<xsl:value-of select="$lsrmPre"/>##_type(_elem-&gt;_name);	


#define GEN_ARRAY_CONV2SOAP(_typeName,_sizeName,_arrName) 	\
DEF_GFALCONV_HEADERS_IN(ArrayOf##_typeName){    \
int i;	\
struct <xsl:value-of select="$lsrmPre"/>_ArrayOf##_typeName* res;	\
SOAP_MAIN_ALLOC;	\
res-&gt;_sizeName=_elem-&gt;_sizeName;	\
res-&gt;_arrName = soap_malloc(soap,res-&gt;_sizeName*sizeof(*(res-&gt;_arrName)));	\
if(_elem-&gt;_sizeName &amp;&amp; !res-&gt;_arrName) return NULL;	\
for(i=0;i&lt;_elem-&gt;_sizeName;i++)	\
{	\
res-&gt;_arrName[i]=conv2soap_<xsl:value-of select="$lsrmPre"/>##_typeName(soap,_elem-&gt;_arrName[i]);	\
if(!res-&gt;_arrName[i]) return NULL;	\
};    \
return res;} 

/*	
	i.e. GEN_ARRAY_CONV2SOAP_SIMPLE(ArayOfUnsignedLong,ULONG64,__sizeunsignedLongArray,unsignedLongArray); 
*/
	
#define GEN_ARRAY_CONV2SOAP_SIMPLE(_typeName,_subTypeName,_sizeName,_arrName) 	\
DEF_GFALCONV_HEADERS_IN(_typeName){    \
int i;	\
struct <xsl:value-of select="$lsrmPre"/>_##_typeName* res;	\
SOAP_MAIN_ALLOC;	\
res-&gt;_sizeName=_elem-&gt;_sizeName;	\
res-&gt;_arrName = soap_malloc(soap,res-&gt;_sizeName*sizeof(*(res-&gt;_arrName)));	\
if(_elem-&gt;_sizeName &amp;&amp; !res-&gt;_arrName) return NULL;	\
for(i=0;i&lt;_elem-&gt;_sizeName;i++)	\
{	\
res-&gt;_arrName[i]=conv2soap_<xsl:value-of select="$lsrmPre"/>##_subTypeName(soap,_elem-&gt;_arrName[i]);	\
if(!res-&gt;_arrName[i]) return NULL;	\
};    \
return res;} 
	
	

#define GEN_ARRAY_CONV2SOAP2(_typeName,_sizeName,_arrName) 	\
int i;	\
struct <xsl:value-of select="$lsrmPre"/>_ArrayOf##_typeName* res;	\
SOAP_MAIN_ALLOC;	\
res-&gt;_sizeName=_elem-&gt;_sizeName;	\
res-&gt;_arrName = soap_malloc(soap,res-&gt;_sizeName*sizeof(*res-&gt;_arrName));	\
if(_elem-&gt;_sizeName &amp;&amp; !res-&gt;_arrName) return NULL;	\
for(i=0;i&lt;_elem-&gt;_sizeName;i++)	\
{	\
res-&gt;_arrName[i]=conv2soap_<xsl:value-of select="$lsrmPre"/>##_typeName(soap,(_typeName)&amp;_elem[i]);	\
if(!res-&gt;_arrName[i]) return NULL;	\
};


#define GFAL_DECL_ALLOC(_name)	\
<xsl:value-of select="$lsrmPre"/>##_name* res;	\
if(!_elem) return NULL;	\
res = (<xsl:value-of select="$lsrmPre"/>##_name*) malloc (sizeof(<xsl:value-of select="$lsrmPre"/>##_name));	\
if(!res) return NULL;

#define GEN_ARRAY_CONV2GFAL(_typeName,_sizeName,_arrName)	\
DEF_GFALCONV_HEADERS_OUT(ArrayOf##_typeName){            \
    <xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName* ret;					\
    int i;										\
    if(!_elem) return NULL;						\
        ret = malloc(sizeof(<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName));	\
        ret-&gt;_arrName = malloc(_elem-&gt;_sizeName*sizeof(*ret-&gt;_arrName));	\
        if ((_elem-&gt;_sizeName &amp;&amp; !ret) || !ret-&gt;_arrName)				\
        {											\
        /* err_outofmemory(ctx);*/				\
        return NULL;							\
        }											\
    for(i = 0; i&lt; _elem-&gt;_sizeName;i++)			\
    {											\
    if(_elem-&gt;_arrName[i])					\
    {										\
    ret-&gt;_arrName[i] = conv2gfal_<xsl:value-of select="$lsrmPre"/>##_typeName(_elem-&gt;_arrName[i]);	\
    if(!ret-&gt;_arrName[i])				\
    {									\
    ret-&gt;_sizeName = i+1;			\
    freeType_<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName(ret);	\
    return NULL;					\
    }									\
    } 										\
    else ret-&gt;_arrName[i] = NULL;			\
    }											\
    ret-&gt;_sizeName = _elem-&gt;_sizeName;        \
    return ret;}	
    

#define GEN_ARRAY_CONV2GFAL_EXT(_typeName,_sizeName,_arrName,_typeNameIn,_sizeNameIn,_arrNameIn)	\
    ArrayOf##_typeName* ret;					\
    int i;										\
    if(!_elem) return NULL;						\
    ret = malloc(sizeof(<xsl:value-of select="$lsrmPre"/>_ArrayOf##_typeName));	\
    ret-&gt;_arrName = malloc(_elem-&gt;_sizeNameIn*sizeof(*ret-&gt;_arrName));	\
    if ((_elem-&gt;_sizeNameIn &amp;&amp; !ret) || !ret-&gt;_arrName)				\
    {											\
    /* err_outofmemory(ctx);*/				\
    return NULL;							\
    }											\
    for(i = 0; i&lt; _elem-&gt;_sizeNameIn;i++)			\
    {											\
        if(_elem-&gt;_arrNameIn[i])					\
        {										\
        ret-&gt;_arrName[i] = conv2gfal_ <xsl:value-of select="$lsrmPre"/>##_typeName##_##_typeNameIn(_elem-&gt;_arrNameIn[i]);	\
        if(!ret-&gt;_arrName[i])				\
        {									\
            ret-&gt;_sizeName = i+1;			\
            freeType_<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName(ret);	\
            return NULL;					\
        }									\
        } 										\
        else ret-&gt;_arrName[i] = NULL;			\
    }											\
    ret-&gt;_sizeName = _elem-&gt;_sizeNameIn;

	
	
#define GEN_ARRAY_CONV2GFAL_SIMPLE(_typeName,_subTypeName,_sizeName,_arrName)	\
DEF_GFALCONV_HEADERS_OUT(_typeName){            \
    <xsl:value-of select="$lsrmPre"/>##_typeName* ret;					\
    int i;										\
    if(!_elem) return NULL;						\
        ret = malloc(sizeof(<xsl:value-of select="$lsrmPre"/>##_typeName));	\
        ret-&gt;_arrName = malloc(_elem-&gt;_sizeName*sizeof(*ret-&gt;_arrName));	\
        if ((_elem-&gt;_sizeName &amp;&amp; !ret) || !ret-&gt;_arrName)				\
        {											\
        /* err_outofmemory(ctx);*/				\
        return NULL;							\
        }											\
    for(i = 0; i&lt; _elem-&gt;_sizeName;i++)			\
    {											\
    if(_elem-&gt;_arrName[i])					\
    {										\
    ret-&gt;_arrName[i] = conv2gfal_<xsl:value-of select="$lsrmPre"/>##_subTypeName(_elem-&gt;_arrName[i]);	\
    if(!ret-&gt;_arrName[i])				\
    {									\
    ret-&gt;_sizeName = i+1;			\
    freeType_<xsl:value-of select="$lsrmPre"/>ArrayOf##_subTypeName(ret);	\
    return NULL;					\
    }									\
    } 										\
    else ret-&gt;_arrName[i] = NULL;			\
    }											\
    ret-&gt;_sizeName = _elem-&gt;_sizeName;        \
    return ret;}	
    



#define GFAL_FREEARRAY_TYPE_DEF(_typeName,_sizeName,_arrName)	\
void freeType_<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName(ArrayOf##_typeName* _elem) 			\
{	int i;													\
    if(!_elem) return;										\
    for(i=0;i&lt;_elem-&gt;_sizeName;i++)							\
    {													\
        if(_elem-&gt;_arrName[i]) 								\
        freeType_##_typeName(_elem-&gt;_arrName[i]);		\
    };													\
    free((void*)_elem-&gt;_arrName);							\
    free((void*)_elem);										\
    _elem = NULL;											\
}

#define GFAL_FREEARRAY(_name)					\
void freeArray_<xsl:value-of select="$lsrmPre"/>##_name(int i, _name** ptrArr) 	\
{	    for(;i&gt;0;i--){if(ptrArr[i-1]) 				\
    free(ptrArr[i-1]);};}	

#define GFAL_FREEARRAY_CALL(_name,_i,_ptr)	freeType_ArrayOf##_name(_ptr);

#define CONSTRUCTOR_ARRAY_DEF(_n,_Name)	\
CONSTRUCTOR_DEC(ArrayOf##_n,_n** _Name##Array ,int _size)	\
{												\
    <xsl:value-of select="$lsrmPre"/>_ArrayOf##_n* tdata1;    					\
    tdata1 = malloc(sizeof(*tdata1));			\
    GCuAssertPtrNotNull(tdata1);				\
    tdata1-&gt;_Name##Array = malloc(sizeof(_n*)*_size);	\
    GCuAssertPtrNotNull(tdata1-&gt;_Name##Array);	\
    tdata1-&gt;__size##_Name##Array = _size;			\
    return tdata1;								\
}



#define ENUM_TO_SOAP(_TYPE,_n)    \
    res->_n=convEnum2soap_<xsl:value-of select="$lsrmPre"/>##_TYPE(soap,_elem->_n);
#define ENUM_TO_SOAP_NOPTR(_TYPE,_n)    \
	res->_n=*((enum <xsl:value-of select="$lsrmPre"/>_##_TYPE*)(convEnum2soap_<xsl:value-of select="$lsrmPre"/>##_TYPE(soap,_elem->_n))); 
	
#define ENUM_FROM_SOAP(_n) res-&gt;_n = *(_elem-&gt;_n);
#define ENUM_FROM_SOAP_NOPTR(_n) res-&gt;_n = (_elem-&gt;_n);	
	
//////////////////////////////
#define NUM_TO_SOAP_PTR(_n) 	\
*(res-&gt;_n) = _elem-&gt;_n;
	


//in
ULONG64 conv2soap_<xsl:value-of select="$lsrmPre"/>UnsignedLong(struct soap *soap, ULONG64 _elem)
{
	return _elem; 
}	

char*  conv2soap_<xsl:value-of select="$lsrmPre"/>String(struct soap *soap, char* _elem)
{
	char * dest = soap_strdup(soap, _elem); 	
	if (!_elem || !dest)return NULL;
	return dest;
}	

//out
ULONG64 conv2gfal_<xsl:value-of select="$lsrmPre"/>UnsignedLong(ULONG64* _elem)
{
	return *_elem;
}	
	
char* conv2gfal_<xsl:value-of select="$lsrmPre"/>String(char* _elem)
{
	char * dest = strdup( _elem); 	
	if (!_elem || !dest)return NULL;
	return dest;	
}	
	

</xsl:template>

    
<xsl:template name="gfalwrapdoc" >
    <xsl:param name="typename"></xsl:param>
    <xsl:param name="lsrmPre"></xsl:param>
    <xsl:param name="isSurl" as="xs:boolean"/>

<xsl:choose>
<xsl:when test="$isSurl">
GFAL_WRAP_DEC_SURL(<xsl:value-of select="$typename"/>)
{
	//init
	char srm_endpoint[256];
	INTERNAL_SOAP_BODY_DECL;
	
	DEF_INOUT(<xsl:value-of select="$typename"/>);
	STANDARD_SURL_CHECK_INIT(get_<xsl:value-of select="$lsrmPre"/><xsl:value-of select="$typename"/>Request_TSurl(elem));
	//do translation, check for conversion problems
	_req = conv2soap_<xsl:value-of select="concat($lsrmPre,$typename)"/>Request(&amp;soap,elem);
	//execute
	if( ret =
	soap_call_<xsl:value-of select="$lsrmPre"/>_<xsl:value-of select="$typename"/>(&amp;soap,srm_endpoint, "<xsl:value-of select="$typename"/>", _req, &amp;_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_<xsl:value-of select="concat($lsrmPre,$typename)"/>Response(_res.<xsl:value-of select="$typename"/>Response);		
	STANDARD_CLEANUP_RETURN (0);	
}
</xsl:when>
<xsl:when test="not($isSurl)">
GFAL_WRAP_DEC(<xsl:value-of select="$typename"/>)
{
	//init
	INTERNAL_SOAP_BODY_DECL;
	DEF_INOUT(<xsl:value-of select="$typename"/>);
	//do translation, check for conversion problems
	_req = conv2soap_<xsl:value-of select="concat($lsrmPre,$typename)"/>Request(&amp;soap,elem);
	//execute
	if( ret =
	soap_call_<xsl:value-of select="$lsrmPre"/>_<xsl:value-of select="$typename"/>(&amp;soap,srm_endpoint, "<xsl:value-of select="$typename"/>", _req, &amp;_res))        
	{
		HANDLE_SOAP_ERROR;
	};	
	//return converted		 
	res = conv2gfal_<xsl:value-of select="concat($lsrmPre,$typename)"/>Response(_res.<xsl:value-of select="$typename"/>Response);		
	STANDARD_CLEANUP_RETURN (0);	
}
    
</xsl:when>    
</xsl:choose>        
</xsl:template>
</xsl:stylesheet>
