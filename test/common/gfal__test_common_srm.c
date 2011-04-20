

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
	if(r <0){
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
	g_list_free(list);
	free(endpoint);
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
	g_list_free(list);
	free(endpoint);
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
	g_list_free(list);
	free(endpoint);
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
	fail_if( endpoint == NULL || strcmp(endpoint,"httpg://grid-cert-03.roma1.infn.it:8446/srm/managerv2") != NULL, " must contain the endpoint");
	fail_if(proto != handle->srm_proto_type, " srm must be the default version of srm");
	gfal_handle_freeG(handle);	
	g_list_free(list);
	free(endpoint);
}
END_TEST


START_TEST(test_gfal_get_fullendpoint){
	const char * surl = "srm://srm-pps:8443/srm/managerv2?SFN=/castor/cern.ch/grid/dteam/castordev/test-srm-pps_8443-srm2_d0t1-ed6b7013-5329-4f5b";
	GError* err=NULL;
	char* endpoint;
	fail_if( (endpoint = gfal_get_fullendpoint(surl,&err)) == NULL || err || strcmp(endpoint,"httpg://srm-pps:8443/srm/managerv2"), " must be successfull");
	//fprintf(stderr, " endpoint from surl %s ", endpoint);
	free(endpoint);
	const char * surl2 = "srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04";	
	fail_if( (endpoint = gfal_get_fullendpoint(surl2,&err)) !=NULL || err==NULL , " must fail");	
	free(endpoint);
}
END_TEST


START_TEST(test_gfal_get_hostname_from_surl)
{
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54");	
	GError * tmp_err=NULL;
	char* resu;
	fail_unless( (resu = gfal_get_hostname_from_surl(list->data, &tmp_err)) && !tmp_err, " must be a success");
	if(resu){
		fail_unless( strcmp("grid-cert-03.roma1.infn.it",resu) == 0, " must be the same string");
		free(resu); 
	}
	g_list_free(list);
	list = g_list_append(NULL,"http://google.com/");	
	fail_if( (resu = gfal_get_hostname_from_surl(list->data, &tmp_err)) && !tmp_err, " must be a success");
	g_list_free(list);
}
END_TEST


START_TEST(test_gfal_get_endpoint_and_setype_from_bdii)
{
	char *endpoint;
	enum gfal_srm_proto srm_type;
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54");	
	GError * err= NULL;
	gfal_handle handle  = gfal_initG(&err);
	if(handle == NULL){
		fail(" handle is not properly allocated");
		return;
	}
	
	int ret = gfal_get_endpoint_and_setype_from_bdii(handle, &endpoint, &srm_type, list, &err);
	if(ret){
		fail( " fail, must be a valid return");		
		gfal_release_GError(&err);
	}
	fail_unless( srm_type == PROTO_SRMv2, " must be the default protocol");
	fail_unless( strcmp(endpoint, "httpg://grid-cert-03.roma1.infn.it:8446/srm/managerv2") == 0, "must be this endpoint ");	
	g_list_free(list);
}
END_TEST


START_TEST(test_gfal_select_best_protocol_and_endpoint)
{
	char *endpoint;
	enum gfal_srm_proto srm_type;
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file75715ccc-1c54-4d18-8824-bdd3716a2b54");	
	GError * err= NULL;
	gfal_handle handle  = gfal_initG(&err);
	if(handle == NULL){
		fail(" handle is not properly allocated");
		return;
	}
	gfal_set_default_storageG(handle, PROTO_SRMv2);
	const char* endpoint_list[] = { "everest", "montblanc", NULL};
	const char* se_type_list[] = { "srm_v1", "srm_v2", NULL };
	int ret = gfal_select_best_protocol_and_endpoint(handle, &endpoint, &srm_type, se_type_list, endpoint_list, &err);
	if(ret){
			fail(" must successfull");
			gfal_release_GError(&err);
	}	
	fail_if(strcmp(endpoint,"montblanc")!=0, " reponse not match correctly");
	// try with another version by default
	gfal_set_default_storageG(handle, PROTO_SRM);
	ret = gfal_select_best_protocol_and_endpoint(handle, &endpoint, &srm_type, se_type_list, endpoint_list, &err);
	if(ret){
			fail(" must successfull");
			gfal_release_GError(&err);
	}	
	fail_if(strcmp(endpoint,"everest")!=0);		
	
}
END_TEST


