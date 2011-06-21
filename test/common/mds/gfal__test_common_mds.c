

/* unit test for common_mds*/


#include <cgreen/cgreen.h>
#include <glib.h>
#include <string.h>
#include "gfal_common.h"
#include "mds/gfal_common_mds.h"
#include "mds/gfal_common_mds_layer.h"
#include "../../unit_test_constants.h"
#include "../../mock/gfal_mds_mock_test.h" 



void test_check_bdii_endpoints_srm()
{

	char **se_types=NULL;
	char **se_endpoints=NULL;
	int i1;
	GError * err=NULL;
	int ret=-1;
	char* endpoints[] = { "grid-cert-03.roma1.infn.it",
						NULL };
	char** ptr = endpoints;
	
#if USE_MOCK
	gfal_mds_external_call.sd_get_se_types_and_endpoints = &mds_mock_sd_get_se_types_and_endpoints;
	define_se_endpoints = calloc(sizeof(char*), 4);
	for(i1=0;i1 <3; ++i1)
		define_se_endpoints[i1]= strdup(TEST_MDS_VALID_ENDPOINT_RESU);
	define_se_types= calloc(sizeof(char*), 4);
	char* types[] = { "srm_v1", "srm_v2", "srm_v1"};
	for(i1=0;i1 <3; ++i1)
		define_se_types[i1]= strdup(types[i1]);	
	will_respond(mds_mock_sd_get_se_types_and_endpoints, 0, want_string(host, TEST_MDS_VALID_ENDPOINT_URL), want_non_null(se_types), want_non_null(se_endpoints));
	will_respond(mds_mock_sd_get_se_types_and_endpoints, EINVAL, want_string(host, TEST_MDS_INVALID_ENDPOINT_URL), want_non_null(se_types), want_non_null(se_endpoints));
	always_return(mds_mock_sd_get_se_types_and_endpoints, EFAULT);
#endif	
	
	while(*ptr != NULL){
		se_types=NULL;
		se_endpoints=NULL;
		ret = gfal_mds_get_se_types_and_endpoints (*ptr, &se_types, &se_endpoints, &err);
		assert_true_with_message(ret == 0, " ret of bdii with error %d %d", ret, errno);
		assert_true_with_message(ret == 0 && strings_are_equal(*se_types, "srm_v1") && strings_are_equal(*se_endpoints, TEST_MDS_VALID_ENDPOINT_RESU), " check if result is valid");
		g_strfreev(se_types);
		g_strfreev(se_endpoints);
		ptr++;
	}

	se_types=NULL;
	se_endpoints=NULL;	
	ret = gfal_mds_get_se_types_and_endpoints ("google.com", &se_types, &se_endpoints, &err);		
	assert_true_with_message(ret != 0 &&  err->code == EINVAL , "must fail, invalid url");
	g_clear_error(&err);
	g_strfreev(se_types);
	g_strfreev(se_endpoints);
	gfal_mds_external_call.sd_get_se_types_and_endpoints = &sd_get_se_types_and_endpoints;
}



void gfal__test_get_lfchost_bdii()
{
	GError* tmp_err =NULL;
	errno = 0;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(tmp_err){
		gfal_release_GError(&tmp_err);
		assert_true_with_message(FALSE, "Error while init handle");
		return;		
	}
	char* lfc = gfal_get_lfchost_bdii(handle, &tmp_err);
	if(!lfc){
		gfal_release_GError(&tmp_err);
		assert_true_with_message(FALSE, " must return correct lfc value");
		return;
	}
	//g_printerr(" lfc name : %s ", lfc);
	free(lfc);
	gfal_handle_freeG(handle);	
}



void gfal__test_get_lfchost_bdii_with_nobdii()
{
	GError* tmp_err =NULL;
	errno = 0;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(tmp_err){
		gfal_release_GError(&tmp_err);
		assert_true_with_message(FALSE, "Error while init handle");
		return;		
	}
	gfal_set_nobdiiG(handle, TRUE);
	char* lfc = gfal_get_lfchost_bdii(handle, &tmp_err);
	if(lfc || tmp_err->code!= EPROTONOSUPPORT ){
		gfal_release_GError(&tmp_err);
		assert_true_with_message(FALSE, " must return an error, nobdii option checked");
		free(lfc);
		return;
	}
	//g_printerr(" lfc name : %s ", lfc);
	free(lfc);	
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
	
}
