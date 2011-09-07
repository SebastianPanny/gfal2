
/* unit test for common_srm */


#include <cgreen/cgreen.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <time.h> 
#include "gfal_common_internal.h"
#include "gfal_posix_internal.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "srm/gfal_common_srm_internal_layer.h"
#include "../unit_test_constants.h"
#include "srm/gfal_common_srm_endpoint.h"
#include "../mock/gfal_mds_mock_test.h"
#include "../mock/gfal_srm_mock_test.h"

#define TEST_SRM_

void setup_mock_bdii(){
#if USE_MOCK
	
#endif
}

void setup_mock_srm(){
#if USE_MOCK
	setup_mock_bdii();
	gfal_srm_external_call.srm_prepare_to_get = &srm_mock_srm_prepare_to_get;
	gfal_srm_external_call.srm_prepare_to_put = &srm_mock_srm_prepare_to_put;
	gfal_srm_external_call.srm_context_init = &srm_mock_srm_context_init;
	gfal_srm_external_call.srm_check_permission= &srm_mock_srm_check_permission;
	gfal_srm_external_call.srm_ls = &srm_mock_srm_ls;
	gfal_srm_external_call.srm_mkdir = &srm_mock_srm_mkdir;
	gfal_srm_external_call.srm_rmdir = &srm_mock_srm_rmdir;
	gfal_srm_external_call.srm_put_done = &srm_mock_srm_put_done;
	gfal_srm_external_call.srm_setpermission= & srm_mock_srm_setpermission;
	gfal_srm_external_call.srm_srmv2_pinfilestatus_delete = &srm_mock_srm_srmv2_pinfilestatus_delete;
	gfal_srm_external_call.srm_srm2__TReturnStatus_delete = &srm_mock_srm_srm2__TReturnStatus_delete;
#endif
}

void mock_srm_access_right_response(char* surl){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;
	gfal_catalogs_instance(handle, NULL);
	char* surls[] = { surl, NULL };
	char* turls[] = { "nawak", NULL };
	int status[] = { 0, 0 };

	define_mock_srmv2_filestatus(1, surls, NULL,  turls, status);
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL);
	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	will_respond(srm_mock_srm_check_permission, 1, want_non_null(context), want_non_null(statuses), want_non_null(input));		
#endif
}

void mock_srm_access_error_response(char* surl, int merror){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;	
	char* explanation2[] = { "enoent mock", NULL };
	int status2[] = { merror, 0 };
	char* surls[] = { surl, NULL };	
	define_mock_srmv2_filestatus(1, surls, explanation2, NULL, status2);
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL);
	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	will_respond(srm_mock_srm_check_permission, 1, want_non_null(context), want_non_null(statuses), want_non_null(input));	
#endif
}



void test_srm_mock_chmod(char* url, int retcode){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	
	setup_mock_srm();
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL);

	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	will_respond(srm_mock_srm_setpermission, retcode, want_non_null(context), want_non_null(input));		
#endif	
	
}





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

/*
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
*/



void test_gfal_check_surl()
{
	GError* err=NULL;
	int ret;
	assert_true_with_message((ret = gfal_surl_checker(TEST_SRM_VALID_SURL_EXAMPLE1,&err) ) == 0, " error in url parse");
	gfal_check_GError(&err);
	assert_true_with_message((ret = gfal_surl_checker(TEST_SRM_VALID_SURL_EXAMPLE1,&err) ) == 0, " error in url parse 2");
	gfal_check_GError(&err);
	assert_false_with_message( (ret = gfal_surl_checker("http://google.com",&err ))== 0, " must fail , bad url");
	g_clear_error(&err);
}



void test_gfal_full_endpoint_checkG()
{
	GError* err = NULL;
	assert_true_with_message( gfal_check_fullendpoint_in_surlG( "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b-aaba-0e1341f30663",&err) ," fail, must be a success" );
	gfal_check_GError(&err);
	assert_true_with_message( gfal_check_fullendpoint_in_surlG("srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it", &err) == FALSE, " fail, must be a failure : bad url");
	gfal_check_GError(&err);
	assert_true_with_message( gfal_check_fullendpoint_in_surlG( "srm://lxb5409.cern.ch:8446/srm/managerv2?SFN=/dpm/cern.ch/home/dteam/srmv2_tests/test_lfc_3897",&err) ," fail, must be a success" );
	gfal_check_GError(&err);
}
	
