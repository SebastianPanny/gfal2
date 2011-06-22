
/* unit test for common_srm */


#include <cgreen/cgreen.h>
#include "gfal_common.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "gfal_common_internal.h"
#include "../unit_test_constants.h"
#include "mds/gfal_common_mds.h"
#include "srm/gfal_common_srm_endpoint.h"
#include <regex.h>
#include <time.h> 

#define TEST_SRM_

void test_create_srm_handle()
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	assert_true_with_message(err==NULL, " error must be NULL");
	gfal_handle_freeG(handle);
}

void test__gfal_convert_full_surl()
{
	int res = -1;
	char buff[2048];
	GError* tmp_err=NULL;
	res=  gfal_get_fullendpointG(TEST_SRM_DPM_FULLENDPOINT_PREFIX, buff, 2048, &tmp_err);
	assert_true_with_message( res == 0 && strings_are_equal(buff, TEST_SRM_DPM_FULLENDPOINT_URL) && tmp_err==NULL, " must be a successfull endpoint convertion");
	g_clear_error(&tmp_err);
	res=  gfal_get_fullendpointG(TEST_SRM_DPM_FULLENDPOINT_PREFIX, buff, 2, &tmp_err);	
	assert_true_with_message( res != 0 && tmp_err!=NULL && tmp_err->code == ENOBUFS, " must be a buffer to small");	
	g_clear_error(&tmp_err);
}


void test_gfal_get_async_1()
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
			gfal_release_GError(&err);
			assert_true_with_message(FALSE, " handle fail to initiated");
	}
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile002");
	int r = gfal_get_asyncG(handle,list,&err);
	if(r <0){
			gfal_release_GError(&err);
			assert_true_with_message(FALSE, "must be a success");
			return;
	}	
	gfal_handle_freeG(handle);
}




void test_gfal_check_surl()
{
	GError* err=NULL;
	int ret;
	assert_true_with_message((ret = gfal_surl_checker(TEST_SRM_VALID_SURL_EXAMPLE1,&err) ) == 0, " error in url parse");
	if(ret)
		gfal_release_GError(&err);
	assert_true_with_message((ret = gfal_surl_checker(TEST_SRM_VALID_SURL_EXAMPLE1,&err) ) == 0, " error in url parse 2");
	if(ret)
		gfal_release_GError(&err);
	assert_false_with_message( (ret = gfal_surl_checker("http://google.com",&err ))== 0, " must fail , bad url");
	g_clear_error(&err);
}



void test_gfal_full_endpoint_check()
{
	GError* err = NULL;
	int ret = -1;
	assert_false_with_message( (ret= gfal_check_fullendpoint_in_surl( "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b-aaba-0e1341f30663",&err)) ," fail, must be a success" );
	if(ret)
		gfal_release_GError(&err);
	assert_false_with_message( (ret = gfal_check_fullendpoint_in_surl("srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it", &err)) != REG_NOMATCH, " fail, must be a failure : bad url");
	assert_false_with_message( (ret= gfal_check_fullendpoint_in_surl( "srm://lxb5409.cern.ch:8446/srm/managerv2?SFN=/dpm/cern.ch/home/dteam/srmv2_tests/test_lfc_3897",&err)) ," fail, must be a success" );
	if(ret)
		gfal_release_GError(&err);
}
	


void test_gfal_auto_get_srm_endpoint_full_endpoint()
{
	GError* err = NULL;
	char * endpoint=NULL;
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	int ret =-1;
		
	GList* list = g_list_append(NULL,"srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b");
	assert_true_with_message(gfal_auto_get_srm_endpoint(handle, NULL, &proto, list, &err) || err ==NULL, "must fail, einval");	// test the params checker
	g_clear_error(&err);
	assert_true_with_message(gfal_auto_get_srm_endpoint(handle, &endpoint, NULL, list, &err) || err ==NULL, "must fail, einval");
	g_clear_error(&err);
	assert_true_with_message(gfal_auto_get_srm_endpoint(handle, &endpoint, NULL, list, &err) || err ==NULL, "must fail, einval");
	g_clear_error(&err);
	
	ret = gfal_auto_get_srm_endpoint(handle, &endpoint, &proto, list, &err) ;
	assert_false_with_message(ret , " must return the correct endpoint");
	assert_false_with_message( endpoint == NULL || strstr(endpoint,"httpg://srm-pps:8443/srm/managerv2") == NULL, " must contain the endpoint");
	assert_false_with_message(proto != handle->srm_proto_type, " srm must be the default version of srm");
	gfal_handle_freeG(handle);	
	g_list_free(list);
	free(endpoint);
}




