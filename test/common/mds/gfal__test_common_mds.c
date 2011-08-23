

/* unit test for common_mds*/


#include <cgreen/cgreen.h>
#include <glib.h>
#include <string.h>
#include "mds/gfal_common_mds.h"
#include "mds/gfal_common_mds_layer.h"
#include "../../unit_test_constants.h"
#include "../../mock/gfal_mds_mock_test.h" 



void test_check_bdii_endpoints_srm()
{

	char **se_types=NULL;
	char **se_endpoints=NULL;

	GError * err=NULL;
	int ret=-1;
	char* endpoints[] = { TEST_MDS_VALID_ENDPOINT_URL,
						NULL };
	char** ptr = endpoints;
	
#if USE_MOCK
	gfal_mds_external_call.sd_get_se_types_and_endpoints = &mds_mock_sd_get_se_types_and_endpoints;
	define_mock_endpoints(TEST_MDS_VALID_ENDPOINT_RESU);

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
		g_clear_error(&err);
		ptr++;
	}

	se_types=NULL;
	se_endpoints=NULL;	
	ret = gfal_mds_get_se_types_and_endpoints (TEST_MDS_INVALID_ENDPOINT_URL, &se_types, &se_endpoints, &err);		
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
	assert_true_with_message(handle !=NULL && tmp_err == NULL, "Error while init handle");
	if(tmp_err)
		return;		
	
#if USE_MOCK
	gfal_mds_external_call.sd_get_lfc_endpoint = &mds_mock_sd_get_lfc_endpoint;
	define_lfc_endpoint = strdup("avalid.lfc.value.fr");	
	will_respond(mds_mock_sd_get_lfc_endpoint, 0, want_non_null(lfc_endpoint));
	always_return(mds_mock_sd_get_lfc_endpoint, EFAULT);
#endif
	char* lfc = gfal_get_lfchost_bdii(handle, &tmp_err);
	assert_true_with_message(lfc!=NULL && strings_are_equal(lfc, "avalid.lfc.value.fr"), "must return correct lfc value");
	gfal_check_GError(&tmp_err);
	lfc = gfal_get_lfchost_bdii(handle, &tmp_err);
	assert_true_with_message(lfc== NULL, "must return correct lfc value");
	g_clear_error(&tmp_err);
	free(lfc);
	gfal_handle_freeG(handle);	
	gfal_mds_external_call.sd_get_lfc_endpoint = &sd_get_lfc_endpoint;
}



void gfal__test_get_lfchost_bdii_with_nobdii()
{
	GError* tmp_err =NULL;
	errno = 0;
	gfal_handle handle = gfal_initG(&tmp_err);
	assert_true_with_message(tmp_err==NULL, "Error while init handle");	
	if(tmp_err)
		return;
	gfal_set_nobdiiG(handle, TRUE);	
#if USE_MOCK
	gfal_mds_external_call.sd_get_lfc_endpoint = &mds_mock_sd_get_lfc_endpoint;
	always_return(mds_mock_sd_get_lfc_endpoint, EFAULT);
#endif

	char* lfc = gfal_get_lfchost_bdii(handle, &tmp_err); // No bdii connected
	assert_true_with_message(lfc == NULL && tmp_err != NULL && tmp_err->code== EPROTONOSUPPORT, " must return an error, nobdii option checked");

	//g_printerr(" lfc name : %s ", lfc);
	free(lfc);	
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
	gfal_mds_external_call.sd_get_lfc_endpoint = &sd_get_lfc_endpoint;
}
