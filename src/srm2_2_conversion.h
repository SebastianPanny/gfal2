    

#ifndef srm22_SOAP_CONVERSION_H_
#define srm22_SOAP_CONVERSION_H_

#include "srm2_2Stub.h"
#include "srm2_2_ifce.h"

/*
* Conversion methods for gsoap to gfal types
* Types are defined in gfal_api.h.
* For arrays we always use compound type with ptr and sizeArr.
* 
* >>Convention>>:
* Embedded structures if hold pointers, such as arrays wrappers 
* are hold as pointers for unification of freeing.
*/


#define DEF_GFALCONV_HEADERS_IN(_name)	\
struct srm22__##_name * conv2soap_srm22_##_name(struct soap *soap, const srm22_##_name * _elem)

	
#define DEF_GFALCONV_HEADERS_OUT(_name)	\
srm22_##_name * conv2gfal_srm22_##_name(const struct srm22__##_name * _elem)


#define DEF_GFALCONV_HEADERS(_name)	\
DEF_GFALCONV_HEADERS_IN(_name); \
DEF_GFALCONV_HEADERS_OUT(_name);


#define DEF_GFALCONVENUM_DEC(_name)	\
enum srm22__ ##_name* convEnum2soap_srm22_##_name(struct soap* soap, int _res);

//in
ULONG64 conv2soap_srm22_unsignedLong(struct soap *soap, ULONG64 _elem);
char*  conv2soap_srm22_String(struct soap *soap, char* _elem);
//out
ULONG64 conv2gfal_srm22_unsignedLong(ULONG64* _elem);
char* conv2gfal_srm22_String(char* _elem);	

        
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
    input/output type.
*/

struct srm22__TRetentionPolicyInfo * conv2soap_srm22_TRetentionPolicyInfo(struct soap *soap, const srm22_TRetentionPolicyInfo * _elem);
srm22_TRetentionPolicyInfo * conv2gfal_srm22_TRetentionPolicyInfo(const struct srm22__TRetentionPolicyInfo * _elem);
            
/*
    TUserPermission    
    is the 
    input/output type.
*/

struct srm22__TUserPermission * conv2soap_srm22_TUserPermission(struct soap *soap, const srm22_TUserPermission * _elem);
srm22_TUserPermission * conv2gfal_srm22_TUserPermission(const struct srm22__TUserPermission * _elem);
            
/*
    ArrayOfTUserPermission    
    is the 
    input/output type.
*/

struct srm22__ArrayOfTUserPermission * conv2soap_srm22_ArrayOfTUserPermission(struct soap *soap, const srm22_ArrayOfTUserPermission * _elem);
srm22_ArrayOfTUserPermission * conv2gfal_srm22_ArrayOfTUserPermission(const struct srm22__ArrayOfTUserPermission * _elem);
            
/*
    TGroupPermission    
    is the 
    input/output type.
*/

struct srm22__TGroupPermission * conv2soap_srm22_TGroupPermission(struct soap *soap, const srm22_TGroupPermission * _elem);
srm22_TGroupPermission * conv2gfal_srm22_TGroupPermission(const struct srm22__TGroupPermission * _elem);
            
/*
    ArrayOfTGroupPermission    
    is the 
    input/output type.
*/

struct srm22__ArrayOfTGroupPermission * conv2soap_srm22_ArrayOfTGroupPermission(struct soap *soap, const srm22_ArrayOfTGroupPermission * _elem);
srm22_ArrayOfTGroupPermission * conv2gfal_srm22_ArrayOfTGroupPermission(const struct srm22__ArrayOfTGroupPermission * _elem);
            
/*
    ArrayOfUnsignedLong    
    is the 
    deep-input type.
*/

struct srm22__ArrayOfUnsignedLong * conv2soap_srm22_ArrayOfUnsignedLong(struct soap *soap, const srm22_ArrayOfUnsignedLong * _elem);
            
/*
    ArrayOfString    
    is the 
    input/output type.
*/

struct srm22__ArrayOfString * conv2soap_srm22_ArrayOfString(struct soap *soap, const srm22_ArrayOfString * _elem);
srm22_ArrayOfString * conv2gfal_srm22_ArrayOfString(const struct srm22__ArrayOfString * _elem);
            
/*
    TReturnStatus    
    is the 
    deep-output type.
*/
srm22_TReturnStatus * conv2gfal_srm22_TReturnStatus(const struct srm22__TReturnStatus * _elem);            	
            