void test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdii()
{
	GError* err = NULL;
	char * endpoint=NULL;
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	int ret =-1;
	
	gfal_set_nobdiiG(handle, TRUE);
	assert_true_with_message(handle->no_bdii_check, " nobdii must be true");
		
	GList* list = g_list_append(NULL,TEST_SRM_DPM_FULLENDPOINT_PREFIX);	
	ret = gfal_auto_get_srm_endpoint(handle, &endpoint, &proto, list, &err);
	assert_false_with_message( ret , " must return the correct endpoint");
	assert_false_with_message( endpoint == NULL || strstr(endpoint,TEST_SRM_DPM_FULLENDPOINT_URL) == 0, " must contain the endpoint");
	assert_false_with_message(proto != handle->srm_proto_type, " srm must be the default version of srm");
	gfal_handle_freeG(handle);	
	g_list_free(list);
	free(endpoint);
}



void test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdii_negative()
{
	GError* err = NULL;
	char * endpoint=NULL;
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	int ret =-1;
	
	gfal_set_nobdiiG(handle, TRUE);
	assert_true_with_message(handle->no_bdii_check, " nobdii must be true");
		
	GList* list = g_list_append(NULL,TEST_SRM_VALID_SURL_EXAMPLE1);
	
	assert_true_with_message( ret = gfal_auto_get_srm_endpoint(handle, &endpoint, &proto, list, &err) , " must return error because not a full srm");
	g_clear_error(&err);
	gfal_handle_freeG(handle);	
	g_list_free(list);
	free(endpoint);
}



void test_gfal_auto_get_srm_endpoint_no_full_with_bdii()
{
	GError* err = NULL;
	char * endpoint=NULL;
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	int ret =-1;
	
	assert_false_with_message(handle->no_bdii_check, " nobdii must be false");
		
	GList* list = g_list_append(NULL,TEST_SRM_DPM_FULLENDPOINT_PREFIX);
	ret = gfal_auto_get_srm_endpoint(handle, &endpoint, &proto, list, &err);
	assert_false_with_message( ret , " must return the correct endpoint");
	assert_false_with_message( endpoint == NULL || strcmp(endpoint,TEST_SRM_DPM_FULLENDPOINT_URL) != 0, " must contain the endpoint");
	assert_false_with_message(proto != handle->srm_proto_type, " srm must be the default version of srm");
	gfal_handle_freeG(handle);	
	g_list_free(list);
	free(endpoint);
}



void test_gfal_get_fullendpoint(){
	const char * surl = "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b";
	GError* err=NULL;
	char* endpoint = gfal_get_fullendpoint(surl,&err);
	assert_false_with_message( endpoint == NULL || err || strcmp(endpoint,"httpg://srm-pps:8443/srm/managerv2"), " must be successfull");
	//fprintf(stderr, " endpoint from surl %s ", endpoint);
	free(endpoint);
	const char * surl2 = "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04";	
	assert_false_with_message( (endpoint = gfal_get_fullendpoint(surl2,&err)) !=NULL || err==NULL , " must fail");	
	g_clear_error(&err);
	free(endpoint);
}



