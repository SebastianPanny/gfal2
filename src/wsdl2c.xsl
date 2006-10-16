<xsl:stylesheet
  xmlns:wsdd="http://xml.apache.org/axis/wsdd/"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
  xmlns:wsdl="http://schemas.xmlsoap.org/wsdl/"
  xmlns:soap="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:xs="http://www.w3.org/2001/XMLSchema"
  xmlns:gsoap="http://www.cern.ch/gsoaptrans"
  version='2.0'>
<xsl:output method="text" version="2.0" encoding="UTF-8"/>

  <xsl:variable name="root" select="/"/>
 <xsl:param name="srmVer">srm2_2</xsl:param>
  <xsl:param name="srmPre">srm22_</xsl:param>
  <xsl:param name="srmPreSoap">srm22__</xsl:param>
  <xsl:param name="headerNameIfce"><xsl:value-of select="concat($srmVer,'_ifce.h')"/></xsl:param>
  <xsl:param name="bodyNameIfce"><xsl:value-of select="concat($srmVer,'_ifce.c')"/></xsl:param>
<xsl:param name="headerNameConv"><xsl:value-of select="concat($srmVer,'_conversion.h')"/></xsl:param>
<xsl:param name="bodyNameConv"><xsl:value-of select="concat($srmVer,'_conversion.c')"/></xsl:param>

    
    
<!--    <xsl:variable name="definedTypes">
        <xsl:text>xsd:boolean|ULONG|TRetentionPolicy</xsl:text>
    </xsl:variable>
-->    
    <!-- can be also used to map simple types to desired output type -->
    <xsl:variable name="defines">
        <e name="int">int</e>
        <e name="unsignedLong">ULONG64</e>
        <e name="boolean">short int</e>
        <e name="dateTime">char*</e>
        <e name="string">char* </e>
        <e name="anyURI">char* </e>
        <e name="ArrayOfAnyURI"><xsl:value-of  select="$srmPre"/>ArrayOfString* </e>
    </xsl:variable>
	<!-- variable that holds special conversion if gfal complex type is different than soap complex type but the underlying types are identical-->
	<!--STRUCT_DEC(ArrayOfString,	
	int __sizestringArray;
	char **stringArray;
		
	struct srm22__ArrayOfAnyURI
	{
		int __sizeurlArray;	/* sequence of elements <urlArray> */
		char **urlArray;
	};
		
	);   --> 	 
	<xsl:variable name="defines2gfalComplexConv">
		<par type="ArrayOfAnyURI" arr="yes" totype="ArrayOfString"> 
			<arg name="urlArray" arr="yes" toname="stringArray"/>			
		</par>
		<par type="SomeotherOfAnyURI" arr="yes" totype="OtherArrayOfString"> 
			<arg name="otherurlArray" arr="yes" toname="otherstringArray"/>			
		</par>
	</xsl:variable>
	
    <xsl:variable name="definesPtr">        
        <e name="dateTime">char*</e>
        <e name="string">char* </e>
        <e name="anyURI">char* </e>
<!--        <e name="int">NUM</e>            
        <e name="unsignedLong">NUM</e>        
-->    </xsl:variable>

	<!--	due to inconsistency of treating lack of nillable param for the simple types by gsoap2.6.2
	we have to specify how to treat lack of thereof in nonillok param.
	if nonillok=true then follows XML schema spec and lack of is as @nillable='false''
	{nillable}	The ·actual value· of the nillable [attribute], if present, otherwise false.
	if nonillok=false then as @nillable='true''
--> 
    <xsl:variable name="conversionMethods">        
        <e name="char" nonillok="1">STRING</e>
        <e name="dateTime">STRING</e>
        <e name="anyURI">STRING</e>
        <e name="string">STRING</e>
        <e name="int" nonillok="0">NUM</e>            
        <e name="unsignedLong" nonillok="1">NUM</e>
        <e name="boolean" nonillok="1">NUM</e>            
    </xsl:variable>
   <xsl:variable name="conversion2SoapSimple">        
        <e name="string">String</e>
        <e name="unsignedLong">ULONG64</e>
    </xsl:variable>    	

	
    <xsl:variable name="simpleTypes" select="($defines/e/@name)"/>            
    
    <xsl:key name="simples" match="/wsdl:definitions/wsdl:types/xs:schema/xs:simpleType/@name|defines/simpleType/@name"
        use="." />
     <xsl:key name="complex" match="/wsdl:definitions/wsdl:types/xs:schema/xs:complexType/@name"
        use="." />    
    		
    <xsl:key name="inputopsM" match="/wsdl:definitions/wsdl:portType/wsdl:operation/wsdl:input/@name"
        use="." />
    <xsl:variable name="inputHead" select="/wsdl:definitions/wsdl:portType/wsdl:operation/wsdl:input/@name"/>
    <xsl:key name="outputopsM" match="/wsdl:definitions/wsdl:portType/wsdl:operation/wsdl:output/@name"
        use="." />    
    <xsl:variable name="outputHead" select="/wsdl:definitions/wsdl:portType/wsdl:operation/wsdl:output/@name"/>    
    
    <xsl:variable  name ="mainChildrenOutputs" select="(//xs:complexType[@name=key('outputopsM',@name)]/xs:sequence/xs:element/substring-after(@type,':'))"/>
    <xsl:variable name="mainChildrenOutputsS" select="$mainChildrenOutputs, $simpleTypes"/>        
    <xsl:variable name="mainChildrenOutputsStart" select="distinct-values($mainChildrenOutputsS)"/>
    <xsl:variable name="outputT"  select="gsoap:product($mainChildrenOutputsStart)"/>
    
    <xsl:variable  name ="mainChildrenInputs" select="(//xs:complexType[@name=key('inputopsM',@name)]/xs:sequence/xs:element/substring-after(@type,':'))"/>
    <xsl:variable name="mainChildrenInputsS" select="$mainChildrenInputs, $simpleTypes"/>        
    <xsl:variable name="mainChildrenInputStart" select="distinct-values($mainChildrenInputsS)"/>    
    <xsl:variable name="inputT"  select="gsoap:product($mainChildrenInputStart)"/>
    
<!--    Types overlapping:-->
    <xsl:variable name="commonTypes" select="distinct-values($outputT[.=$inputT])" />    
    
<!--    Input types wo common:-->
    <xsl:variable name="realinputT" select="distinct-values($inputT[not(.=$commonTypes)])"/>

<!--    Output types wo common:-->
    <xsl:variable name="realoutputT" select="distinct-values($outputT[not(.=$commonTypes)])"/>
	
<!-- Special conversion complex fields
	check if the subelem is 
-->
<xsl:variable name="complexConversion" select="//xs:complexType[@name=$defines2gfalComplexConv/*[@name]]"/>
    	
    
    <xsl:include href="headers.xsl"/>
        
    
<!--            MAIN TEMPLATE           -->
<xsl:template match="/">
    <xsl:call-template name="HEADER_IFCE"/> 
    <xsl:call-template name="BODY_IFCE"/> 
   <xsl:call-template name="HEADER_CONV"/> 
    <xsl:call-template name="BODY_CONV"/>
    
    
    
 </xsl:template>
<!-- =================================================================================== -->
<!-- types -->

    <!-- hash arr tester-->
    <xsl:template match="simpleType">
        <xsl:variable name="types" select="."/>
        <xsl:for-each select="$types">            
            <xsl:variable name="type" select="string(@name)" />
            <xsl:variable name="sim" select ="string('int')"/>
            <xsl:value-of select="key('simples',$type)"/>
            <xsl:choose>
                <xsl:when test="not(string(key('simples', $sim))=$sim)">nie ma 
                </xsl:when>
                <xsl:otherwise> <xsl:text> jest  
                </xsl:text>   </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>  
    </xsl:template>
    

<!-- =================================================================================== -->
<!-- serializer for types -->
    
<!--    simple type ifce header-->
<xsl:template match="xs:simpleType" mode="serializer">
TYPEDENUM_DEC(<xsl:value-of select="@name"/>       
        <xsl:apply-templates select="xs:restriction/xs:enumeration" mode="pod"/>);       