/*
    TSURLReturnStatus    
    is the 
    deep-output type.
*/
srm22_TSURLReturnStatus * conv2gfal_srm22_TSURLReturnStatus(const struct srm22__TSURLReturnStatus * _elem);            	
            
/*
    ArrayOfTSURLReturnStatus    
    is the 
    deep-output type.
*/
srm22_ArrayOfTSURLReturnStatus * conv2gfal_srm22_ArrayOfTSURLReturnStatus(const struct srm22__ArrayOfTSURLReturnStatus * _elem);            	
            
/*
    TSURLLifetimeReturnStatus    
    is the 
    deep-output type.
*/
srm22_TSURLLifetimeReturnStatus * conv2gfal_srm22_TSURLLifetimeReturnStatus(const struct srm22__TSURLLifetimeReturnStatus * _elem);            	
            
/*
    ArrayOfTSURLLifetimeReturnStatus    
    is the 
    deep-output type.
*/
srm22_ArrayOfTSURLLifetimeReturnStatus * conv2gfal_srm22_ArrayOfTSURLLifetimeReturnStatus(const struct srm22__ArrayOfTSURLLifetimeReturnStatus * _elem);            	
            
/*
    TMetaDataPathDetail    
    is the 
    deep-output type.
*/
srm22_TMetaDataPathDetail * conv2gfal_srm22_TMetaDataPathDetail(const struct srm22__TMetaDataPathDetail * _elem);            	
            
/*
    ArrayOfTMetaDataPathDetail    
    is the 
    deep-output type.
*/
srm22_ArrayOfTMetaDataPathDetail * conv2gfal_srm22_ArrayOfTMetaDataPathDetail(const struct srm22__ArrayOfTMetaDataPathDetail * _elem);            	
            
/*
    TMetaDataSpace    
    is the 
    deep-output type.
*/
srm22_TMetaDataSpace * conv2gfal_srm22_TMetaDataSpace(const struct srm22__TMetaDataSpace * _elem);            	
            
/*
    ArrayOfTMetaDataSpace    
    is the 
    deep-output type.
*/
srm22_ArrayOfTMetaDataSpace * conv2gfal_srm22_ArrayOfTMetaDataSpace(const struct srm22__ArrayOfTMetaDataSpace * _elem);            	
            
/*
    TDirOption    
    is the 
    deep-input type.
*/

struct srm22__TDirOption * conv2soap_srm22_TDirOption(struct soap *soap, const srm22_TDirOption * _elem);
            
/*
    TExtraInfo    
    is the 
    input/output type.
*/

struct srm22__TExtraInfo * conv2soap_srm22_TExtraInfo(struct soap *soap, const srm22_TExtraInfo * _elem);
srm22_TExtraInfo * conv2gfal_srm22_TExtraInfo(const struct srm22__TExtraInfo * _elem);
            
/*
    ArrayOfTExtraInfo    
    is the 
    input/output type.
*/

struct srm22__ArrayOfTExtraInfo * conv2soap_srm22_ArrayOfTExtraInfo(struct soap *soap, const srm22_ArrayOfTExtraInfo * _elem);
srm22_ArrayOfTExtraInfo * conv2gfal_srm22_ArrayOfTExtraInfo(const struct srm22__ArrayOfTExtraInfo * _elem);
            
/*
    ArrayOfAnyURI    
    is the 
    complex-conversion type.
*/
srm22_ArrayOfString * conv2gfal_srm22__ArrayOfAnyURI_2_ArrayOfString(const struct srm22__ArrayOfAnyURI * _elem);
struct srm22__ArrayOfAnyURI * conv2soap_srm22__ArrayOfAnyURI_2_ArrayOfString(struct soap *soap, srm22_ArrayOfString * _elem);            	
            
/*
    TTransferParameters    
    is the 
    deep-input type.
*/

struct srm22__TTransferParameters * conv2soap_srm22_TTransferParameters(struct soap *soap, const srm22_TTransferParameters * _elem);
            
/*
    TGetFileRequest    
    is the 
    deep-input type.
*/

struct srm22__TGetFileRequest * conv2soap_srm22_TGetFileRequest(struct soap *soap, const srm22_TGetFileRequest * _elem);
            
/*
    ArrayOfTGetFileRequest    
    is the 
    deep-input type.
*/

struct srm22__ArrayOfTGetFileRequest * conv2soap_srm22_ArrayOfTGetFileRequest(struct soap *soap, const srm22_ArrayOfTGetFileRequest * _elem);
            
/*
    TPutFileRequest    
    is the 
    deep-input type.
*/