void test_gfal_get_hostname_from_surl()
{
	GList* list = g_list_append(NULL,TEST_SRM_VALID_SURL_EXAMPLE1);	
	GError * tmp_err=NULL;
	char* resu;
	assert_true_with_message( (resu = gfal_get_hostname_from_surl(list->data, &tmp_err)) && !tmp_err, " must be a success");
	if(resu){
		char* p = strchr(TEST_SRM_DPM_ENDPOINT_PREFIX+7, '/');
		if(!p)
			assert_true_with_message(FALSE, " no / contained in the url");
		assert_true_with_message( strncmp(TEST_SRM_DPM_ENDPOINT_PREFIX+6,resu, p-TEST_SRM_DPM_ENDPOINT_PREFIX-7) == 0, " must be the same string");
		free(resu); 
	}
	g_list_free(list);
	list = g_list_append(NULL,"http://google.com/");	
	assert_false_with_message( (resu = gfal_get_hostname_from_surl(list->data, &tmp_err)) && !tmp_err, " must be an error");
	g_clear_error(&tmp_err);
	g_list_free(list);
	free(resu);
}



void test_gfal_get_endpoint_and_setype_from_bdii()
{
	char *endpoint;
	enum gfal_srm_proto srm_type;
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54");	
	GError * err= NULL;
	gfal_handle handle  = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, " handle is not properly allocated");
		return;
	}
	
	int ret = gfal_get_endpoint_and_setype_from_bdii(handle, &endpoint, &srm_type, list, &err);
	if(ret){
		assert_true_with_message(FALSE,  " fail, must be a valid return");		
		gfal_release_GError(&err);
		return;
	}
	assert_true_with_message( srm_type == PROTO_SRMv2, " must be the default protocol");
	assert_true_with_message( strcmp(endpoint, "httpg://grid-cert-03.roma1.infn.it:8446/srm/managerv2") == 0, "must be this endpoint ");	
	g_list_free(list);
	free(endpoint);
	gfal_handle_freeG(handle);
}



void test_gfal_select_best_protocol_and_endpoint()
{
	char *endpoint=NULL;
	enum gfal_srm_proto srm_type;
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54");	
	GError * err= NULL;
	gfal_handle handle  = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, " handle is not properly allocated");
		return;
	}
	gfal_set_default_storageG(handle, PROTO_SRMv2);
	const char* endpoint_list[] = { "everest", "montblanc", NULL};
	const char* se_type_list[] = { "srm_v1", "srm_v2", NULL };
	int ret = gfal_select_best_protocol_and_endpoint(handle, &endpoint, &srm_type, se_type_list, endpoint_list, &err);
	if(ret){
			assert_true_with_message(FALSE, " must successfull");
			gfal_release_GError(&err);
	}
	assert_false_with_message(strcmp(endpoint,"montblanc")!=0, " reponse not match correctly");
	free(endpoint);	
	// try with another version by default
	gfal_set_default_storageG(handle, PROTO_SRM);
	ret = gfal_select_best_protocol_and_endpoint(handle, &endpoint, &srm_type, se_type_list, endpoint_list, &err);
	if(ret){
			assert_true_with_message(FALSE, " must successfull");
			gfal_release_GError(&err);
	}	
	assert_true_with_message(strcmp(endpoint,"everest") ==0, "must be a valid check");	
	free(endpoint);	
	gfal_handle_freeG(handle);
}




void gfal_get_asyncG_empty_req()
{
	GError *err = NULL;
	GList* list = g_list_append(NULL,NULL);
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, "fail to init handle");
		return;
	}
	int ret = gfal_get_asyncG(handle, list, &err);
	assert_true_with_message(ret, " this request must fail, no surls in the list");
	g_list_free(list);
	g_clear_error(&err);
	list = g_list_append(NULL,"google.com");
	list = g_list_append(NULL,"amazon.com");
	ret = gfal_get_asyncG(handle, list, &err);
	assert_true_with_message(ret, " this request must fail, bad surl in the list");
	g_clear_error(&err);	
	gfal_handle_freeG(handle);
	
}


void gfal_get_asyncG_empty_old_nonexist_surl()
{
	GError *err = NULL;
	GList* list =NULL;
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, "fail to init handle");
		return;
	}
	list = g_list_append(NULL,"srm://lxdpm103.cern.ch/dpm/cern.ch/home/dteam/generated/2009-09-21/file712de300-7a1e-4c78-90d2-0ba187e5a8da");
	int ret = gfal_get_asyncG(handle, list, &err);
	assert_true_with_message(ret, " this request must fail old non exist surl");
	g_list_free(list);	
	g_clear_error(&err);
	gfal_handle_freeG(handle);
}