</xsl:template>
<!--     simple type ifce body -->
<xsl:template match="xs:simpleType" mode="body">
DEF_GFALCONVENUM_DEF(<xsl:value-of select="@name"/>);
</xsl:template>
    
<xsl:template match="xs:simpleType" mode="conversion-body">DEF_GFALCONVENUM_DEF(<xsl:value-of select="@name"/>);
</xsl:template>
    
<!-- complex type ifce header    
    Depending on the type generate:
    IN    - if input or shared but no surl 
    REQ - if input or shared and surl available
    OUT - if output
-->
<xsl:template match="xs:complexType" mode="serializer">   
    <xsl:variable name="structType" select="gsoap:getTypeFlow(@name)"/>
<!--    Wez wszystkie te complex ktore maja jeden z elementow @name like '*surl*-->    
    <xsl:variable name="isSurl" select="./xs:sequence/xs:element/@name[matches(.,'[\w]*[sS][uU][rR][lL][\w]*')][1]"/>
    <xsl:variable name="inputSurl"  select="gsoap:productSurl(@name)"/>
/*
    <xsl:value-of select="@name"/>    
    is the 
    <xsl:value-of select="gsoap:getTypeFlowText(@name)"/>
<xsl:choose>
<xsl:when test="not(contains($inputSurl,'!')) and ($structType = 1 or $structType=2 or $structType=4)">
*/    
STRUCT_DEC_IN(
        </xsl:when>
<xsl:when test="(contains($inputSurl,'!')) and ($structType = 1 or $structType=2 or $structType=4)">
<xsl:if test="contains($inputSurl,'!') and not($structType = 3 or $structType=5)">
    and getSurl is for:
    <xsl:value-of select="$inputSurl"/>
</xsl:if> 
*/    
STRUCT_DEC_REQ(
        </xsl:when>
<xsl:when test="($structType = 3 or $structType=5)">
*/
STRUCT_DEC_OUT(
         </xsl:when>
    </xsl:choose>
<xsl:value-of select="@name"/>,
<xsl:apply-templates select="xs:sequence/xs:element" mode="pod"/>);
</xsl:template>

    <!-- complex type ifce header    
        Depending on the type generate:
        IN    - if input or shared but no surl 
        REQ - if input or shared and surl available
        OUT - if output
    -->
<xsl:template match="xs:complexType" mode="conversion">   
        <xsl:variable name="structType" select="gsoap:getTypeFlow(@name)"/>
        <!--    Wez wszystkie te complex ktore maja jeden z elementow @name like '*surl*-->    
        <xsl:variable name="isSurl" select="./xs:sequence/xs:element/@name[matches(.,'[\w]*[sS][uU][rR][lL][\w]*')][1]"/>
	<xsl:variable name="type" select="@name"/>
/*
    <xsl:value-of select="@name"/>    
    is the 
    <xsl:value-of select="gsoap:getTypeFlowText(@name)"/> type.
*/
<xsl:choose>
            <xsl:when test="($structType=2 or $structType=4)">
struct <xsl:value-of select="concat($srmPre,'_',@name)"/> * conv2soap_<xsl:value-of select="concat($srmPre,@name)"/>(struct soap *soap, const <xsl:value-of select="concat($srmPre,@name)"/> * _elem);
            </xsl:when>
            <xsl:when test="($structType = 1)">
struct <xsl:value-of select="concat($srmPre,'_',@name)"/> * conv2soap_<xsl:value-of select="concat($srmPre,@name)"/>(struct soap *soap, const <xsl:value-of select="concat($srmPre,@name)"/> * _elem);
<xsl:value-of select="concat($srmPre,@name)"/> * conv2gfal_<xsl:value-of select="concat($srmPre,@name)"/>(const struct <xsl:value-of select="concat($srmPre,'_',@name)"/> * _elem);
            </xsl:when>
            <xsl:when test="($structType = 3 or $structType=5)">
<xsl:value-of select="concat($srmPre,@name)"/> * conv2gfal_<xsl:value-of select="concat($srmPre,@name)"/>(const struct <xsl:value-of select="concat($srmPre,'_',@name)"/> * _elem);            	
            </xsl:when>
            <xsl:when test="($structType = 6)">
            	<xsl:variable name="complexConvTemp" select="$defines2gfalComplexConv/*[@type=$type and @arr='yes']"/>
<xsl:value-of select="concat($srmPre,$complexConvTemp/@totype)"/> * conv2gfal_<xsl:value-of select="concat($srmPreSoap,$complexConvTemp/@type,'_2_',$complexConvTemp/@totype)"/>(const struct <xsl:value-of select="concat($srmPreSoap,@name)"/> * _elem);
struct <xsl:value-of select="concat($srmPreSoap,$complexConvTemp/@type)"/> * conv2soap_<xsl:value-of select="concat($srmPreSoap,$complexConvTemp/@type,'_2_',$complexConvTemp/@totype)"/>(struct soap *soap, <xsl:value-of select="concat($srmPre,$complexConvTemp/@totype)"/> * _elem);            	
            </xsl:when>
        </xsl:choose>
</xsl:template>
    
<!--   complex type BODY IFCE-->
<xsl:template match="xs:complexType" mode="body">   
        <xsl:variable name="structType" select="gsoap:getTypeFlow(@name)"/>
        <!--    Wez wszystkie te complex ktore maja jeden z elementow @name like '*surl*-->    
	<xsl:variable name="isSurl"  select="gsoap:productSurl(@name)"/>
	<xsl:variable name="isSurlGuard"  select="contains($isSurl,':')"/>

	
/*
<xsl:value-of select="@name"/>    
is the 
<xsl:value-of select="gsoap:getTypeFlowText(@name)"/>
<xsl:if test="contains($isSurl,'!') and not($structType = 3 or $structType=5)">
    and getSurl is for:
    <xsl:value-of select="$isSurl"/>
</xsl:if> 
	
*/           
<xsl:choose>
<xsl:when test="not($isSurlGuard) and ($structType = 1 or $structType=2 or $structType=4)">
CONSTRUCTOR_DEC(<xsl:value-of select="@name"/>,
<xsl:call-template name="body">
        <xsl:with-param name="p" select="xs:sequence/xs:element"/>
        <xsl:with-param name="delim" select="string(',')"/>
</xsl:call-template>)
{
    <xsl:value-of select="concat($srmPre,@name)"/>* tdata1;
	NEW_DATA(tdata1);
	GCuAssertPtrNotNull(tdata1);
<xsl:apply-templates select="xs:sequence/xs:element" mode="body_ctor_body"/>
	return tdata1;                
}
GFAL_FREE_TYPE_DEC(<xsl:value-of select="@name"/>)
{
	if(!_arg)return;
<xsl:apply-templates select="xs:sequence/xs:element" mode="body-free"/>
	END_GFAL_FREE;
}

</xsl:when>
<xsl:when test="($isSurlGuard) and ($structType = 1 or $structType=2 or $structType=4)">
CONSTRUCTOR_DEC(<xsl:value-of select="@name"/>,
<xsl:call-template name="body">
    <xsl:with-param name="p" select="xs:sequence/xs:element"/>
    <xsl:with-param name="delim" select="string(',')"/>
</xsl:call-template>)
{
<xsl:value-of select="concat($srmPre,@name)"/>* tdata1;
    NEW_DATA(tdata1);
    GCuAssertPtrNotNull(tdata1);
<xsl:apply-templates select="xs:sequence/xs:element" mode="body_ctor_body"/>
    return tdata1;    
}                
GFAL_FREE_TYPE_DEC(<xsl:value-of select="@name"/>)
{
	if(!_arg)return;
	<xsl:apply-templates select="xs:sequence/xs:element" mode="body-free"/>
	END_GFAL_FREE;	
}
<xsl:variable name="toksMain" select="reverse(tokenize($isSurl, '->|!'))"/>	
<xsl:variable name="toks" select="(reverse(tokenize(normalize-space($toksMain[2]),':')))"/>
<xsl:variable name ="definedComplex" select="key('complex',$toks[1])"/>	
<!--check if complex type, if so call complex getter	-->
	
	
char* get_<xsl:value-of select="concat($srmPre,@name)"/>_TSurl(<xsl:value-of select="concat($srmPre,@name)"/>* _arg)
{	
	if(!_arg) return NULL;
	return (char*)<xsl:choose>
<xsl:when test="$definedComplex">get_<xsl:value-of select="concat($srmPre,$toks[1])"/>_TSurl(_arg-><xsl:value-of select="$toks[2]"/>);</xsl:when>
		<xsl:otherwise>_arg-><xsl:value-of select="$toks[2]"/>;</xsl:otherwise>
</xsl:choose>
};
</xsl:when>
    
<xsl:when test="($structType = 3 or $structType=5)">
GFAL_FREE_TYPE_DEC(<xsl:value-of select="@name"/>)
{
	if(!_arg)return;
	<xsl:apply-templates select="xs:sequence/xs:element" mode="body-free"/>
	END_GFAL_FREE;
}
                
</xsl:when>
</xsl:choose>
</xsl:template>
<!--    
    1. allocate main obj
    2. allocate all simple members
    3. convert simple types
    4. convert enum types
    5. convert complex types
    6. convert array or complex types as defined in defines2gfalComplexConv to limit the number of  types if there transitive equivalents 
-->    
<!--   complex type BODY CONVERSION-->
<xsl:template match="xs:complexType" mode="conversion-body">   
        <xsl:variable name="structType" select="gsoap:getTypeFlow(@name)"/>
        <!--    Wez wszystkie te complex ktore maja jeden z elementow @name like '*surl*-->    
        <xsl:variable name="isSurl" select="./xs:sequence/xs:element/@name[matches(.,'[\w]*[sS][uU][rR][lL][\w]*')][1]"/>
/*
<xsl:value-of select="@name"/>    
is the 
<xsl:value-of select="gsoap:getTypeFlowText(@name)"/> type.
*/           <xsl:choose>
            <xsl:when test="($structType=2 or $structType=4)">

struct <xsl:value-of select="concat($srmPreSoap,@name)"/> * conv2soap_<xsl:value-of select="concat($srmPre,@name)"/>(struct soap *soap, const <xsl:value-of select="concat($srmPre,@name)"/> * _elem)
{
	struct  <xsl:value-of select="concat($srmPreSoap,@name)"/>* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
<xsl:apply-templates select="xs:sequence/xs:element" mode="conv_in_body"/>
	return res;                
}
</xsl:when>
<xsl:when test="($structType = 1)">
struct <xsl:value-of select="$srmPre"/>_<xsl:value-of select="@name"/> * conv2soap_<xsl:value-of select="concat($srmPre,@name)"/>(struct soap *soap, const <xsl:value-of select="concat($srmPre,@name)"/> * _elem)
{
	struct  <xsl:value-of select="concat($srmPreSoap,@name)"/>* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
<xsl:apply-templates select="xs:sequence/xs:element" mode="conv_in_body"/>
	return res;                
}

//DEF_GFALCONV_HEADERS_OUT(<xsl:value-of select="@name"/>)
<xsl:value-of select="concat($srmPre,@name)"/> * conv2gfal_<xsl:value-of select="concat($srmPre,@name)"/>(const struct <xsl:value-of select="concat($srmPreSoap,@name)"/>* _elem)
{
	GFAL_DECL_ALLOC(<xsl:value-of select="@name"/>);
<xsl:apply-templates select="xs:sequence/xs:element" mode="conv_out_body"/>     
	return res;		
}                
</xsl:when>
    	
<xsl:when test="($structType = 3 or $structType=5)">
//DEF_GFALCONV_HEADERS_OUT(<xsl:value-of select="@name"/>)
<xsl:value-of select="concat($srmPre,@name)"/> * conv2gfal_<xsl:value-of select="concat($srmPre,@name)"/>(const struct <xsl:value-of select="concat($srmPreSoap,@name)"/>* _elem)
{
	GFAL_DECL_ALLOC(<xsl:value-of select="@name"/>);
<xsl:apply-templates select="xs:sequence/xs:element" mode="conv_out_body"/>     
	return res;
}
</xsl:when>
<xsl:when test="($structType = 6)">
		<xsl:variable name="itype" select="@name"/>
	<xsl:call-template name="complex-conversion-body">
	        <xsl:with-param name="itype" select="$itype"/>
	</xsl:call-template>
</xsl:when>
	
</xsl:choose>
</xsl:template>

    
<xsl:template match="xs:complexType[xs:sequence/xs:element/@maxOccurs='unbounded']" mode="body">
<xsl:if test="gsoap:checkType(xs:sequence/xs:element)=2">
    <xsl:variable name="structType" select="gsoap:getTypeFlow(@name)"/>
    <xsl:variable name="structTypeEmb" select="substring-after(xs:sequence/xs:element/@type,':')"/>
    <xsl:variable name="fieldName" select="xs:sequence/xs:element/@name"/>
	
    
/* 
<xsl:value-of select="@name"/>    
is the 
<xsl:value-of select="gsoap:getTypeFlowText(@name)"/>
*/
CONSTRUCTOR_DEC(<xsl:value-of select="@name"/>,<xsl:value-of select="concat($srmPre,$structTypeEmb)"/>** <xsl:value-of select="$fieldName"/>,int _size)	
{												
	<xsl:value-of select="concat($srmPre,@name)"/>* tdata1;    					
	tdata1 = malloc(sizeof(*tdata1));			
	GCuAssertPtrNotNull(tdata1);				
	tdata1-><xsl:value-of select="$fieldName"/> = malloc(sizeof(<xsl:value-of select="concat($srmPre,$structTypeEmb)"/>*)*_size);	
	GCuAssertPtrNotNull(tdata1-><xsl:value-of select="$fieldName"/>);	
	tdata1->__size<xsl:value-of select="$fieldName"/> = _size;	
	return tdata1;								
}
GFAL_FREEARRAY_TYPE_DEF(<xsl:value-of select="$structTypeEmb"/>,__size<xsl:value-of select="$fieldName"/>, <xsl:value-of select="$fieldName"/>);
    <xsl:variable name="elem" select="./xs:sequence/xs:element/substring-after(@type,':')"/>
    <xsl:variable name="isSurl" select="$root//xs:complexType[@name=$elem]//xs:sequence/xs:element/@name[matches(.,'[\w]*[sS][uU][rR][lL][\w]*')][1]"/>
    <xsl:if test="(exists($isSurl)) and ($structType = 1 or $structType=2 or $structType=4)">
<!--getARRSURL_DEF(<xsl:value-of select="$structTypeEmb"/>,<xsl:value-of select="$fieldName"/>);
    	getSURL_DEC(<xsl:value-of select="$lsrmPre"/>ArrayOf##_typeName)	\
-->
char* get_<xsl:value-of select="concat($srmPre,@name)"/>_TSurl(<xsl:value-of select="concat($srmPre,@name)"/>* _arg)    	
{						
	if(!_arg || !(_arg-><xsl:value-of select="$fieldName"/>[0])) return NULL;
	return get_<xsl:value-of select="concat($srmPre,$structTypeEmb)"/>_TSurl(_arg-><xsl:value-of select="$fieldName"/>[0]);
}
    	
    </xsl:if>
</xsl:if>
</xsl:template>        
    
<!--    complex array type ifce header-->
<!-- <xsl:template match="xs:complexType[starts-with(@name,'ArrayOf')]" mode="serializer">    -->
<xsl:template match="xs:complexType[xs:sequence/xs:element/@maxOccurs='unbounded']" mode="serializer">
<xsl:if test="gsoap:checkType(xs:sequence/xs:element)=2">
<xsl:variable name="structType" select="gsoap:getTypeFlow(@name)"/>
/* 
    <xsl:value-of select="@name"/>    
    is the 
    <xsl:value-of select="gsoap:getTypeFlowText(@name)"/>
*/
<xsl:variable name="typ" select="substring-after(xs:sequence/xs:element/@type,':')"/>
<xsl:variable name="nam" select="xs:sequence/xs:element/@name"/>	

DEF_GFAL_ARRAY(<xsl:value-of select="concat($typ,',',$nam)"/>);		               
GFAL_FREEARRAY_TYPE_DEC(<xsl:value-of select="$typ"/>);			
CONSTRUCTOR_DEC(ArrayOf<xsl:value-of select="$typ"/>,<xsl:value-of select="concat($srmPre,$typ)"/>** <xsl:value-of select="$nam"/>,int _size);
    <xsl:variable name="elem" select="./xs:sequence/xs:element/substring-after(@type,':')"/>
    <xsl:variable name="isSurl" select="$root//xs:complexType[@name=$elem]//xs:sequence/xs:element/@name[matches(.,'[\w]*[sS][uU][rR][lL][\w]*')][1]"/>
    <xsl:if test="(exists($isSurl)) and ($structType = 1 or $structType=2 or $structType=4)">
getSURL_DEC(<xsl:value-of select="@name"/>);
    </xsl:if>
    
</xsl:if>
</xsl:template>        

    <!-- 
    	Array conversion
    	If the type is a simple one
    	create special array with no sub-caller.    
    
    -->
<xsl:template match="xs:complexType[xs:sequence/xs:element/@maxOccurs='unbounded']" mode="conversion-body">
<!--    
	if the array embedded type is a simple one use conversion matrix to get the converter
-->
<xsl:variable name="structType" select="gsoap:getTypeFlow(@name)"/>
<xsl:variable name="type" select="substring-after(xs:sequence/xs:element/@type,':')"/>
<xsl:variable name="itype" select="@name"/>
<xsl:variable name="isSimpleConversion" select="$conversion2SoapSimple/*[@name=$type]"/>	
/*
    Array handling for:
    <xsl:value-of select="@name"/>    
    is the 
    <xsl:value-of select="gsoap:getTypeFlowText(@name)"/>
*/
<xsl:choose>
<xsl:when test="($structType = 1) and not(exists($isSimpleConversion))">
GEN_ARRAY_CONV2SOAP(<xsl:value-of select="substring-after(xs:sequence/xs:element/@type,':')"/>,__size<xsl:value-of select="xs:sequence/xs:element/@name"/>, <xsl:value-of select="xs:sequence/xs:element/@name"/>);
GEN_ARRAY_CONV2GFAL(<xsl:value-of select="substring-after(xs:sequence/xs:element/@type,':')"/>,__size<xsl:value-of select="xs:sequence/xs:element/@name"/>, <xsl:value-of select="xs:sequence/xs:element/@name"/>);    
</xsl:when>
<xsl:when test="($structType = 1) and (exists($isSimpleConversion))">
GEN_ARRAY_CONV2SOAP_SIMPLE(<xsl:value-of select="@name"/>,<xsl:value-of select="$isSimpleConversion"/>,__size<xsl:value-of select="xs:sequence/xs:element/@name"/>, <xsl:value-of select="xs:sequence/xs:element/@name"/>);
GEN_ARRAY_CONV2GFAL_SIMPLE(<xsl:value-of select="@name"/>,<xsl:value-of select="$isSimpleConversion"/>,__size<xsl:value-of select="xs:sequence/xs:element/@name"/>, <xsl:value-of select="xs:sequence/xs:element/@name"/>);    
</xsl:when>
	
<xsl:when test="($structType=2 or $structType=4) and not(exists($isSimpleConversion))">
GEN_ARRAY_CONV2SOAP(<xsl:value-of select="substring-after(xs:sequence/xs:element/@type,':')"/>,__size<xsl:value-of select="xs:sequence/xs:element/@name"/>, <xsl:value-of select="xs:sequence/xs:element/@name"/>);
</xsl:when>
<xsl:when test="($structType=2 or $structType=4) and (exists($isSimpleConversion))">
GEN_ARRAY_CONV2SOAP_SIMPLE(<xsl:value-of select="@name"/>,<xsl:value-of select="$isSimpleConversion/@name"/>,__size<xsl:value-of select="xs:sequence/xs:element/@name"/>, <xsl:value-of select="xs:sequence/xs:element/@name"/>);
</xsl:when>
	
<xsl:when test="($structType = 3 or $structType=5)">
GEN_ARRAY_CONV2GFAL(<xsl:value-of select="substring-after(xs:sequence/xs:element/@type,':')"/>,__size<xsl:value-of select="xs:sequence/xs:element/@name"/>, <xsl:value-of select="xs:sequence/xs:element/@name"/>);    
</xsl:when>
<xsl:when test="($structType = 6)">
	<xsl:call-template name="complex-conversion-body">
	        <xsl:with-param name="itype" select="$itype"/>
	</xsl:call-template>

</xsl:when>
</xsl:choose>	
	
</xsl:template>        

	
<!--structures fields decls  ifce body -->
<xsl:template name="complex-conversion-body">
    <xsl:param name="itype"/>
<xsl:variable name="complexConvTemp" select="$defines2gfalComplexConv/*[@type=$itype and @arr='yes']"/>
<xsl:value-of select="concat($srmPre,$complexConvTemp/@totype)"/> * conv2gfal_<xsl:value-of select="concat($srmPreSoap,$complexConvTemp/@type,'_2_',$complexConvTemp/@totype)"/>(const struct <xsl:value-of select="concat($srmPreSoap,@name)"/> * _elem)
{	
	GFAL_DECL_ALLOC(<xsl:value-of select="($complexConvTemp/@totype)"/>);
	//conv	
	return res;
}
	
struct <xsl:value-of select="concat($srmPreSoap,$complexConvTemp/@type)"/> * conv2soap_<xsl:value-of select="concat($srmPreSoap,$complexConvTemp/@type,'_2_',$complexConvTemp/@totype)"/>(struct soap *soap, <xsl:value-of select="concat($srmPre,$complexConvTemp/@totype)"/> * _elem)
{
	struct  <xsl:value-of select="concat($srmPreSoap,$complexConvTemp/@type)"/>* res;
	SOAP_MAIN_ALLOC;
	GCuAssertPtrNotNull(res);
	//conv
	return res;
}
</xsl:template>
   	

<!-- documentation -->
<!--enumeration decls   H, finds also repetitions and prefixed them with the typename-->
<xsl:template match="xs:enumeration" mode="pod">
<xsl:variable  name="value" select="@value"/>	
<xsl:variable  name="counted" select="count(/wsdl:definitions/wsdl:types/xs:schema/xs:simpleType/xs:restriction/xs:enumeration[./@value=$value])"/>
<xsl:variable name="pref" select="if ($counted &gt; 1) then concat(./parent::node()/parent::node()/@name,'_') else ''"/>, <xsl:value-of select="$srmPre"/><xsl:value-of select="concat($pref,$value)"/> =<xsl:value-of select="position()-1"/>	
<xsl:if test="not(position()=last())"> </xsl:if>   
</xsl:template>    

    <!--structures fields decls  ifce header  -->
<xsl:template match="xs:element" mode="pod">
     <xsl:variable name="type" select="substring-after(@type, &quot;:&quot;)"/>
    <xsl:variable name ="defined" select="$defines/*[@name=$type]"/>    
    <xsl:choose>    
     <xsl:when test="(string(key('simples', $type))=$type)">
         <xsl:text>    </xsl:text>
    <xsl:value-of select="concat($srmPre,string(key('simples', $type))) "/><xsl:text> </xsl:text>    
    </xsl:when>
    <xsl:when test="(string-length($defined) gt 0) ">
        <xsl:text>    </xsl:text>
    <xsl:value-of select=" string($defined) "/> <xsl:text> </xsl:text>  
    </xsl:when>        
    <xsl:otherwise> 
        <xsl:text>    </xsl:text>
    <xsl:value-of select="concat($srmPre,string($type)) "/>*  <xsl:text> </xsl:text>   </xsl:otherwise>
    </xsl:choose>
    <xsl:value-of select=" @name"/>;
</xsl:template>
    
<!--structures fields decls  ifce body -->
<xsl:template name="body">
    <xsl:param name="p"/>
    <xsl:param name="delim"/>
    <xsl:for-each select="$p">
    <xsl:variable name="type" select="substring-after(@type, &quot;:&quot;)"/>
    <xsl:variable name ="defined" select="$defines/*[@name=$type]"/>    
    <xsl:choose>    
        <xsl:when test="(string(key('simples', $type))=$type)">
            <xsl:text>    </xsl:text>
            <xsl:value-of select="concat($srmPre,string(key('simples', $type))) "/><xsl:text> </xsl:text>    
        </xsl:when>
        <xsl:when test="(string-length($defined) gt 0) ">
            <xsl:text>    </xsl:text>
            <xsl:value-of select=" string($defined) "/> <xsl:text> </xsl:text>  
        </xsl:when>        
        <xsl:otherwise> 
            <xsl:text>    </xsl:text>
            <xsl:value-of select="concat($srmPre,string($type)) "/>*  <xsl:text> </xsl:text>   </xsl:otherwise>
    </xsl:choose>
    <xsl:value-of select=" @name"/>
        <xsl:if test="not(position()=last())"><xsl:value-of select="concat($delim,'&#xA;')"/></xsl:if>   
    </xsl:for-each>
</xsl:template>
    
    <!--structures fields decls  ifce body -->
<xsl:template match="xs:element" mode="body-free">
    <xsl:variable name="type" select="substring-after(@type, &quot;:&quot;)"/>
    <xsl:variable name ="definedComplex" select="key('complex',$type)"/>
    <xsl:variable name ="definedPtr" select="$definesPtr/*[@name=$type]"/>
	<xsl:variable name="complexConvTemp" select="$defines2gfalComplexConv/*[@type=$type and @arr='yes']"/>	
<xsl:choose>
<xsl:when test="(string-length($definedPtr) gt 0) ">	FREE_SUB_OBJ(<xsl:value-of select=" @name"/>);
</xsl:when>
<xsl:when test="exists($definedComplex) and exists($complexConvTemp)">	FREE_TYPE(<xsl:value-of select="concat($complexConvTemp/@totype,',',@name)"/>);
</xsl:when>	
<xsl:when test="exists($definedComplex)">	FREE_TYPE(<xsl:value-of select="string($type)"/>,<xsl:value-of select=" @name"/>);
</xsl:when>
</xsl:choose>        
</xsl:template>
	
<!--structures fields decls  ifce body ctor body -->
<xsl:template match="xs:element" mode="body_ctor_body">	COPY_CHK(<xsl:value-of select=" @name"/>);
</xsl:template>

<!--    
Choose alloc and conversion method depending on the type:
1.soap_ptr_alloc for all but complex objs
2. CONVERSION:
a. STRING_TO_SOAP
b. NUM_TO_SOAP
c. conv_type,enums

i.e.
    NUM_FROM_SOAP(remainingPinTime);
    CONV2GFAL_OBJ(TReturnStatus,status);
    STRING_FROM_SOAP_EMB(transferURL);
    ENUM_FROM_SOAP(fileStorageType);
-->
    
<xsl:template match="xs:element" mode="conv_in_body">
    <xsl:variable name="type" select="substring-after(@type, &quot;:&quot;)"/>
        <xsl:variable name ="definedComplex" select="key('complex',$type)"/>
    <xsl:variable name ="definedPtr" select="$definesPtr/*[@name=$type]"/>
    <xsl:variable name ="convTok" select="$conversionMethods/*[@name=$type]"/>
    <xsl:variable name="complexConvTemp" select="$defines2gfalComplexConv/*[@type=$type and @arr='yes']"/>
	<xsl:variable name="nillable" select="if (@nillable='true' or ((not(exists(@nillable))) and exists($convTok/@nonillok) and ($convTok/@nonillok eq '0'))) then  boolean(1)  else boolean(0)"/>    	
<xsl:choose>
<xsl:when test="not(exists($definedComplex) or exists(key('simples',$type))) and $nillable">	SOAP_PTR_ALLOC(<xsl:value-of select=" @name"/>);
    </xsl:when>
</xsl:choose>
    <xsl:choose>
<xsl:when test="contains($convTok,'STRING')">	STRING_TO_SOAP_EMB(<xsl:value-of select=" @name"/>);
        </xsl:when>
<xsl:when test="exists($definedComplex) and exists($complexConvTemp)">		res-&gt;<xsl:value-of select=" @name"/> = conv2soap_<xsl:value-of select="concat($srmPreSoap,$complexConvTemp/@type,'_2_',$complexConvTemp/@totype)"/>(soap,_elem-&gt;<xsl:value-of select=" @name"/>);
</xsl:when>
<xsl:when test="exists($definedComplex)">	res-&gt;<xsl:value-of select=" @name"/>=conv2soap_<xsl:value-of select="$srmPre"/><xsl:value-of select="string($type)"/>(soap,(_elem-&gt;<xsl:value-of select=" @name"/>));
</xsl:when>    	
<xsl:when test="(string(key('simples', $type))=$type) and $nillable">	res-><xsl:value-of select=" @name"/>=convEnum2soap_<xsl:value-of select="concat($srmPre,$type)"/>(soap,_elem-><xsl:value-of select=" @name"/>);	
</xsl:when>
<xsl:when test="(string(key('simples', $type))=$type) and not($nillable)">
	res-><xsl:value-of select=" @name"/> = *((enum <xsl:value-of select="concat($srmPre,$type)"/>*)(convEnum2soap_<xsl:value-of select="concat($srmPre,$type)"/>(soap,_elem-><xsl:value-of select=" @name"/>)));
</xsl:when>
<xsl:when test="contains($convTok,'NUM') and ($nillable )">	NUM_TO_SOAP_PTR(<xsl:value-of select="@name"/>);                
</xsl:when>
<!--<xsl:when test="contains($convTok,'NUM') and ($nillable or 	(not(exists(@nillable))) and not($convTok/nonillok))">	NUM_TO_SOAP(<xsl:value-of select="@name"/>);                
</xsl:when>
-->    	
<xsl:when test="contains($convTok,'NUM') and (not($nillable) )" >	NUM_TO_SOAP(<xsl:value-of select=" @name"/>);
</xsl:when>
    	
    </xsl:choose>
</xsl:template>

<!--    CONVERT OUT BODY - element-->
<xsl:template match="xs:element" mode="conv_out_body">
    <xsl:variable name="type" select="substring-after(@type, &quot;:&quot;)"/>
    <xsl:variable name ="definedComplex" select="key('complex',$type)"/>
    <xsl:variable name ="definedPtr" select="$definesPtr/*[@name=$type]"/>
    <xsl:variable name ="convTok" select="$conversionMethods/*[@name=$type]"/>
    <xsl:variable name="nillable" select="@nillable='true'"/>
<!--<xsl:choose>
    <xsl:when test="(string-length($definedPtr) gt 0) ">    SOAP_PTR_ALLOC(<xsl:value-of select=" @name"/>);
    </xsl:when>
</xsl:choose>
-->    
<xsl:choose>
        <xsl:when test="contains($convTok,'STRING')">	STRING_FROM_SOAP_EMB(<xsl:value-of select=" @name"/>);
        </xsl:when>
<xsl:when test="contains($convTok,'NUM') and $nillable">	NUM_FROM_SOAP_PTR(<xsl:value-of select=" @name"/>);
</xsl:when>
<xsl:when test="contains($convTok,'NUM') and not($nillable)">	NUM_FROM_SOAP(<xsl:value-of select=" @name"/>);
</xsl:when>    
<xsl:when test="exists($definedComplex)">	CONV2GFAL_OBJ(<xsl:value-of select="string($type)"/>,<xsl:value-of select=" @name"/>);
    </xsl:when>
<xsl:when test="(string(key('simples', $type))=$type) and $nillable">	ENUM_FROM_SOAP(<xsl:value-of select=" @name"/>);  
</xsl:when>    
<xsl:when test="(string(key('simples', $type))=$type) and not($nillable)">	ENUM_FROM_SOAP_NOPTR(<xsl:value-of select=" @name"/>);  
</xsl:when>    
<xsl:when test="exists($definedComplex) and exists($defines2gfalComplexConv/*[@type=$type])">
	<xsl:variable name="complexConvTemp" select="$defines2gfalComplexConv/*[@type=$type and @arr='yes']"/>	res-&gt;<xsl:value-of select=" @name"/> = conv2gfal_<xsl:value-of select="concat($srmPreSoap,$complexConvTemp/@type,'_2_',$complexConvTemp/@totype)"/>(_elem-&gt;<xsl:value-of select=" @name"/>);
</xsl:when>
    </xsl:choose>
</xsl:template>
    
<!--string mapping    -->
<!--<xsl:template match="xs:element[@type=&quot;xsd:string&quot;]" mode="pod">
    char* <xsl:value-of select="@name"/>;
</xsl:template>
<xsl:template match="xs:element[@type=&quot;xsd:anyURI&quot;]" mode="pod">
    char* <xsl:value-of select="@name"/>;
</xsl:template>
--><!-- date mapping -->
<!--    <xsl:template match="xs:element[@type=&quot;dateTime&quot;]" mode="pod">
        char* <xsl:value-of select="@name"/>;
    </xsl:template>
-->   
    <!--    int mapping-->
<!--<xsl:template match="xs:element[@type=&quot;xsd:int&quot;]" mode="pod">
        int <xsl:value-of select="@name"/>;
</xsl:template>
-->
 
<!-- simple type -->

<!-- =================================================================================== -->
<!-- methods H--> 

<xsl:template match="wsdl:operation" mode="serializer">
	<xsl:variable name="input" select="wsdl:input/@name"/>	
        <xsl:variable name="output" select="wsdl:output/@name"/>
<!--<xsl:apply-templates select="//wsdl:message[@name=$output]" mode="podo"/> <xsl:value-of select="@name"/>(<xsl:apply-templates select="//wsdl:message[@name=$input]" mode="podi"/>)&gt;-->
<!--	input surl in hierarchy	-->
<xsl:variable name="inputSurl"  select="gsoap:productSurl($input)"/>
<!--    <xsl:variable name="complexConvTemp" select="$defines2gfalComplexConv/*[@type=$type and @arr='yes']"/>    -->
<xsl:choose>
<xsl:when test="$inputSurl">
/* path to surl is: <xsl:value-of select="$inputSurl"/> */     
GFAL_WRAP_DEC_SURL(<xsl:value-of select="@name"/>);        
</xsl:when>
<xsl:otherwise>
GFAL_WRAP_DEC(<xsl:value-of select="@name"/>);        
</xsl:otherwise>
</xsl:choose>

</xsl:template>

    <!--IFCE BODY-->
<xsl:template match="wsdl:operation" mode="body">
        <xsl:variable name="input" select="wsdl:input/@name"/>
        <xsl:variable name="output" select="wsdl:output/@name"/>
        <!--<xsl:apply-templates select="//wsdl:message[@name=$output]" mode="podo"/> <xsl:value-of select="@name"/>(<xsl:apply-templates select="//wsdl:message[@name=$input]" mode="podi"/>)&gt;-->
    <xsl:call-template name="gfalwrapdoc">
        <xsl:with-param name="typename" select="@name"/>
        <xsl:with-param name="lsrmPre" select="$srmPre"/>
        <xsl:with-param name="isSurl"  select="contains(gsoap:productSurl($input),'!')"/>
    </xsl:call-template>
</xsl:template>
    
    
<xsl:template match="wsdl:portType">
    <xsl:apply-templates select="wsdl:operation" mode="method"/>
</xsl:template>

<xsl:template match="wsdl:message" mode="method">
    <xsl:apply-templates select="wsdl:part" mode="method"/>
</xsl:template>

<xsl:function name="gsoap:getSignatureType" >
    <xsl:param name="el" as="element(xs:element)" />        
    
    <xsl:variable name ="type" select="substring-after($el/@type,':')"/>
    <xsl:variable name ="name" select="$el/@name"/>
    <xsl:variable name ="defined" select="$defines/*[@name=$type]"/>
    
    <xsl:choose>    
            <xsl:when test="(string(key('simples', $type,$el))=$type)"> 
<!--                <xsl:value-of select=" "/>-->
                <xsl:sequence select="concat($type, string(' '), $name)"/>; 
            </xsl:when>
            <xsl:when test="(string-length($defined) gt 0) "> 
<!--                    <xsl:value-of select="string($defined) "/> -->
                <xsl:value-of select="concat($type,string(' '),$defined)"/>; 
            </xsl:when>
            <xsl:otherwise> 
<!--                <xsl:param name ="ret" select="concat($type,'* ')"/>-->
                <xsl:value-of select="concat($type,string('* '), $name)"/>;              
           </xsl:otherwise>
    </xsl:choose>
</xsl:function>

    <xsl:function name="gsoap:checkType" >
        <xsl:param name="el" as="element(xs:element)" />        
        
        <xsl:variable name ="type" select="substring-after($el/@type,':')"/>
        <xsl:variable name ="name" select="$el/@name"/>
        <xsl:variable name ="defined" select="$defines/*[@name=$type]"/>        
        <xsl:choose>    
            <xsl:when test="(string-length($defined) gt 0) ">                
                <xsl:value-of select="1"/>              
            </xsl:when>
            <xsl:otherwise>                
                <xsl:value-of select="2"/>              
            </xsl:otherwise>
        </xsl:choose>
    </xsl:function>

<!--     checks if type is primitive/enum or complex-->
    <xsl:function name="gsoap:checkTypeSimple" >
        <xsl:param name="el" as="xs:string" />        
       
        <xsl:variable name ="itype" select="$el"/>
        <xsl:variable name ="defined" select="$defines/*[@name=$itype]"/>
        <xsl:variable name="tra" select="trace($el,'chkSimple')"/>
        <xsl:choose>    
            <xsl:when test="((string-length($defined) gt 0) or (string(key('simples', $el,$root)) eq $el))">                
                <xsl:value-of select="1"/>              
            </xsl:when>
            <xsl:otherwise>                
                <xsl:value-of select="2"/>              
            </xsl:otherwise>
        </xsl:choose>
    </xsl:function>
    
    <xsl:function name="gsoap:inputCheck" as="node()">
        <xsl:param name="list" as="node()"/>
       
        <xsl:for-each select="$list">
            <xsl:variable name="ret" select="gsoap:checkType(.)"/>
            <xsl:choose>
                <xsl:when test="$ret=1">
                    <xsl:sequence select="."/>  
                </xsl:when>
                <xsl:when test="$ret=2">
                    <xsl:sequence select="distinct-values(gsoap:inputCheck(.))"/>  
                </xsl:when>
            </xsl:choose>

        </xsl:for-each>

    </xsl:function>
    
   
<!--    This returns any attribute-->
  <xsl:function name="gsoap:difference">
        <xsl:param name="nodes1"  />
        <xsl:param name="nodes2"  />
       <xsl:variable name="n1" select="$nodes1/.."/>
      <xsl:variable name="n2" select="$nodes2/.."/>
        <xsl:value-of select="$n1[count(.|$n2) != count($n2)]" />
    </xsl:function>

    
<xsl:template name="createTree"> 
    <xsl:param name="fragment"/>
                 <xsl:apply-templates mode="createTree"
                    select=" $fragment" />
</xsl:template>
    
    
<xsl:function name="gsoap:product" as="xs:string*">
    <xsl:param name="types" as="xs:string*" />
    <xsl:choose>
        <xsl:when test="(exists($types))">
            <xsl:variable name="S" select="string('')"/>
            <xsl:sequence select="gsoap:productM($types, $S,'')"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:sequence select="string('')"/>
        </xsl:otherwise>
    </xsl:choose>
</xsl:function>


<xsl:function name="gsoap:productM" as="xs:string*">
    <xsl:param name="types" as="xs:string*" />
    <xsl:param name="product" as="xs:string*"/>
    <xsl:param name="visited" as="xs:string*"/>
    
    <xsl:choose>
        <xsl:when test="(exists($types))">
            <xsl:variable name="S" select="string('ffff')"/>                
            <xsl:sequence select="gsoap:productM($types[position() > 1 and .!=$product],
                gsoap:checker($types[1],$product,concat('=',$types[1],'=',$visited)),$visited)"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:sequence select="$product "/>
        </xsl:otherwise>
    </xsl:choose>        
</xsl:function>

<xsl:function name="gsoap:checker" as="xs:string*">
    <xsl:param name="itype" as="xs:string" />
    <xsl:param name="product" as="xs:string*" />
    <xsl:param name="visited" as="xs:string*" />        
    <xsl:choose>            
        <xsl:when test="gsoap:checkTypeSimple($itype)=2 ">
            <xsl:variable name="zx" select="$root//xs:complexType[@name=$itype]/xs:sequence/xs:element/substring-after(@type,':')"/>
            <xsl:variable name="nexist" select="not(exists($product[.=$zx]))"/>
        
            <xsl:variable name="vistmp" select="concat($visited,':',$itype)"/>
            
            <xsl:choose>
<!--                    <xsl:when test="$nexist">-->
                <xsl:when test="not(contains(substring-after($visited,'='),concat('=',$itype,'=')))">                        
                    <xsl:variable name="extprod" select="$itype, $product"/>                        
                    <xsl:variable name="complexRecurs" select="gsoap:productM($zx,$extprod,$visited)"/>
                    <xsl:variable name="out" select="$complexRecurs , $itype, $extprod"></xsl:variable>
                    <xsl:sequence select="distinct-values($out)"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:variable name="extprod" select="concat('struct ',$itype), $product"/>  
                     <xsl:variable name="out" select="$extprod"></xsl:variable>
                    <xsl:sequence select="distinct-values($out)"/>
                </xsl:otherwise>
            </xsl:choose>                
        </xsl:when>
        <xsl:otherwise>
            <xsl:sequence select="$product"/>                
          </xsl:otherwise>            
    </xsl:choose>
</xsl:function>

	
	
    
  <xsl:function name="gsoap:getTypeFlow" as ="xs:integer">
      <xsl:param name="itype" as="xs:string" />
    <xsl:choose>
        <xsl:when test="exists($commonTypes[.=$itype])">
            <xsl:value-of>1</xsl:value-of>
        </xsl:when>
        <xsl:when test="exists($realinputT[.=$itype])">
            <xsl:value-of>2</xsl:value-of>
        </xsl:when>        
        <xsl:when test="exists($realoutputT[.=$itype])">
            <xsl:value-of>3</xsl:value-of>
        </xsl:when>       
        <xsl:when test="exists($inputHead[.=$itype])">
            <xsl:value-of>4</xsl:value-of>
        </xsl:when>       
        <xsl:when test="exists($outputHead[.=$itype])">
            <xsl:value-of>5</xsl:value-of>
        </xsl:when>
        <xsl:when test="exists($defines2gfalComplexConv/*[@arr='yes' and @type=$itype])">
            <xsl:value-of>6</xsl:value-of>
        </xsl:when>
    	
        <xsl:otherwise>0</xsl:otherwise>       
    </xsl:choose>      
  </xsl:function>
    
    <xsl:function name="gsoap:getTypeFlowText" as ="xs:string">
        <xsl:param name="itype" as="xs:string" />
        <xsl:choose>
            <xsl:when test="exists($commonTypes[.=$itype])">
                <xsl:value-of>input/output</xsl:value-of>
            </xsl:when>
            <xsl:when test="exists($realinputT[.=$itype])">
                <xsl:value-of>deep-input</xsl:value-of>
            </xsl:when>        
            <xsl:when test="exists($realoutputT[.=$itype])">
                <xsl:value-of>deep-output</xsl:value-of>
            </xsl:when>       
            <xsl:when test="exists($inputHead[.=$itype])">
                <xsl:value-of>main-input</xsl:value-of>
            </xsl:when>       
            <xsl:when test="exists($outputHead[.=$itype])">
                <xsl:value-of>main-output</xsl:value-of>
            </xsl:when>
        <xsl:when test="exists($defines2gfalComplexConv/*[@arr='yes' and @type=$itype])">
            <xsl:value-of>complex-conversion</xsl:value-of>
        </xsl:when>        	
            <xsl:otherwise>unknown</xsl:otherwise>       
        </xsl:choose>
        
    </xsl:function>
  
<xsl:template name="HEADER_IFCE">
    <!--     HEADER IFCE    -->
    <xsl:result-document href="{$headerNameIfce}">
        
        <!--<xsl:apply-templates select="$definesDoc/defines/simpleType"/>-->        
        <xsl:variable name="service" select="/wsdl:definitions/wsdl:portType/@name"/>
        
        <xsl:call-template name="ifce_h_header">
            <xsl:with-param name="lsrmPre" select="$srmPre"/>
            <xsl:with-param name="lsrmVer" select="$srmVer"/>            
        </xsl:call-template>
        
        /*       
        All input types:
        <xsl:value-of select="$inputHead, $inputT"/>
        All output types:
        <xsl:value-of select="$outputHead, $outputT"/>
        Types overlapping:
        <xsl:value-of select="$commonTypes" />
        Input types wo common:
        <xsl:value-of select="$realinputT" />
        Output types wo common:
        <xsl:value-of select="$realoutputT" />
        */
        
        <!--    <xsl:apply-templates select="/wsdl:definitions/wsdl:types/xs:schema"/>-->
        <!--    <xsl:apply-templates select="/wsdl:definitions/wsdl:portType"/>-->
        
        <!--    enumerations -->
        <xsl:apply-templates mode="serializer"
            select="/wsdl:definitions/wsdl:types/xs:schema/xs:simpleType"/>               
        <xsl:apply-templates mode="serializer"
            select="/wsdl:definitions/wsdl:types/xs:schema/xs:complexType"/>
        <xsl:text>/********** methods **********/</xsl:text>        
        <xsl:apply-templates mode="serializer"
            select="/wsdl:definitions/wsdl:portType/wsdl:operation"/>
        
#endif /*<xsl:value-of select="$srmVer"/>_IFCE_H_ */

</xsl:result-document> 
</xsl:template>

<xsl:template name="BODY_IFCE">
    <!--    BODY IFCE -->
    <xsl:result-document href="{$bodyNameIfce}">
        
        <!--<xsl:apply-templates select="$definesDoc/defines/simpleType"/>-->        
        <xsl:variable name="service" select="/wsdl:definitions/wsdl:portType/@name"/>
        
        <xsl:call-template name="ifce_c_header">
            <xsl:with-param name="lsrmPre" select="$srmPre"/>
            <xsl:with-param name="lsrmVer" select="$srmVer"/>            
        </xsl:call-template>
        
        /*       
        All input types:
        <xsl:value-of select="$inputHead, $inputT"/>
        All output types:
        <xsl:value-of select="$outputHead, $outputT"/>
        Types overlapping:
        <xsl:value-of select="$commonTypes" />
        Input types wo common:
        <xsl:value-of select="$realinputT" />
        Output types wo common:
        <xsl:value-of select="$realoutputT" />
        */
        
        <!--    enumerations -->
<!--<xsl:apply-templates mode="body"
            select="/wsdl:definitions/wsdl:types/xs:schema/xs:simpleType"/>               
-->
<xsl:apply-templates mode="body"
            select="/wsdl:definitions/wsdl:types/xs:schema/xs:complexType"/>
        <xsl:text>/********** methods **********/</xsl:text>        
<xsl:apply-templates mode="body"
            select="/wsdl:definitions/wsdl:portType/wsdl:operation"/>
    </xsl:result-document> 
</xsl:template>


<xsl:template name="HEADER_CONV">
    <!--     HEADER IFCE    -->
    <xsl:result-document href="{$headerNameConv}">
<!--        <xsl:apply-templates select="$definesDoc/defines/simpleType"/>-->
        <xsl:variable name="service" select="/wsdl:definitions/wsdl:portType/@name"/>
        
<xsl:call-template name="conv_h_header">
    <xsl:with-param name="lsrmPre" select="$srmPre"/>
    <xsl:with-param name="lsrmVer" select="$srmVer"/>            
</xsl:call-template>
        
/*       
All input types:
    <xsl:value-of select="$inputHead, $inputT"/>
    All output types:
    <xsl:value-of select="$outputHead, $outputT"/>
    Types overlapping:
    <xsl:value-of select="$commonTypes" />
    Input types wo common:
    <xsl:value-of select="$realinputT" />
    Output types wo common:
    <xsl:value-of select="$realoutputT" />
*/
        
        <!--    <xsl:apply-templates select="/wsdl:definitions/wsdl:types/xs:schema"/>-->
        <!--    <xsl:apply-templates select="/wsdl:definitions/wsdl:portType"/>-->
        
        <!--    enumerations -->
<!--        <xsl:apply-templates mode="conversion"
            select="/wsdl:definitions/wsdl:types/xs:schema/xs:simpleType"/>               
-->        <xsl:apply-templates mode="conversion"
            select="/wsdl:definitions/wsdl:types/xs:schema/xs:complexType"/>
<!--        <xsl:text>/********** methods **********/</xsl:text>        
        <xsl:apply-templates mode="conversion"
            select="/wsdl:definitions/wsdl:portType/wsdl:operation"/>
-->        
#endif /*<xsl:value-of select="$srmVer"/>_SOAP_CONVERSION_H_ */

</xsl:result-document> 
</xsl:template>        

<!--    BODY CONVERSION -->
<xsl:template name="BODY_CONV">    
        <xsl:result-document href="{$bodyNameConv}">
            
            <!--<xsl:apply-templates select="$definesDoc/defines/simpleType"/>-->        
            <xsl:variable name="service" select="/wsdl:definitions/wsdl:portType/@name"/>
            
            <xsl:call-template name="conv_c_header">
                <xsl:with-param name="lsrmPre" select="$srmPre"/>
                <xsl:with-param name="lsrmVer" select="$srmVer"/>            
            </xsl:call-template>
            
/*       
    All input types:
    <xsl:value-of select="$inputHead, $inputT"/>
    All output types:
    <xsl:value-of select="$outputHead, $outputT"/>
    Types overlapping:
    <xsl:value-of select="$commonTypes" />
    Input types wo common:
    <xsl:value-of select="$realinputT" />
    Output types wo common:
    <xsl:value-of select="$realoutputT" />
*/
            
            <!--    enumerations -->
<xsl:apply-templates mode="conversion-body" select="/wsdl:definitions/wsdl:types/xs:schema/xs:simpleType"/>
            
<xsl:apply-templates mode="conversion-body"                select="/wsdl:definitions/wsdl:types/xs:schema/xs:complexType"/>
</xsl:result-document> 
</xsl:template>        
  
<xsl:template name="gfalFreeTypeBody" >
        <xsl:param name="node"></xsl:param>
        <xsl:param name="lsrmPre"></xsl:param>    
        <xsl:variable name="typename" select="@name"/>
GFAL_FREE_TYPE_DEC(TSURLLifetimeReturnStatus)
{
if(!_arg)return; 
FREE_SUB_OBJ(surl);
FREE_TYPE(TReturnStatus,status);

END_GFAL_FREE;
}
</xsl:template>
    
	
 <!--surl finder function
	traverses all the subnodes of the root input type until it finds surl
	and returns access path to it
	otherwise returns nothing for the root input type
 	does conversion if the type defined is to be converted
-->	
  
<xsl:function name="gsoap:productSurl" as="xs:string*">
    <xsl:param name="types" as="xs:string" />
    <xsl:choose>
        <xsl:when test="(exists($types))">
            <xsl:variable name="S" select="string('')"/>
<!--        	get all descendants of this type which is input (diff then complex) type-->
            <xsl:variable name="ret" select="gsoap:productMSurl($root//xs:complexType[@name=$types]//xs:sequence/xs:element, $S,concat('=',$types,'='))"/>
            <xsl:choose>
                <xsl:when test="exists($ret[contains(.,'!')])">
                    <xsl:sequence select="$ret"/>        
                </xsl:when>
                <xsl:otherwise>
                    <xsl:sequence select="string('')"/>        
                </xsl:otherwise>
            </xsl:choose>          
            
        </xsl:when>
        <xsl:otherwise>
            <xsl:sequence select="string('')"/>
        </xsl:otherwise>
    </xsl:choose>
</xsl:function>


<xsl:function name="gsoap:productMSurl" as="xs:string*">
<xsl:param name="types" as="node()*" />
<xsl:param name="product" as="xs:string"/>
<xsl:param name="visited" as="xs:string*"/>

    <xsl:choose>
        <xsl:when test="(exists($types) and not(exists($product[contains(.,'!')])))">
            <xsl:variable name="S" select="string('ffff')"/>        	
        	<xsl:sequence select="gsoap:productMSurl($types[position() > 1],
                gsoap:checkerSurl($types[1],$product,concat('=',substring-after($types[1]/@type,':'),'=',$visited)),$visited)"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:sequence select="$product"/>
        </xsl:otherwise>
    </xsl:choose>        
</xsl:function>

<xsl:function name="gsoap:checkerSurl" as="xs:string*">
<xsl:param name="node" as="node()"/>
<xsl:param name="product" as="xs:string" />
<xsl:param name="visited" as="xs:string*" />

	
<xsl:variable name="isSurlComplex" select="$node/@name[matches(.,'[\w]*[sS][uU][rR][lL][\w]*')][1]"/>
<xsl:variable name="itype" select="substring-after($node/@type,':')"/>
<xsl:variable name="iname" select="$node/@name"/>
	
<xsl:choose>
        	<xsl:when test="gsoap:checkTypeSimple($itype)=2">
<!--            <xsl:variable name="zx" select="$root//xs:complexType[@name=$itype]/xs:sequence/xs:element/substring-after(@type,':')"/>-->
        	
	<xsl:variable name="zx" select="$root//xs:complexType[@name=$node/substring-after(@type,':')]/xs:sequence/xs:element"/>        	
	<xsl:variable name="nexist" select="not(exists($product[.=$zx]))"/>        
	<xsl:variable name="vistmp" select="concat($visited,':',$itype)"/>
	
<!--	<xsl:if test="exists($isSurlComplex)">
		<xsl:sequence select="concat($visited,'=',$isSurlComplex)"/>                	
	</xsl:if>
-->            <xsl:choose>
                <xsl:when test="not(contains(substring-after($visited,'='),concat('=',$itype,'=')))">                        
                    <xsl:variable name="extprod" select="concat($iname,':',$itype,'->' ,$product)"/>                        
                    <xsl:sequence  select="gsoap:productMSurl($zx,$extprod,$visited)"/>                    
                </xsl:when>
	 <!--	cycle detected		-->
                <xsl:otherwise>
<!--                    <xsl:variable name="extprod" select="$itype, $visited"/>  
                     <xsl:variable name="out" select="$extprod"></xsl:variable>
	/*cycle detected*/
-->                    <xsl:sequence select="concat($iname,':',$itype,'->', $visited)"/>
                </xsl:otherwise>
            </xsl:choose>                
        </xsl:when>
<!--    	simple type-->
        <xsl:otherwise>
	<xsl:variable name="isSurl" select="$node/@name[matches(.,'[\w]*[sS][uU][rR][lL][\w]*')][1]|$node/@name[matches(.,'[\w]*[sS][uU][rR][lL][\w]*')]"/>
            <xsl:choose>
                <xsl:when test="$isSurl">
	<!-- do conversion here if applicable, not very generic - conversion should be done on input to recursive call probably-->
            	<xsl:variable name="complexConvTemp" select="$defines2gfalComplexConv/*[@type=$itype]"/>
		<xsl:variable name="otype" select="if (exists($complexConvTemp)) then ($complexConvTemp/@totype) else ($itype)"/>
		<xsl:variable name="oname" select="if (exists($complexConvTemp)) then ($complexConvTemp/arg/@toname) else ($isSurl)"/>

    	    <xsl:sequence select="concat($isSurl,':',$otype,'!',$product)"/> 
    	</xsl:when>
	<xsl:when test="not(exists($isSurl))">
                    <xsl:sequence select="$product"/> 
       </xsl:when>
            </xsl:choose>
          </xsl:otherwise>            
</xsl:choose>    
</xsl:function>
	
</xsl:stylesheet> 