struct srm22__TPutFileRequest * conv2soap_srm22_TPutFileRequest(struct soap *soap, const srm22_TPutFileRequest * _elem);
            
/*
    ArrayOfTPutFileRequest    
    is the 
    deep-input type.
*/

struct srm22__ArrayOfTPutFileRequest * conv2soap_srm22_ArrayOfTPutFileRequest(struct soap *soap, const srm22_ArrayOfTPutFileRequest * _elem);
            
/*
    TCopyFileRequest    
    is the 
    deep-input type.
*/

struct srm22__TCopyFileRequest * conv2soap_srm22_TCopyFileRequest(struct soap *soap, const srm22_TCopyFileRequest * _elem);
            
/*
    ArrayOfTCopyFileRequest    
    is the 
    deep-input type.
*/

struct srm22__ArrayOfTCopyFileRequest * conv2soap_srm22_ArrayOfTCopyFileRequest(struct soap *soap, const srm22_ArrayOfTCopyFileRequest * _elem);
            
/*
    TGetRequestFileStatus    
    is the 
    deep-output type.
*/
srm22_TGetRequestFileStatus * conv2gfal_srm22_TGetRequestFileStatus(const struct srm22__TGetRequestFileStatus * _elem);            	
            
/*
    ArrayOfTGetRequestFileStatus    
    is the 
    deep-output type.
*/
srm22_ArrayOfTGetRequestFileStatus * conv2gfal_srm22_ArrayOfTGetRequestFileStatus(const struct srm22__ArrayOfTGetRequestFileStatus * _elem);            	
            
/*
    TBringOnlineRequestFileStatus    
    is the 
    deep-output type.
*/
srm22_TBringOnlineRequestFileStatus * conv2gfal_srm22_TBringOnlineRequestFileStatus(const struct srm22__TBringOnlineRequestFileStatus * _elem);            	
            
/*
    ArrayOfTBringOnlineRequestFileStatus    
    is the 
    deep-output type.
*/
srm22_ArrayOfTBringOnlineRequestFileStatus * conv2gfal_srm22_ArrayOfTBringOnlineRequestFileStatus(const struct srm22__ArrayOfTBringOnlineRequestFileStatus * _elem);            	
            
/*
    TPutRequestFileStatus    
    is the 
    deep-output type.
*/
srm22_TPutRequestFileStatus * conv2gfal_srm22_TPutRequestFileStatus(const struct srm22__TPutRequestFileStatus * _elem);            	
            
/*
    ArrayOfTPutRequestFileStatus    
    is the 
    deep-output type.
*/
srm22_ArrayOfTPutRequestFileStatus * conv2gfal_srm22_ArrayOfTPutRequestFileStatus(const struct srm22__ArrayOfTPutRequestFileStatus * _elem);            	
            
/*
    TCopyRequestFileStatus    
    is the 
    deep-output type.
*/
srm22_TCopyRequestFileStatus * conv2gfal_srm22_TCopyRequestFileStatus(const struct srm22__TCopyRequestFileStatus * _elem);            	
            
/*
    ArrayOfTCopyRequestFileStatus    
    is the 
    deep-output type.
*/
srm22_ArrayOfTCopyRequestFileStatus * conv2gfal_srm22_ArrayOfTCopyRequestFileStatus(const struct srm22__ArrayOfTCopyRequestFileStatus * _elem);            	
            
/*
    TRequestSummary    
    is the 
    deep-output type.
*/
srm22_TRequestSummary * conv2gfal_srm22_TRequestSummary(const struct srm22__TRequestSummary * _elem);            	
            
/*
    ArrayOfTRequestSummary    
    is the 
    deep-output type.
*/
srm22_ArrayOfTRequestSummary * conv2gfal_srm22_ArrayOfTRequestSummary(const struct srm22__ArrayOfTRequestSummary * _elem);            	
            
/*
    TSURLPermissionReturn    
    is the 
    deep-output type.
*/
srm22_TSURLPermissionReturn * conv2gfal_srm22_TSURLPermissionReturn(const struct srm22__TSURLPermissionReturn * _elem);            	
            
/*
    ArrayOfTSURLPermissionReturn    
    is the 
    deep-output type.
*/
srm22_ArrayOfTSURLPermissionReturn * conv2gfal_srm22_ArrayOfTSURLPermissionReturn(const struct srm22__ArrayOfTSURLPermissionReturn * _elem);            	
            