void test_gfal_get_endpoint_and_setype_from_bdiiG(){
	GError* err= NULL;
	int i1;
	char buff_endpoint[2048];
	memset(buff_endpoint, '\0', sizeof(char)*2048);
	enum gfal_srm_proto proto;
	
#if USE_MOCK
	setup_mock_bdii();
	char buff_tmp[2048];
	char* p = TEST_SRM_DPM_ENDPOINT_PREFIX+ strlen(GFAL_PREFIX_SRM);
	g_strlcpy(buff_tmp, p, strchr(p, '/')-p+1);
	define_se_endpoints = calloc(sizeof(char*), 4);
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	




#endif
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");	
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	char* surl = TEST_SRM_DPM_ENDPOINT_PREFIX;
	int ret = gfal_get_endpoint_and_setype_from_bdiiG(&opts, surl, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err== NULL && strings_are_equal(buff_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL) && proto== PROTO_SRMv2, " must be a valid endpoint resolution");
	gfal_check_GError(&err);
	memset(buff_endpoint, '\0', sizeof(char)*2048);
	ret = gfal_get_endpoint_and_setype_from_bdiiG(&opts, "srm://lxb540dfshhhh9.cern.ch:8446/test/invalid", buff_endpoint, 2048, &proto, &err);
	assert_true_with_message(ret != 0 && err != NULL && err->code==ENXIO && *buff_endpoint== '\0', " must fail, invalid point");
	g_clear_error(&err);
	gfal_handle_freeG(handle);
}


void test_gfal_srm_determine_endpoint_full_endpointG()
{
#if USE_MOCK
	setup_mock_bdii();

#endif
	GError* err = NULL;
	char buff_endpoint[2048];
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	int ret =-1;
	ret = gfal_srm_determine_endpoint(&opts, TEST_SRM_DPM_FULLENDPOINT_PREFIX, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL), " must be a succesfull endpoint determination %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);	
	
	ret = gfal_srm_determine_endpoint(&opts, "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b", buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, "httpg://srm-pps:8443/srm/managerv2"), " must be a succesfull endpoint determination 2 %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);
	gfal_handle_freeG(handle);	
}




void test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdiiG()
{
#if USE_MOCK
	setup_mock_bdii();

#endif
	GError* err = NULL;
	char buff_endpoint[2048];
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_catalogs_instance(handle, NULL);
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gfal_set_nobdiiG(handle, TRUE);
	int ret =-1;
	ret = gfal_srm_determine_endpoint(&opts, TEST_SRM_DPM_FULLENDPOINT_PREFIX, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL), " must be a succesfull endpoint determination %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);	
	
	ret = gfal_srm_determine_endpoint(&opts, "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b", buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, "httpg://srm-pps:8443/srm/managerv2"), " must be a succesfull endpoint determination 2 %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);
	memset(buff_endpoint,0, sizeof(char)*2048);
	ret = gfal_srm_determine_endpoint(&opts, TEST_SRM_VALID_SURL_EXAMPLE1, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret !=0 && err != NULL && err->code== EINVAL && *buff_endpoint=='\0', " must be a reported error, bdii is disable");
	g_clear_error(&err);
	gfal_handle_freeG(handle);	
}



