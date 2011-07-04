
/* unit test for posix open func */


#include <cgreen/cgreen.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>
#include "../mock/gfal_lfc_mock_test.h"
#include "../mock/gfal_srm_mock_test.h"
#include "../mock/gfal_mds_mock_test.h"
#include "../mock/gfal_voms_mock_test.h"
#include "../../src/posix/gfal_posix_internal.h"

void test_mock_lfc_open_valid(const char* lfc_url){ 
#if USE_MOCK
	test_rfio_mock_all();
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;	
	char* tab[]= { TEST_SRM_VALID_SURL_EXAMPLE1, NULL };	
	char* tab_turl[] = { TEST_SRM_TURL_EXAMPLE1, NULL };
	int res[] = { 0, 0 };
	define_mock_endpoints(TEST_SRM_DPM_FULLENDPOINT_URL); // mock the mds for the srm endpoitn resolution
	will_respond(mds_mock_sd_get_se_types_and_endpoints, 0, want_string(host, TEST_SRM_DPM_CORE_URL), want_non_null(se_types), want_non_null(se_endpoints));
	define_mock_filereplica(1, tab);
	will_respond(lfc_mock_getreplica, 0, want_string(path, lfc_url+4), want_non_null(nbentries), want_non_null(rep_entries));	
	will_respond(srm_mock_srm_context_init, 0, want_non_null(context), want_string(srm_endpoint, TEST_SRM_DPM_FULLENDPOINT_URL));
	define_mock_srmv2_pinfilestatus(1, tab, NULL, tab_turl, res);
	will_respond(srm_mock_srm_prepare_to_get, 1, want_non_null(context), want_non_null(input), want_non_null(output));

	will_respond(rfio_mock_open, 15, want_non_null(path));
	will_respond(rfio_mock_close, 0, want(fd, 15));
#endif
}

void test_mock_lfc_open_enoent(const char* lfc_url){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;	
	will_respond(lfc_mock_getreplica, ENOENT, want_string(path, lfc_url+4), want_non_null(nbentries), want_non_null(rep_entries));	
	
#endif	
}

void test_mock_lfc_open_eacces(const char* lfc_url){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	setup_mock_srm();
	if( gfal_check_GError(&mock_err))
		return;	
	will_respond(lfc_mock_getreplica, EACCES, want_string(path, lfc_url+4), want_non_null(nbentries), want_non_null(rep_entries));	
	
#endif	
}


void test_open_posix_all_simple()
{
	int ret = gfal_open(NULL, O_RDONLY, 555);
	assert_true_with_message( ret < 0 && gfal_posix_code_error() == EFAULT && errno==EFAULT, " must be a EFAULT response");
	gfal_posix_clear_error();
	
}

static void test_generic_open_simple(char* url_exist, char* url_noent, char* url_noaccess){
	int ret = -1;
	int fd = gfal_open(url_exist, O_RDONLY, 555);
	assert_true_with_message(fd >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid open %d %d %d", fd, gfal_posix_code_error(), errno);
	gfal_posix_check_error();
	ret = gfal_close(fd);
	assert_true_with_message(fd !=0 && ret==0 && gfal_posix_code_error()==0 && errno==0, " must be a valid close %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_check_error();	
	ret = gfal_close(fd);
	assert_true_with_message( ret==-1 && gfal_posix_code_error()==EBADF && errno==EBADF, " must be a bad descriptor %d %d %d", ret, gfal_posix_code_error(), errno);

	gfal_posix_clear_error();
	fd = gfal_open(url_noent, O_RDONLY, 555);
	assert_true_with_message( fd <=0 && gfal_posix_code_error()==ENOENT && errno==ENOENT, " must be a non existing file %d %d %d", ret, gfal_posix_code_error(), errno);
	
		
	gfal_posix_clear_error();
	fd = gfal_open(url_noaccess, O_RDONLY, 555);
	assert_true_with_message( fd <=0 && gfal_posix_code_error()==EACCES && errno==EACCES, " must be a non accessible file %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_clear_error();		
	
}


void test_open_posix_local_simple()
{
	system(TEST_LOCAL_OPEN_CREATE_COMMAND);
	
	test_generic_open_simple(TEST_LOCAL_OPEN_EXIST, TEST_LOCAL_OPEN_NOEXIST, TEST_LOCAL_OPEN_NOACCESS);

}



void test_open_posix_lfc_simple()
{
	test_mock_lfc_open_valid(TEST_LFC_OPEN_EXIST);
	test_mock_lfc_open_enoent(TEST_LFC_OPEN_NOEXIST);
	test_mock_lfc_open_eacces(TEST_LFC_OPEN_NOACCESS);
	test_generic_open_simple(TEST_LFC_OPEN_EXIST, TEST_LFC_OPEN_NOEXIST, TEST_LFC_OPEN_NOACCESS);

}



void test_open_posix_srm_simple()
{

	int ret = -1;
	int fd = gfal_open(TEST_SRM_OPEN_EXIST, O_RDONLY, 555);
	if(fd <=0 || gfal_posix_code_error() != 0 || errno != 0 ){
		assert_true_with_message(FALSE, " must be a valid file descriptor %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	ret = gfal_close(fd);
	if(ret !=0 || gfal_posix_code_error() != 0 || errno != 0 ){
		assert_true_with_message(FALSE, " must be a valid close");
		gfal_posix_release_error();
		return;
	}
	ret = gfal_close(fd);
	if(ret ==0 || gfal_posix_code_error() != EBADF || errno != EBADF){
		assert_true_with_message(FALSE, " must be an non existant file descriptor  %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;		
	}
	gfal_posix_clear_error();
	fd = gfal_open(TEST_SRM_OPEN_NOEXIST, O_RDONLY, 555);
	if(fd >0 || gfal_posix_code_error() != ENOENT || errno != ENOENT ){
		assert_true_with_message(FALSE, " must be a non existing file %d %d %d", fd, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}		
	gfal_posix_clear_error();
	
}



void test_open_posix_guid_simple()
{
	int ret = -1;
	int fd = gfal_open(TEST_GUID_OPEN_EXIST, O_RDONLY, 555);
	if(fd <=0 || gfal_posix_code_error() != 0 || errno != 0 ){
		assert_true_with_message(FALSE, " must be a valid file descriptor %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	ret = gfal_close(fd);
	if(ret !=0 || gfal_posix_code_error() != 0 || errno != 0 ){
		assert_true_with_message(FALSE, " must be a valid close");
		gfal_posix_release_error();
		return;
	}
	ret = gfal_close(fd);
	if(ret ==0 || gfal_posix_code_error() != EBADF || errno != EBADF){
		assert_true_with_message(FALSE, " must be an non existant file descriptor  %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;		
	}
	gfal_posix_clear_error();
	fd = gfal_open(TEST_GUID_OPEN_NONEXIST, O_RDONLY, 555);
	if(fd >0 || gfal_posix_code_error() != ENOENT || errno != ENOENT ){
		assert_true_with_message(FALSE, " must be a non existing file %d %d %d", fd, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}		
	gfal_posix_clear_error();
}