/*
    TPermissionReturn    
    is the 
    deep-output type.
*/
srm22_TPermissionReturn * conv2gfal_srm22_TPermissionReturn(const struct srm22__TPermissionReturn * _elem);            	
            
/*
    ArrayOfTPermissionReturn    
    is the 
    deep-output type.
*/
srm22_ArrayOfTPermissionReturn * conv2gfal_srm22_ArrayOfTPermissionReturn(const struct srm22__ArrayOfTPermissionReturn * _elem);            	
            
/*
    TRequestTokenReturn    
    is the 
    deep-output type.
*/
srm22_TRequestTokenReturn * conv2gfal_srm22_TRequestTokenReturn(const struct srm22__TRequestTokenReturn * _elem);            	
            
/*
    ArrayOfTRequestTokenReturn    
    is the 
    deep-output type.
*/
srm22_ArrayOfTRequestTokenReturn * conv2gfal_srm22_ArrayOfTRequestTokenReturn(const struct srm22__ArrayOfTRequestTokenReturn * _elem);            	
            
/*
    TSupportedTransferProtocol    
    is the 
    deep-output type.
*/
srm22_TSupportedTransferProtocol * conv2gfal_srm22_TSupportedTransferProtocol(const struct srm22__TSupportedTransferProtocol * _elem);            	
            
/*
    ArrayOfTSupportedTransferProtocol    
    is the 
    deep-output type.
*/
srm22_ArrayOfTSupportedTransferProtocol * conv2gfal_srm22_ArrayOfTSupportedTransferProtocol(const struct srm22__ArrayOfTSupportedTransferProtocol * _elem);            	
            
/*
    srmReserveSpaceRequest    
    is the 
    main-input type.
*/

struct srm22__srmReserveSpaceRequest * conv2soap_srm22_srmReserveSpaceRequest(struct soap *soap, const srm22_srmReserveSpaceRequest * _elem);
            
/*
    srmReserveSpaceResponse    
    is the 
    main-output type.
*/
srm22_srmReserveSpaceResponse * conv2gfal_srm22_srmReserveSpaceResponse(const struct srm22__srmReserveSpaceResponse * _elem);            	
            
/*
    srmStatusOfReserveSpaceRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmStatusOfReserveSpaceRequestRequest * conv2soap_srm22_srmStatusOfReserveSpaceRequestRequest(struct soap *soap, const srm22_srmStatusOfReserveSpaceRequestRequest * _elem);
            
/*
    srmStatusOfReserveSpaceRequestResponse    
    is the 
    main-output type.
*/
srm22_srmStatusOfReserveSpaceRequestResponse * conv2gfal_srm22_srmStatusOfReserveSpaceRequestResponse(const struct srm22__srmStatusOfReserveSpaceRequestResponse * _elem);            	
            
/*
    srmReleaseSpaceRequest    
    is the 
    main-input type.
*/

struct srm22__srmReleaseSpaceRequest * conv2soap_srm22_srmReleaseSpaceRequest(struct soap *soap, const srm22_srmReleaseSpaceRequest * _elem);
            
/*
    srmReleaseSpaceResponse    
    is the 
    main-output type.
*/
srm22_srmReleaseSpaceResponse * conv2gfal_srm22_srmReleaseSpaceResponse(const struct srm22__srmReleaseSpaceResponse * _elem);            	
            
/*
    srmUpdateSpaceRequest    
    is the 
    main-input type.
*/

struct srm22__srmUpdateSpaceRequest * conv2soap_srm22_srmUpdateSpaceRequest(struct soap *soap, const srm22_srmUpdateSpaceRequest * _elem);
            
/*
    srmUpdateSpaceResponse    
    is the 
    main-output type.
*/
srm22_srmUpdateSpaceResponse * conv2gfal_srm22_srmUpdateSpaceResponse(const struct srm22__srmUpdateSpaceResponse * _elem);            	
            
/*
    srmStatusOfUpdateSpaceRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmStatusOfUpdateSpaceRequestRequest * conv2soap_srm22_srmStatusOfUpdateSpaceRequestRequest(struct soap *soap, const srm22_srmStatusOfUpdateSpaceRequestRequest * _elem);
            
/*
    srmStatusOfUpdateSpaceRequestResponse    
    is the 
    main-output type.
*/
srm22_srmStatusOfUpdateSpaceRequestResponse * conv2gfal_srm22_srmStatusOfUpdateSpaceRequestResponse(const struct srm22__srmStatusOfUpdateSpaceRequestResponse * _elem);            	
            
/*
    srmGetSpaceMetaDataRequest    
    is the 
    main-input type.
*/

