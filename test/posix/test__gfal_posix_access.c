
/* unit test for posix access func */


#include <cgreen/cgreen.h>
#include <errno.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include "lfc/lfc_ifce_ng.h"
#include "../mock/gfal_lfc_mock_test.h"


void test_access_posix_guid_exist()
{
	int i1;
	// test exist guid
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	mock_catalog(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	define_linkinfos= calloc(sizeof(struct lfc_linkinfo),3);
	define_numberlinkinfos=3;
	for(i1=0; i1< define_numberlinkinfos; ++i1)
		g_strlcpy(define_linkinfos[i1].path, "/dteam/test/imagine", 2048);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GUID_VALID_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_access, 0, want_string(path, "/dteam/test/imagine"), want(mode, F_OK));
	will_respond(lfc_mock_getlinks, ENOENT, want_string(guid, TEST_GUID_NOEXIST_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));		
	always_return(lfc_mock_getlinks, EINVAL);
#endif
	int ret;
	gfal_posix_clear_error();
	errno =0;
	ret = gfal_access(TEST_GUID_VALID_ACCESS, F_OK);
	assert_true_with_message(ret==0 && gfal_posix_code_error()==0 && errno == 0, " must be a valid access to the guid %d %d %d",ret, errno, gfal_posix_code_error());
	gfal_posix_check_error();
	gfal_posix_clear_error();
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, F_OK);
	assert_true_with_message(ret == -1 && errno == ENOENT, "must be a non exist guid ");
	gfal_posix_clear_error();
	ret = gfal_access("google.com", F_OK);
	assert_true_with_message(ret == -1 && errno == EPROTONOSUPPORT && gfal_posix_code_error() == EPROTONOSUPPORT, " must be a syntax error");
	gfal_posix_clear_error();

}



void test_access_posix_guid_read()
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_GUID_ONLY_READ_ACCESS, R_OK);
	assert_true_with_message(ret==0 && gfal_posix_code_error()==0 && errno == 0, " must be a valid access to the guid %d %d %d",ret, errno, gfal_posix_code_error());
	gfal_posix_check_error();
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, R_OK);
	assert_true_with_message(ret == -1 && gfal_posix_code_error() == ENOENT && errno == ENOENT, " must be a non existing guid %d %d %d",ret, errno, gfal_posix_code_error());
	gfal_posix_clear_error();
	ret = gfal_access(TEST_GUID_NO_READ_ACCESS, R_OK);
	assert_true_with_message( ret == -1 && errno == EACCES && gfal_posix_code_error() == EACCES, "must be an unaccessible file %d %d %d",ret, errno, gfal_posix_code_error());
	gfal_posix_clear_error();
	
}



void test_access_posix_guid_write()
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_GUID_WRITE_ACCESS, W_OK);
	if(ret != 0){
		assert_true_with_message(FALSE, " must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}	
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, W_OK);
	if(ret >=0 || errno != ENOENT){
		assert_true_with_message(FALSE, "must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_GUID_NO_WRITE_ACCESS, W_OK);
	if(ret >=0 || errno != EACCES){
		assert_true_with_message(FALSE, "must be an non-writable file %s", strerror(errno));
		gfal_posix_release_error();
		return;		
	}	
	gfal_posix_clear_error();	
}



void test_access_posix_lfn_exist()
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_LFC_ONLY_READ_ACCESS, F_OK);
	if(ret != 0){
		assert_true_with_message(FALSE, " must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_LFC_NOEXIST_ACCESS, F_OK);
	if(ret >=0 || errno != ENOENT){
		assert_true_with_message(FALSE, "must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access("google.com", F_OK);
	if(ret >=0 || errno != EPROTONOSUPPORT){
		assert_true_with_message(FALSE, " must be a syntax error %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();
}



void test_access_posix_lfn_read()
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_LFC_ONLY_READ_ACCESS, R_OK);
	if(ret != 0){
		assert_true_with_message(FALSE, " must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_LFC_NOEXIST_ACCESS, R_OK);
	if(ret >=0 || errno != ENOENT){
		assert_true_with_message(FALSE, "must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_LFC_NO_READ_ACCESS, R_OK);
	if(ret >= 0 || errno != EACCES){
		assert_true_with_message(FALSE, " must be a non-readable %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();	
	
}



void test_access_posix_lfn_write()
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_LFC_WRITE_ACCESS, W_OK);
	if(ret != 0){
		assert_true_with_message(FALSE, " must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_LFC_NOEXIST_ACCESS, W_OK);
	if(ret >=0 || errno != ENOENT){
		assert_true_with_message(FALSE, "must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_LFC_NO_WRITE_ACCESS, W_OK);
	if(ret >=0 || errno != EACCES){
		assert_true_with_message(FALSE, " must be a non-readable %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();	
	
}




void test_access_posix_srm_exist()
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_SRM_ONLY_READ_ACCESS, F_OK);
	if(ret != 0){
		assert_true_with_message(FALSE, " must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_SRM_NOEXIST_ACCESS, F_OK);
	if(ret >=0 || errno != ENOENT){
		assert_true_with_message(FALSE, "must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access("google.com", F_OK);
	if(ret >=0 || errno != EPROTONOSUPPORT){
		assert_true_with_message(FALSE, " must be a syntax error %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();
}




void test_access_posix_srm_read()
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_SRM_ONLY_READ_ACCESS, R_OK);
	if(ret != 0){
		assert_true_with_message(FALSE, " must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_SRM_NOEXIST_ACCESS, R_OK);
	if(ret >=0 || errno != ENOENT){
		assert_true_with_message(FALSE, "must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_SRM_NO_READ_ACCESS, R_OK);
	if(ret >=0 || errno != EACCES){
		assert_true_with_message(FALSE, " must be an invalid read access error %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();
}



void test_access_posix_srm_write()
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_SRM_WRITE_ACCESS, W_OK);
	if(ret != 0){
		assert_true_with_message(FALSE, " must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_SRM_NOEXIST_ACCESS, W_OK);
	if(ret >=0  || errno != ENOENT){
		assert_true_with_message(FALSE, "must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_SRM_NO_WRITE_ACCESS, W_OK);
	if(ret >=0 || errno != EACCES){
		assert_true_with_message(FALSE, " must be an invalid write access %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();
}