void test_gfal_srm_determine_endpoint_not_fullG()
{
	
#if USE_MOCK
	int i1;
	setup_mock_bdii();
	char buff_tmp[2048];
	char* p = TEST_SRM_DPM_ENDPOINT_PREFIX+ strlen(GFAL_PREFIX_SRM);
	g_strlcpy(buff_tmp, p, strchr(p, '/')-p+1);
	define_se_endpoints = calloc(sizeof(char*), 4);
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	



#endif
	GError* err = NULL;
	char buff_endpoint[2048];
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gfal_catalogs_instance(handle, NULL);
	int ret =-1;
	ret = gfal_srm_determine_endpoint(&opts, TEST_SRM_DPM_ENDPOINT_PREFIX, buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret ==0 && err == NULL && strings_are_equal(buff_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL), " must be a succesfull endpoint determination %d %ld %s", ret, err, buff_endpoint);
	gfal_check_GError(&err);	
	
	ret = gfal_srm_determine_endpoint(&opts, "http://google.com", buff_endpoint, 2048, &proto, &err);
	assert_true_with_message( ret !=0 && err != NULL, "error must be reported");
	g_clear_error(&err);
	gfal_handle_freeG(handle);	
}







void test_gfal_get_hostname_from_surl()
{
	GError * tmp_err=NULL;
	char hostname[2048];
	memset(hostname, '\0', 2048);
	int ret= gfal_get_hostname_from_surlG(TEST_SRM_VALID_SURL_EXAMPLE1, hostname, 2048, &tmp_err);
	assert_true_with_message( ret==0  && tmp_err==NULL && *hostname!='\0', " must be a success");
	
	char* p = strchr(TEST_SRM_DPM_ENDPOINT_PREFIX+7, '/');
	assert_true_with_message(p!=NULL, " no / contained in the url");
	if(!p)	
		return;
	assert_true_with_message( strncmp(TEST_SRM_DPM_ENDPOINT_PREFIX+6,hostname, p-TEST_SRM_DPM_ENDPOINT_PREFIX-7) == 0, " must be the same string");
}



void test_gfal_select_best_protocol_and_endpointG()
{
	char endpoint[2048];
	memset(endpoint, '\0', sizeof(char)*2048);
	enum gfal_srm_proto srm_type;
	GError * err= NULL;
	gfal_handle handle  = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_catalogs_instance(handle, NULL);
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gfal_set_default_storageG(&opts, PROTO_SRMv2);
	char* endpoint_list[] = { "everest", "montblanc", NULL};
	char* se_type_list[] = { "srm_v1", "srm_v2", NULL };
	int ret = gfal_select_best_protocol_and_endpointG(&opts, se_type_list, endpoint_list, &endpoint, 2048, &srm_type, &err);
	assert_true_with_message(ret ==0 && err == NULL, " must be a succefull call to the best select");
	gfal_check_GError(&err);

	assert_true_with_message(strings_are_equal(endpoint,"montblanc"), " reponse not match correctly");
	// try with another version by default
	gfal_set_default_storageG(&opts, PROTO_SRM);
	ret = gfal_select_best_protocol_and_endpointG(&opts, se_type_list, endpoint_list, &endpoint, 2048, &srm_type, &err);
	assert_true_with_message(ret ==0 && err == NULL, " must be a succefull call to the best select");
	gfal_check_GError(&err);
	assert_true_with_message(strings_are_equal(endpoint,"everest") , "must be a valid check");	
	gfal_handle_freeG(handle);
}