struct srm22__srmGetSpaceMetaDataRequest * conv2soap_srm22_srmGetSpaceMetaDataRequest(struct soap *soap, const srm22_srmGetSpaceMetaDataRequest * _elem);
            
/*
    srmGetSpaceMetaDataResponse    
    is the 
    main-output type.
*/
srm22_srmGetSpaceMetaDataResponse * conv2gfal_srm22_srmGetSpaceMetaDataResponse(const struct srm22__srmGetSpaceMetaDataResponse * _elem);            	
            
/*
    srmChangeSpaceForFilesRequest    
    is the 
    main-input type.
*/

struct srm22__srmChangeSpaceForFilesRequest * conv2soap_srm22_srmChangeSpaceForFilesRequest(struct soap *soap, const srm22_srmChangeSpaceForFilesRequest * _elem);
            
/*
    srmChangeSpaceForFilesResponse    
    is the 
    main-output type.
*/
srm22_srmChangeSpaceForFilesResponse * conv2gfal_srm22_srmChangeSpaceForFilesResponse(const struct srm22__srmChangeSpaceForFilesResponse * _elem);            	
            
/*
    srmStatusOfChangeSpaceForFilesRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmStatusOfChangeSpaceForFilesRequestRequest * conv2soap_srm22_srmStatusOfChangeSpaceForFilesRequestRequest(struct soap *soap, const srm22_srmStatusOfChangeSpaceForFilesRequestRequest * _elem);
            
/*
    srmStatusOfChangeSpaceForFilesRequestResponse    
    is the 
    main-output type.
*/
srm22_srmStatusOfChangeSpaceForFilesRequestResponse * conv2gfal_srm22_srmStatusOfChangeSpaceForFilesRequestResponse(const struct srm22__srmStatusOfChangeSpaceForFilesRequestResponse * _elem);            	
            
/*
    srmExtendFileLifeTimeInSpaceRequest    
    is the 
    main-input type.
*/

struct srm22__srmExtendFileLifeTimeInSpaceRequest * conv2soap_srm22_srmExtendFileLifeTimeInSpaceRequest(struct soap *soap, const srm22_srmExtendFileLifeTimeInSpaceRequest * _elem);
            
/*
    srmExtendFileLifeTimeInSpaceResponse    
    is the 
    main-output type.
*/
srm22_srmExtendFileLifeTimeInSpaceResponse * conv2gfal_srm22_srmExtendFileLifeTimeInSpaceResponse(const struct srm22__srmExtendFileLifeTimeInSpaceResponse * _elem);            	
            
/*
    srmPurgeFromSpaceRequest    
    is the 
    main-input type.
*/

struct srm22__srmPurgeFromSpaceRequest * conv2soap_srm22_srmPurgeFromSpaceRequest(struct soap *soap, const srm22_srmPurgeFromSpaceRequest * _elem);
            
/*
    srmPurgeFromSpaceResponse    
    is the 
    main-output type.
*/
srm22_srmPurgeFromSpaceResponse * conv2gfal_srm22_srmPurgeFromSpaceResponse(const struct srm22__srmPurgeFromSpaceResponse * _elem);            	
            
/*
    srmGetSpaceTokensRequest    
    is the 
    main-input type.
*/

struct srm22__srmGetSpaceTokensRequest * conv2soap_srm22_srmGetSpaceTokensRequest(struct soap *soap, const srm22_srmGetSpaceTokensRequest * _elem);
            
/*
    srmGetSpaceTokensResponse    
    is the 
    main-output type.
*/
srm22_srmGetSpaceTokensResponse * conv2gfal_srm22_srmGetSpaceTokensResponse(const struct srm22__srmGetSpaceTokensResponse * _elem);            	
            
/*
    srmSetPermissionRequest    
    is the 
    main-input type.
*/

struct srm22__srmSetPermissionRequest * conv2soap_srm22_srmSetPermissionRequest(struct soap *soap, const srm22_srmSetPermissionRequest * _elem);
            
/*
    srmSetPermissionResponse    
    is the 
    main-output type.
*/
srm22_srmSetPermissionResponse * conv2gfal_srm22_srmSetPermissionResponse(const struct srm22__srmSetPermissionResponse * _elem);            	
            
/*
    srmCheckPermissionRequest    
    is the 
    main-input type.
*/

struct srm22__srmCheckPermissionRequest * conv2soap_srm22_srmCheckPermissionRequest(struct soap *soap, const srm22_srmCheckPermissionRequest * _elem);
            