void test_gfal_is_finished()
{
	char *endpoint;
	enum gfal_srm_proto srm_type;
	GList* list = g_list_append(NULL,TEST_SRM_VALID_SURL_EXAMPLE1);	
	GError * err= NULL;
	gfal_handle handle  = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, " handle is not properly allocated");
		return;
	}
	int ret = gfal_get_asyncG(handle, list, &err);
	assert_false_with_message(ret <0, " this request must be a success, valid surl");
	if(ret <0){
		gfal_release_GError(&err);
		return;
	}
	
	clock_t end;
	end = clock () + 5 * CLOCKS_PER_SEC ;
	gboolean finished = FALSE;
	while(!finished){
		if(end < clock())
			break;
		finished = gfal_async_request_is_finishedG(handle, &err);
		if(err)
			break;
	}
	assert_true_with_message(finished, " the request must finished correctly");
	assert_false_with_message(err, " error return must be empty ");
	if(err)
		gfal_release_GError(&err);
	
	//g_list_free(list);
	gfal_handle_freeG(handle);
}



void test_gfal_waiting_async()
{
	char *endpoint;
	enum gfal_srm_proto srm_type;
	GList* list = g_list_append(NULL,TEST_SRM_VALID_SURL_EXAMPLE1);	
	GError * err= NULL;
	gfal_handle handle  = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, " handle is not properly allocated");
		return;
	}
	int ret = gfal_get_asyncG(handle, list, &err);
	assert_false_with_message(ret <0, " this request must be a success, valid surl");
	if(ret <0){
		gfal_release_GError(&err);
		return;
	}
	
	ret= gfal_wait_async_requestG(handle, 5, &err);
	assert_true_with_message(ret == 0, " the request must finished correctly : code %d ", ret);
	assert_false_with_message(err, " error return must be empty ");
	if(err)
		gfal_release_GError(&err);
	
	gfal_handle_freeG(handle);	
	g_list_free(list);
}


void test_gfal_get_async_resultsG()
{
	GError *err = NULL;
	GList* list = g_list_append(NULL,TEST_SRM_VALID_SURL_EXAMPLE1);	
	GList* list_resu;	
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, "fail to init handle");
		return;
	}
	int ret = gfal_get_asyncG(handle, list, &err);
	assert_false_with_message(ret <0, " this request must be a success, valid surl");
	if(ret <0){
		gfal_release_GError(&err);
		return;
	}
	ret=  gfal_wait_async_requestG(handle, 5, &err);
	assert_true_with_message(ret == 0, " the request must finished correctly");
	if(ret ){
		gfal_handle_freeG(handle);
		return;
	}
		
	ret = gfal_get_async_resultsG(handle, &list_resu, &err);
	//fprintf(stderr, " result turl : %s", list_resu->data);
	assert_false_with_message(ret !=1 , " must return good status");
	assert_false_with_message(err, " error report must be null");
	while(list_resu != NULL){
		assert_false_with_message( list_resu->data == NULL, " must be a success ");
	//	g_printerr(" turl : %s", list_resu->data);
		list_resu = g_list_next(list_resu);
	}
	g_list_free(list);
	g_list_free_full(list_resu, free);
	gfal_handle_freeG(handle);
	return;
}


void test_gfal_get_async_resultsG_empty()
{
	GError *err = NULL;
	gfal_handle handle = NULL; // srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile002
	GList* list;
	int ret = gfal_get_async_resultsG(handle, &list, &err);
	assert_true_with_message(ret <0, " must be a failure, handle empty");	
	g_clear_error(&err);
	handle = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, "fail to init handle");
		return;
	}
	ret = gfal_get_async_resultsG(handle, &list, &err);
	assert_true_with_message(ret<0  , " must be a failure, ne get before");	
	g_clear_error(&err);
	gfal_handle_freeG(handle);
}