START_TEST(gfal_get_asyncG_empty_req)
{
	GError *err = NULL;
	GList* list = g_list_append(NULL,NULL);
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
		fail("fail to init handle");
		return;
	}
	int ret = gfal_get_asyncG(handle, list, &err);
	fail_unless(ret, " this request must fail, no surls in the list");
	g_list_free(list);
	g_clear_error(&err);
	list = g_list_append(NULL,"google.com");
	list = g_list_append(NULL,"amazon.com");
	ret = gfal_get_asyncG(handle, list, &err);
	fail_unless(ret, " this request must fail, bad surl in the list");
	
	
	
}
END_TEST

START_TEST(gfal_get_asyncG_empty_old_nonexist_surl)
{
	GError *err = NULL;
	GList* list = g_list_append(NULL,NULL);
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
		fail("fail to init handle");
		return;
	}

	list = g_list_append(NULL,"srm://lxdpm103.cern.ch/dpm/cern.ch/home/dteam/generated/2009-09-21/file712de300-7a1e-4c78-90d2-0ba187e5a8da");
	int ret = gfal_get_asyncG(handle, list, &err);
	fail_unless(ret, " this request must fail old non exist surl");
	
	
	
}
END_TEST


START_TEST(test_gfal_get_async_resultsG)
{
	GError *err = NULL;
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file22a56c33-c9b1-44c7-bbc5-a4ff0ee11e32");	
	GList* list_resu;
	GList* list_resu_err;		
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
		fail("fail to init handle");
		return;
	}
	int ret = gfal_get_asyncG(handle, list, &err);
	fail_if(ret <0, " this request must be a success, valid surl");
	if(ret <0){
		gfal_release_GError(&err);
		return;
	}
	ret = gfal_get_async_resultsG(handle, &list_resu, &list_resu_err, &err);
	fprintf(stderr, " result turl : %s", list_resu->data);
	fail_if(ret !=1 , " must return good status");
	fail_if(list_resu_err->data, " must not have error ");
	return;
}
END_TEST

START_TEST(test_gfal_get_async_resultsG_empty)
{
	GError *err = NULL;
	gfal_handle handle = NULL; // srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile002
	GList* list;
	int ret = gfal_get_async_resultsG(handle, &list, NULL, &err);
	fail_unless(ret <0, " must be a failure, handle empty");	
	g_clear_error(&err);
	handle = gfal_initG(&err);
	if(handle == NULL){
		fail("fail to init handle");
		return;
	}
	ret = gfal_get_async_resultsG(handle, &list, NULL, &err);
	fail_unless(ret<0  , " must be a failure, ne get before");	
	g_clear_error(&err);
}
END_TEST


START_TEST(test_full_gfal_get_request)
{
	GError *err = NULL;
	gfal_handle handle = NULL; 
	GList* list = g_list_append(NULL,"srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/2006-07-04/file22a56c33-c9b1-44c7-bbc5-a4ff0ee11e32");
	GList *list_resu, *list_resu_err;
	handle = gfal_initG(&err);
	if(handle == NULL){
		fail("fail to init handle");
		return;
	}		
	int ret = gfal_get_asyncG(handle, list, &err);
	fail_if(ret <0, " this request must be a success, valid surl");
	if(ret <0){
		gfal_release_GError(&err);
		return;
	}	

	ret = gfal_get_async_resultsG(handle, &list_resu, &list_resu_err, &err);
	fail_if(ret !=1 , " must return good status");
	fail_if(list_resu_err->data, " must not have error ");
	fail_if(g_list_length(list_resu) !=1 || g_list_length(list_resu_err) != 1, " bad result size");
	fail_unless(strcmp(list_resu->data, "gsiftp://atlas-storage-02.roma1.infn.it/atlas-storage-02.roma1.infn.it:/data2/dteam/2009-01-28/file22a56c33-c9b1-44c7-bbc5-a4ff0ee11e32.8246597.0") == 0, " is not the good turl");
}
END_TEST