/*
    srmCheckPermissionResponse    
    is the 
    main-output type.
*/
srm22_srmCheckPermissionResponse * conv2gfal_srm22_srmCheckPermissionResponse(const struct srm22__srmCheckPermissionResponse * _elem);            	
            
/*
    srmGetPermissionRequest    
    is the 
    main-input type.
*/

struct srm22__srmGetPermissionRequest * conv2soap_srm22_srmGetPermissionRequest(struct soap *soap, const srm22_srmGetPermissionRequest * _elem);
            
/*
    srmGetPermissionResponse    
    is the 
    main-output type.
*/
srm22_srmGetPermissionResponse * conv2gfal_srm22_srmGetPermissionResponse(const struct srm22__srmGetPermissionResponse * _elem);            	
            
/*
    srmMkdirRequest    
    is the 
    main-input type.
*/

struct srm22__srmMkdirRequest * conv2soap_srm22_srmMkdirRequest(struct soap *soap, const srm22_srmMkdirRequest * _elem);
            
/*
    srmMkdirResponse    
    is the 
    main-output type.
*/
srm22_srmMkdirResponse * conv2gfal_srm22_srmMkdirResponse(const struct srm22__srmMkdirResponse * _elem);            	
            
/*
    srmRmdirRequest    
    is the 
    main-input type.
*/

struct srm22__srmRmdirRequest * conv2soap_srm22_srmRmdirRequest(struct soap *soap, const srm22_srmRmdirRequest * _elem);
            
/*
    srmRmdirResponse    
    is the 
    main-output type.
*/
srm22_srmRmdirResponse * conv2gfal_srm22_srmRmdirResponse(const struct srm22__srmRmdirResponse * _elem);            	
            
/*
    srmRmRequest    
    is the 
    main-input type.
*/

struct srm22__srmRmRequest * conv2soap_srm22_srmRmRequest(struct soap *soap, const srm22_srmRmRequest * _elem);
            
/*
    srmRmResponse    
    is the 
    main-output type.
*/
srm22_srmRmResponse * conv2gfal_srm22_srmRmResponse(const struct srm22__srmRmResponse * _elem);            	
            
/*
    srmLsRequest    
    is the 
    main-input type.
*/

struct srm22__srmLsRequest * conv2soap_srm22_srmLsRequest(struct soap *soap, const srm22_srmLsRequest * _elem);
            
/*
    srmLsResponse    
    is the 
    main-output type.
*/
srm22_srmLsResponse * conv2gfal_srm22_srmLsResponse(const struct srm22__srmLsResponse * _elem);            	
            
/*
    srmStatusOfLsRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmStatusOfLsRequestRequest * conv2soap_srm22_srmStatusOfLsRequestRequest(struct soap *soap, const srm22_srmStatusOfLsRequestRequest * _elem);
            
/*
    srmStatusOfLsRequestResponse    
    is the 
    main-output type.
*/
srm22_srmStatusOfLsRequestResponse * conv2gfal_srm22_srmStatusOfLsRequestResponse(const struct srm22__srmStatusOfLsRequestResponse * _elem);            	
            
/*
    srmMvRequest    
    is the 
    main-input type.
*/

struct srm22__srmMvRequest * conv2soap_srm22_srmMvRequest(struct soap *soap, const srm22_srmMvRequest * _elem);
            
/*
    srmMvResponse    
    is the 
    main-output type.
*/
srm22_srmMvResponse * conv2gfal_srm22_srmMvResponse(const struct srm22__srmMvResponse * _elem);            	
            
/*
    srmPrepareToGetRequest    
    is the 
    main-input type.
*/

struct srm22__srmPrepareToGetRequest * conv2soap_srm22_srmPrepareToGetRequest(struct soap *soap, const srm22_srmPrepareToGetRequest * _elem);
            
/*
    srmPrepareToGetResponse    
    is the 
    main-output type.
*/
srm22_srmPrepareToGetResponse * conv2gfal_srm22_srmPrepareToGetResponse(const struct srm22__srmPrepareToGetResponse * _elem);            	
            
/*
    srmStatusOfGetRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmStatusOfGetRequestRequest * conv2soap_srm22_srmStatusOfGetRequestRequest(struct soap *soap, const srm22_srmStatusOfGetRequestRequest * _elem);
            
/*
    srmStatusOfGetRequestResponse    
    is the 
    main-output type.
*/
srm22_srmStatusOfGetRequestResponse * conv2gfal_srm22_srmStatusOfGetRequestResponse(const struct srm22__srmStatusOfGetRequestResponse * _elem);            	
            