void test_gfal_async_results_errcode()			// verify the errcode of a bad request
{
	gfal_handle handle = NULL;
	GList* list = g_list_append(NULL, "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile002");
	GList* errcode=NULL;
	GError* err = NULL;
	int ret = gfal_get_async_results_errcodesG(handle, &errcode, &err);			// test without handle, must fail
	if( ret >= 0 || !err || errcode ){
		assert_true_with_message(FALSE, " must return error, handle not initiated");
		return;
	}
	g_clear_error(&err);
	handle = gfal_initG(&err);
	if(err){
		assert_true_with_message(FALSE, " must initiated properly");
		gfal_release_GError(&err);
		return;				
	}
	ret = gfal_get_async_results_errcodesG(handle, &errcode, &err);			// test without get before, must fail
	if( ret>=0 || !err || errcode){
		assert_true_with_message(FALSE, " must fail, no request before");
		return;				
	}
	g_clear_error(&err);
	ret = gfal_get_asyncG(handle, list, &err);
	if(ret <=0){
		assert_true_with_message(FALSE, " need to be a success");
		gfal_release_GError(&err);
		return;
	}
	ret=  gfal_wait_async_requestG(handle, 5, &err);
	if(ret ){
		assert_true_with_message(FALSE, " the request must finished correctly");
		gfal_release_GError(&err);
		gfal_handle_freeG(handle);
		return;
	}
	ret = gfal_get_async_results_errcodesG(handle, &errcode, &err);			// test without get before, must fail
	if( ret <=0 || err ){
		assert_true_with_message(FALSE, " must be a success");
		return;				
	}
	gfal_handle_freeG(handle);
	
}


void test_full_gfal_get_request()
{
	GError *err = NULL;
	gfal_handle handle = NULL; 
	GList* list = g_list_append(NULL,TEST_SRM_VALID_SURL_EXAMPLE1);
	GList *list_resu, *list_resu_err;
	handle = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, "fail to init handle");
		return;
	}		
	int ret = gfal_get_asyncG(handle, list, &err);
	if(ret <0){
		assert_true_with_message(FALSE, " this request must be a success, valid surl");
		gfal_release_GError(&err);
		return;
	}
	ret=  gfal_wait_async_requestG(handle, 5, &err);
	if(ret ){
		assert_true_with_message(FALSE, " the request must finished correctly");
		gfal_release_GError(&err);
		gfal_handle_freeG(handle);
		return;
	}
	ret = gfal_get_async_resultsG(handle, &list_resu, &err);
	if(err || list_resu->data == NULL ){
		assert_true_with_message(FALSE,  " error must not occured");
		gfal_release_GError(&err);
		gfal_handle_freeG(handle);
		return;			
	}
	if(  strncmp(list_resu->data, TEST_SRM_TURL_EXAMPLE1,1024) != 0){
		assert_true_with_message(FALSE,  " is not the good turl  : %s, %s", list_resu->data, TEST_SRM_TURL_EXAMPLE1);
		gfal_handle_freeG(handle);
		return;
	}
	ret = gfal_get_async_results_errcodesG(handle, &list_resu_err, &err);
	GList* tmp_list = list_resu_err;
	if(ret <0){
			assert_true_with_message(FALSE, " return error on errcode get ");
			gfal_release_GError(&err);
			gfal_handle_freeG(handle);
			return;
	}
	while( tmp_list != NULL){
		if( tmp_list->data != GINT_TO_POINTER(0)){
			assert_true_with_message(FALSE,  "must not report error");
			gfal_handle_freeG(handle);	
			return;		
		}
		tmp_list = g_list_next(tmp_list);
	}
	g_list_free(list_resu_err);
	ret = gfal_get_async_results_errstringG(handle, &list_resu_err, &err);
	if(ret <0){
			assert_true_with_message(FALSE, " return error on err string get ");
			gfal_release_GError(&err);
			gfal_handle_freeG(handle);
			return;		
	}
	tmp_list = list_resu_err;
	while( tmp_list != NULL){
		if( tmp_list->data != NULL){
			assert_true_with_message(FALSE,  "must not report error string ");
			gfal_handle_freeG(handle);	
			return;		
		}
		tmp_list = g_list_next(tmp_list);
	}
	g_list_free_full(list_resu_err,free); 
	g_list_free_full(list_resu,free); 
	gfal_handle_freeG(handle);
}


