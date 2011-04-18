

/* unit test for common_srm */


#include <check.h>
#include "gfal_common.h"





START_TEST (test_create_srm_handle)
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	fail_unless(handle != NULL, " handle is not properly allocated");
	fail_unless(err==NULL, " error must be NULL");
	gfal_handle_freeG(handle);
}
END_TEST

START_TEST(test_glist_to_surls)
{
		GList* list = g_list_append(NULL,"bob");
		list = g_list_append(list, "gilles");
		list = g_list_append(list, "john");
		int n = g_list_length(list);
		fail_unless(n == 3, " size error, see internal glist doc");
		char** surls =gfal_GList_to_tab(list);
		int i;
		for(i=0; i< n; ++i){
			char * str= list->data;
			fail_unless(strncmp(str,surls[i],100) == 0, " must be the same string");
			list = g_list_next(list);
		}
		fail_unless(surls[n]==NULL, " last element+1 must be null");
}
END_TEST


START_TEST(test_gfal_get_async_1)
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
			gfal_release_GError(&err);
			fail(" handle fail to initiated");
	}
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile002");
	int r = gfal_get_asyncG(handle,list,&err);
	if(r < 0){
			gfal_release_GError(&err);
			fail("must be a success");
			return;
	}	
	
}
END_TEST



START_TEST(test_gfal_check_surl)
{
	GError* err=NULL;
	int ret;
	fail_unless((ret = gfal_surl_checker("srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54",&err) ) == 0, " error in url parse");
	if(ret)
		gfal_release_GError(&err);
	fail_unless((ret = gfal_surl_checker("srm://g_rid-cer_t-03.rOMa1.in_fn.it/dpm/roma1.in_fn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54",&err) ) == 0, " error in url parse 2");
	if(ret)
		gfal_release_GError(&err);
	fail_if( (ret = gfal_surl_checker("http://google.com",&err ))== 0, " must fail , bad url");
}
END_TEST


START_TEST(test_gfal_full_endpoint_check)
{
	GError* err = NULL;
	int ret = -1;
	fail_if( (ret= gfal_check_fullendpoint_in_surl( "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b-aaba-0e1341f30663",&err)) ," fail, must be a success" );
	if(ret)
		gfal_release_GError(&err);
	fail_if( (ret = gfal_check_fullendpoint_in_surl("srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it", &err)) != REG_NOMATCH, " fail, must be a failure : bad url");
	if(ret)
		gfal_release_GError(&err);
	fail_if( (ret= gfal_check_fullendpoint_in_surl( "srm://lxb5409.cern.ch:8446/srm/managerv2?SFN=/dpm/cern.ch/home/dteam/srmv2_tests/test_lfc_3897",&err)) ," fail, must be a success" );
	if(ret)
		gfal_release_GError(&err);
}
END_TEST	


START_TEST(test_gfal_auto_get_srm_endpoint_full_endpoint)
{
	GError* err = NULL;
	char * endpoint=NULL;
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	fail_unless(handle != NULL, " handle is not properly allocated");
	int ret =-1;
		
	GList* list = g_list_append(NULL,"srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b");
	fail_unless(gfal_auto_get_srm_endpoint(handle, NULL, &proto, list, &err) || err ==NULL, "must fail, einval");	// test the params checker
	g_clear_error(&err);
	fail_unless(gfal_auto_get_srm_endpoint(handle, &endpoint, NULL, list, &err) || err ==NULL, "must fail, einval");
	g_clear_error(&err);
	fail_unless(gfal_auto_get_srm_endpoint(handle, &endpoint, NULL, list, &err) || err ==NULL, "must fail, einval");
	g_clear_error(&err);
	
	fail_if( ret = gfal_auto_get_srm_endpoint(handle, &endpoint, &proto, list, &err) , " must return the correct endpoint");
	fail_if( endpoint == NULL || strstr(endpoint,"httpg://srm-pps:8443/srm/managerv2") == NULL, " must contain the endpoint");
	fail_if(proto != handle->srm_proto_type, " srm must be the default version of srm");
	gfal_handle_freeG(handle);	
}
END_TEST



START_TEST(test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdii)
{
	GError* err = NULL;
	char * endpoint=NULL;
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	fail_unless(handle != NULL, " handle is not properly allocated");
	int ret =-1;
	
	gfal_set_nobdiiG(handle, TRUE);
	fail_unless(handle->no_bdii_check, " nobdii must be true");
		
	GList* list = g_list_append(NULL,"srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b");
	
	fail_if( ret = gfal_auto_get_srm_endpoint(handle, &endpoint, &proto, list, &err) , " must return the correct endpoint");
	fail_if( endpoint == NULL || strstr(endpoint,"httpg://srm-pps:8443/srm/managerv2") == NULL, " must contain the endpoint");
	fail_if(proto != handle->srm_proto_type, " srm must be the default version of srm");
	gfal_handle_freeG(handle);	
}
END_TEST


START_TEST(test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdii_negative)
{
	GError* err = NULL;
	char * endpoint=NULL;
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	fail_unless(handle != NULL, " handle is not properly allocated");
	int ret =-1;
	
	gfal_set_nobdiiG(handle, TRUE);
	fail_unless(handle->no_bdii_check, " nobdii must be true");
		
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54");
	
	fail_unless( ret = gfal_auto_get_srm_endpoint(handle, &endpoint, &proto, list, &err) , " must return error because not a full srm");
	gfal_handle_freeG(handle);	
}
END_TEST


START_TEST(test_gfal_auto_get_srm_endpoint_no_full_with_bdii)
{
	GError* err = NULL;
	char * endpoint=NULL;
	enum gfal_srm_proto proto;
	gfal_handle handle = gfal_initG(&err);
	fail_unless(handle != NULL, " handle is not properly allocated");
	int ret =-1;
	
	fail_if(handle->no_bdii_check, " nobdii must be false");
		
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54");
	
	fail_if( ret = gfal_auto_get_srm_endpoint(handle, &endpoint, &proto, list, &err) , " must return the correct endpoint");
	fail_if( endpoint == NULL || strstr(endpoint,"httpg://") == NULL, " must contain the endpoint");
	fail_if(proto != handle->srm_proto_type, " srm must be the default version of srm");
	gfal_handle_freeG(handle);	
}
END_TEST


START_TEST(test_gfal_get_fullendpoint){
	const char * surl = "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b";
	GError* err=NULL;
	char* endpoint;
	fail_if( (endpoint = gfal_get_fullendpoint(surl,&err)) == NULL || err || strcmp(endpoint,"httpg://srm-pps:8443/srm/managerv2"), " must be successfull");
	fprintf(stderr, " endpoint from surl %s ", endpoint);
	
	const char * surl2 = "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04";	
	fail_if( (endpoint = gfal_get_fullendpoint(surl2,&err)) !=NULL || err==NULL , " must fail");	
	
}
END_TEST