/*
    srmBringOnlineRequest    
    is the 
    main-input type.
*/

struct srm22__srmBringOnlineRequest * conv2soap_srm22_srmBringOnlineRequest(struct soap *soap, const srm22_srmBringOnlineRequest * _elem);
            
/*
    srmBringOnlineResponse    
    is the 
    main-output type.
*/
srm22_srmBringOnlineResponse * conv2gfal_srm22_srmBringOnlineResponse(const struct srm22__srmBringOnlineResponse * _elem);            	
            
/*
    srmStatusOfBringOnlineRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmStatusOfBringOnlineRequestRequest * conv2soap_srm22_srmStatusOfBringOnlineRequestRequest(struct soap *soap, const srm22_srmStatusOfBringOnlineRequestRequest * _elem);
            
/*
    srmStatusOfBringOnlineRequestResponse    
    is the 
    main-output type.
*/
srm22_srmStatusOfBringOnlineRequestResponse * conv2gfal_srm22_srmStatusOfBringOnlineRequestResponse(const struct srm22__srmStatusOfBringOnlineRequestResponse * _elem);            	
            
/*
    srmPrepareToPutRequest    
    is the 
    main-input type.
*/

struct srm22__srmPrepareToPutRequest * conv2soap_srm22_srmPrepareToPutRequest(struct soap *soap, const srm22_srmPrepareToPutRequest * _elem);
            
/*
    srmPrepareToPutResponse    
    is the 
    main-output type.
*/
srm22_srmPrepareToPutResponse * conv2gfal_srm22_srmPrepareToPutResponse(const struct srm22__srmPrepareToPutResponse * _elem);            	
            
/*
    srmStatusOfPutRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmStatusOfPutRequestRequest * conv2soap_srm22_srmStatusOfPutRequestRequest(struct soap *soap, const srm22_srmStatusOfPutRequestRequest * _elem);
            
/*
    srmStatusOfPutRequestResponse    
    is the 
    main-output type.
*/
srm22_srmStatusOfPutRequestResponse * conv2gfal_srm22_srmStatusOfPutRequestResponse(const struct srm22__srmStatusOfPutRequestResponse * _elem);            	
            
/*
    srmCopyRequest    
    is the 
    main-input type.
*/

struct srm22__srmCopyRequest * conv2soap_srm22_srmCopyRequest(struct soap *soap, const srm22_srmCopyRequest * _elem);
            
/*
    srmCopyResponse    
    is the 
    main-output type.
*/
srm22_srmCopyResponse * conv2gfal_srm22_srmCopyResponse(const struct srm22__srmCopyResponse * _elem);            	
            
/*
    srmStatusOfCopyRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmStatusOfCopyRequestRequest * conv2soap_srm22_srmStatusOfCopyRequestRequest(struct soap *soap, const srm22_srmStatusOfCopyRequestRequest * _elem);
            
/*
    srmStatusOfCopyRequestResponse    
    is the 
    main-output type.
*/
srm22_srmStatusOfCopyRequestResponse * conv2gfal_srm22_srmStatusOfCopyRequestResponse(const struct srm22__srmStatusOfCopyRequestResponse * _elem);            	
            
/*
    srmReleaseFilesRequest    
    is the 
    main-input type.
*/

struct srm22__srmReleaseFilesRequest * conv2soap_srm22_srmReleaseFilesRequest(struct soap *soap, const srm22_srmReleaseFilesRequest * _elem);
            
/*
    srmReleaseFilesResponse    
    is the 
    main-output type.
*/
srm22_srmReleaseFilesResponse * conv2gfal_srm22_srmReleaseFilesResponse(const struct srm22__srmReleaseFilesResponse * _elem);            	
            
/*
    srmPutDoneRequest    
    is the 
    main-input type.
*/

struct srm22__srmPutDoneRequest * conv2soap_srm22_srmPutDoneRequest(struct soap *soap, const srm22_srmPutDoneRequest * _elem);
            
/*
    srmPutDoneResponse    
    is the 
    main-output type.
*/
srm22_srmPutDoneResponse * conv2gfal_srm22_srmPutDoneResponse(const struct srm22__srmPutDoneResponse * _elem);            	
            
/*
    srmAbortRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmAbortRequestRequest * conv2soap_srm22_srmAbortRequestRequest(struct soap *soap, const srm22_srmAbortRequestRequest * _elem);
            
/*
    srmAbortRequestResponse    
    is the 
    main-output type.
*/
srm22_srmAbortRequestResponse * conv2gfal_srm22_srmAbortRequestResponse(const struct srm22__srmAbortRequestResponse * _elem);            	
            