void test_gfal_get_request_struct()
{
	GError *err = NULL;
	gfal_handle handle = NULL; 
	GList* list = g_list_append(NULL,TEST_SRM_VALID_SURL_EXAMPLE1);
	gfal_srm_result* results=NULL;
	
	handle = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, "fail to init handle");
		return;
	}		
	int ret = gfal_get_asyncG(handle, list, &err);
	if(ret <0){
		assert_true_with_message(FALSE, " this request must be a success, valid surl");
		gfal_release_GError(&err);
		return;
	}
	ret=  gfal_wait_async_requestG(handle, 5, &err);
	if(ret ){
		assert_true_with_message(FALSE, " the request must finished correctly");
		gfal_release_GError(&err);
		gfal_handle_freeG(handle);
		return;
	}
	ret = gfal_get_async_results_structG(handle, &results, &err);
	if(err || ret != g_list_length(list) || results == NULL ){
		assert_true_with_message(FALSE,  " error must not occured");
		gfal_release_GError(&err);
		gfal_handle_freeG(handle);
		return;			
	}
	int i;
	for(i=0 ; i< ret; ++i){
		assert_false_with_message( results->err_code !=0, "fail must be a valid request : %d ", results->err_code );
		assert_false_with_message( *(results->err_str)  != 0, "fail, must be an empty error string : %s", results->err_str);
		assert_false_with_message( strncmp(results->turl,TEST_SRM_TURL_EXAMPLE1,1024)!=0, "fail, must be a valid turl : %s %s ", results->turl, TEST_SRM_TURL_EXAMPLE1);
	}
	free(results);
	return;

}



void test_full_gfal_get_request_multi()
{
	GError *err = NULL;
	gfal_handle handle = NULL; 
	GList* list = g_list_append(NULL,TEST_SRM_VALID_SURL_EXAMPLE1);
	list = g_list_append(list, TEST_SRM_INVALID_SURL_EXAMPLE2);
	list = g_list_append(list,TEST_SRM_VALID_SURL_EXAMPLE1);
	GList *list_resu=NULL, *list_resu_err=NULL;
	handle = gfal_initG(&err);
	if(handle == NULL){
		assert_true_with_message(FALSE, "fail to init handle");
		return;
	}		
	int ret = gfal_get_asyncG(handle, list, &err);
	assert_false_with_message(ret <0, " this request must be a success, valid surl");
	if(ret <0){
		gfal_release_GError(&err);
		return;
	}
	ret=  gfal_wait_async_requestG(handle, 5, &err);
	assert_true_with_message(ret == 0, " the request must finished correctly");
	if(ret ){
		gfal_handle_freeG(handle);
		return;
	}
	ret = gfal_get_async_resultsG(handle, &list_resu, &err);
	if( ret <= 0 ){
		assert_true_with_message(FALSE,  " must not return error ");
		gfal_handle_freeG(handle);
		return;
	}
	
	const char* resu[] = { TEST_SRM_TURL_EXAMPLE1, NULL, TEST_SRM_TURL_EXAMPLE1 }; // check all results
	if( check_GList_Result_String(list_resu, resu) != TRUE){
		assert_true_with_message(FALSE, " incorrect result %s", list_resu->data);
		gfal_handle_freeG(handle);
		return;
	}
	
	ret = gfal_get_async_results_errcodesG(handle, &list_resu_err, &err);
	if( ret<0 ){
		assert_true_with_message(FALSE,  " must not report error ");
		gfal_handle_freeG(handle);	
		return;		
	}
	
	
	g_list_free(list_resu_err);
	ret = gfal_get_async_results_errstringG(handle, &list_resu_err, &err);
	if( ret <= 0){
		assert_true_with_message(FALSE,  " must be a success ");
		gfal_handle_freeG(handle);	
		return;		
	}
	const char* resu_errstring[] = { NULL, TEST_SRM_IFCE_INVALID_PATH_REP, NULL };
	if( check_GList_Result_String(list_resu_err, resu_errstring) != TRUE){
		assert_true_with_message(FALSE, " incorrect error string");
		gfal_handle_freeG(handle);
		return;
	}
	g_list_free_full(list_resu_err, &free);
	gfal_handle_freeG(handle);

}