void test_gfal_srm_getTURLS_one_success()
{
	setup_mock_srm();
	setup_mock_bdii();
	int i1;
#if USE_MOCK
	define_se_endpoints = calloc(sizeof(char*), 4); // set the response of the MDS layer for endpoint
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	


	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	defined_get_output = calloc(sizeof(struct srmv2_pinfilestatus),1);
	defined_get_output[0].turl= strdup(TEST_SRM_TURL_EXAMPLE1);
	will_respond(srm_mock_srm_prepare_to_get, 1, want_non_null(context), want_non_null(input), want_non_null(output));
	always_return(srm_mock_srm_srmv2_filestatus_delete,0);
	always_return(srm_mock_srm_srmv2_pinfilestatus_delete,0);
#endif
	GError* tmp_err=NULL;
	gfal_handle handle  = gfal_initG(&tmp_err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_catalogs_instance(handle, NULL);	
	gfal_srm_opt_initG(&opts, handle);

	gfal_srm_result* resu=NULL;
	char* surls[] = {TEST_SRM_VALID_SURL_EXAMPLE1, NULL};
	int ret = gfal_srm_getTURLS(&opts, surls, &resu, &tmp_err);
	assert_true_with_message(ret ==1 && resu != NULL && tmp_err == NULL, " must be a successfull request");
	gfal_check_GError(&tmp_err);
	assert_true_with_message(resu[0].err_code == 0 && *(resu[0].err_str)== '\0' && strings_are_equal(resu[0].turl, TEST_SRM_TURL_EXAMPLE1), 
				" must be a valid turl, maybe the turl has changed %d %ld %s",resu[0].err_code, resu[0].err_str, resu[0].turl);	
	free(resu);
	gfal_handle_freeG(handle);
}


void test_gfal_srm_getTURLS_bad_urls()
{
	GError* tmp_err=NULL;
	gfal_handle handle  = gfal_initG(&tmp_err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srm_result* resu=NULL;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	char* surls[] = {NULL, NULL};
	int ret = gfal_srm_getTURLS(&opts, surls, &resu, &tmp_err);
	assert_true_with_message(ret <=0 && resu == NULL && tmp_err != NULL, " must be a failure, invalid SURLs ");
	g_clear_error(&tmp_err);
	free(resu);
	gfal_handle_freeG(handle);
}


void test_gfal_srm_getTURLS_pipeline_success()
{
	setup_mock_srm();
	setup_mock_bdii();
	int i1;
#if USE_MOCK
	define_se_endpoints = calloc(sizeof(char*), 4); // set the response of the MDS layer for endpoint
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(TEST_SRM_DPM_FULLENDPOINT_URL);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	


	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	defined_get_output = calloc(sizeof(struct srmv2_pinfilestatus),3);
	defined_get_output[0].turl= strdup(TEST_SRM_TURL_EXAMPLE1);
	defined_get_output[1].status= ENOENT;
	defined_get_output[1].explanation = strdup("err msg");
	defined_get_output[2].turl = strdup(TEST_SRM_TURL_EXAMPLE1);
	will_respond(srm_mock_srm_prepare_to_get, 3, want_non_null(context), want_non_null(input), want_non_null(output));
	always_return(srm_mock_srm_srmv2_filestatus_delete,0);
	always_return(srm_mock_srm_srmv2_pinfilestatus_delete,0);
#endif
	GError* tmp_err=NULL;
	gfal_handle handle  = gfal_initG(&tmp_err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	if(handle==NULL)
		return;
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gfal_catalogs_instance(handle, NULL);
	gfal_srm_result* resu=NULL;
	char* surls[] = {TEST_SRM_VALID_SURL_EXAMPLE1, TEST_SRM_INVALID_SURL_EXAMPLE2, TEST_SRM_VALID_SURL_EXAMPLE1, NULL};
	int ret = gfal_srm_getTURLS(&opts, surls, &resu, &tmp_err);
	assert_true_with_message(ret ==g_strv_length(surls) && resu != NULL && tmp_err == NULL, " must be a successfull request");
	gfal_check_GError(&tmp_err);
	assert_true_with_message(resu[0].err_code == 0 && *(resu[0].err_str)== '\0' && strings_are_equal(resu[0].turl, TEST_SRM_TURL_EXAMPLE1), 
				" must be a valid turl, maybe the turl has changed %d %ld %s",resu[0].err_code, resu[0].err_str, resu[0].turl);	

	assert_true_with_message(resu[1].err_code == ENOENT && *(resu[1].err_str)!= '\0' && *(resu[1].turl) == '\0', 
				" must be a invalid turl 2 ");	
	assert_true_with_message(resu[2].err_code == 0 && *(resu[2].err_str)== '\0' && strings_are_equal(resu[2].turl, TEST_SRM_TURL_EXAMPLE1), 
				" must be a valid turl 3, maybe the turl has changed ");	
	free(resu);
	gfal_handle_freeG(handle);
}


/*
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

}*/