/*
    srmAbortFilesRequest    
    is the 
    main-input type.
*/

struct srm22__srmAbortFilesRequest * conv2soap_srm22_srmAbortFilesRequest(struct soap *soap, const srm22_srmAbortFilesRequest * _elem);
            
/*
    srmAbortFilesResponse    
    is the 
    main-output type.
*/
srm22_srmAbortFilesResponse * conv2gfal_srm22_srmAbortFilesResponse(const struct srm22__srmAbortFilesResponse * _elem);            	
            
/*
    srmSuspendRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmSuspendRequestRequest * conv2soap_srm22_srmSuspendRequestRequest(struct soap *soap, const srm22_srmSuspendRequestRequest * _elem);
            
/*
    srmSuspendRequestResponse    
    is the 
    main-output type.
*/
srm22_srmSuspendRequestResponse * conv2gfal_srm22_srmSuspendRequestResponse(const struct srm22__srmSuspendRequestResponse * _elem);            	
            
/*
    srmResumeRequestRequest    
    is the 
    main-input type.
*/

struct srm22__srmResumeRequestRequest * conv2soap_srm22_srmResumeRequestRequest(struct soap *soap, const srm22_srmResumeRequestRequest * _elem);
            
/*
    srmResumeRequestResponse    
    is the 
    main-output type.
*/
srm22_srmResumeRequestResponse * conv2gfal_srm22_srmResumeRequestResponse(const struct srm22__srmResumeRequestResponse * _elem);            	
            
/*
    srmGetRequestSummaryRequest    
    is the 
    main-input type.
*/

struct srm22__srmGetRequestSummaryRequest * conv2soap_srm22_srmGetRequestSummaryRequest(struct soap *soap, const srm22_srmGetRequestSummaryRequest * _elem);
            
/*
    srmGetRequestSummaryResponse    
    is the 
    main-output type.
*/
srm22_srmGetRequestSummaryResponse * conv2gfal_srm22_srmGetRequestSummaryResponse(const struct srm22__srmGetRequestSummaryResponse * _elem);            	
            
/*
    srmExtendFileLifeTimeRequest    
    is the 
    main-input type.
*/

struct srm22__srmExtendFileLifeTimeRequest * conv2soap_srm22_srmExtendFileLifeTimeRequest(struct soap *soap, const srm22_srmExtendFileLifeTimeRequest * _elem);
            
/*
    srmExtendFileLifeTimeResponse    
    is the 
    main-output type.
*/
srm22_srmExtendFileLifeTimeResponse * conv2gfal_srm22_srmExtendFileLifeTimeResponse(const struct srm22__srmExtendFileLifeTimeResponse * _elem);            	
            
/*
    srmGetRequestTokensRequest    
    is the 
    main-input type.
*/

struct srm22__srmGetRequestTokensRequest * conv2soap_srm22_srmGetRequestTokensRequest(struct soap *soap, const srm22_srmGetRequestTokensRequest * _elem);
            
/*
    srmGetRequestTokensResponse    
    is the 
    main-output type.
*/
srm22_srmGetRequestTokensResponse * conv2gfal_srm22_srmGetRequestTokensResponse(const struct srm22__srmGetRequestTokensResponse * _elem);            	
            
/*
    srmGetTransferProtocolsRequest    
    is the 
    main-input type.
*/

struct srm22__srmGetTransferProtocolsRequest * conv2soap_srm22_srmGetTransferProtocolsRequest(struct soap *soap, const srm22_srmGetTransferProtocolsRequest * _elem);
            
/*
    srmGetTransferProtocolsResponse    
    is the 
    main-output type.
*/
srm22_srmGetTransferProtocolsResponse * conv2gfal_srm22_srmGetTransferProtocolsResponse(const struct srm22__srmGetTransferProtocolsResponse * _elem);            	
            
/*
    srmPingRequest    
    is the 
    main-input type.
*/

struct srm22__srmPingRequest * conv2soap_srm22_srmPingRequest(struct soap *soap, const srm22_srmPingRequest * _elem);
            
/*
    srmPingResponse    
    is the 
    main-output type.
*/
srm22_srmPingResponse * conv2gfal_srm22_srmPingResponse(const struct srm22__srmPingResponse * _elem);            	
            
        
#endif /*srm2_2_SOAP_CONVERSION_H_ */

