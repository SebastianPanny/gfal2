
/* unit test for posix access func */


#include <cgreen/cgreen.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "../unit_test_constants.h"
#include "../../src/posix/gfal_posix_internal.h"

#include "gfal_posix_api.h"


#define TEST_GFAL_LOCAL_FILE_CHMOD_READ  "/tmp/testchmodread0011"


void test__gfal_posix_chmod_read_lfn(){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	mock_catalog(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;
	will_respond(lfc_mock_chmod, 0, want_string(path, TEST_LFC_MOD_READ_FILE+4), want(mode, 0));
	will_respond(lfc_mock_access, EACCES, want_string(path, TEST_LFC_MOD_READ_FILE+4), want(mode, R_OK));
	will_respond(lfc_mock_chmod, ENOENT, want_string(path, TEST_LFC_MOD_UNEXIST_FILE+4), want(mode, 0));
	will_respond(lfc_mock_chmod, 0, want_string(path, TEST_LFC_MOD_READ_FILE+4), want(mode, 0555));
	will_respond(lfc_mock_access, 0, want_string(path, TEST_LFC_MOD_READ_FILE+4), want(mode, R_OK));
	always_return(lfc_mock_chmod, EINVAL);
#endif
	int res = gfal_chmod(TEST_LFC_MOD_READ_FILE, 0);	// reduce the right to the file to 0
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success %d %d %d",res, errno, gfal_posix_code_error());
 	gfal_posix_check_error();
	res = gfal_access(TEST_LFC_MOD_READ_FILE, R_OK);
	assert_true_with_message( res == -1 && errno == EACCES && gfal_posix_code_error() == EACCES, "must be a non accessible file");		
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_LFC_MOD_UNEXIST_FILE, 0);
	assert_true_with_message( res == -1 && errno == ENOENT && gfal_posix_code_error() == ENOENT, "must be a non-existing file");		
	gfal_posix_clear_error();
	res = gfal_chmod("google.com", 0);
	assert_true_with_message( res == -1 && errno == EPROTONOSUPPORT && gfal_posix_code_error() == EPROTONOSUPPORT, "must be a bad url");		
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_LFC_MOD_READ_FILE, 0555);		// reset the right of the file
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success");
 	gfal_posix_check_error();
	res = gfal_access(TEST_LFC_MOD_READ_FILE, R_OK);
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success");
 	gfal_posix_check_error();

}


void test__gfal_posix_chmod_read_guid(){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	mock_catalog(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;
	char* tab_res1[]= { "/dteam/test0011", NULL };
	define_mock_linkinfos(1,tab_res1);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GUID_MODE_READ_FILE+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_chmod, 0, want_string(path, "/dteam/test0011"), want(mode, 0));

#endif	
	int res = gfal_chmod(TEST_GUID_MODE_READ_FILE, 0);	// reduce the right to the file to 0
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success %d %d %d",res, errno, gfal_posix_code_error());
 	gfal_posix_check_error();
#if USE_MOCK
	define_mock_linkinfos(1,tab_res1);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GUID_MODE_READ_FILE+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_access, EACCES, want_string(path, "/dteam/test0011"), want(mode, R_OK));
#endif
	res = gfal_access(TEST_GUID_MODE_READ_FILE, R_OK);
	assert_true_with_message( res == -1 && errno == EACCES && gfal_posix_code_error() == EACCES, "must be a non accessible file");		
	gfal_posix_clear_error();
#if USE_MOCK
	define_mock_linkinfos(1,tab_res1);
	will_respond(lfc_mock_getlinks, ENOENT, want_string(guid, TEST_GUID_NOEXIST_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GUID_MODE_READ_FILE+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_chmod, 0, want_string(path, "/dteam/test0011"), want(mode, 0555));
#endif

	res = gfal_chmod(TEST_GUID_NOEXIST_ACCESS, 0);
	assert_true_with_message( res == -1 && errno == ENOENT && gfal_posix_code_error() == ENOENT, "must be a non-existing file");		
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_GUID_MODE_READ_FILE, 0555);		// reset the right of the file
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success");
 	gfal_posix_check_error();
#if USE_MOCK
 	define_mock_linkinfos(1,tab_res1);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GUID_MODE_READ_FILE+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	will_respond(lfc_mock_access, 0, want_string(path, "/dteam/test0011"), want(mode, R_OK));
	always_return(lfc_mock_chmod, EINVAL);
#endif
	res = gfal_access(TEST_GUID_MODE_READ_FILE, R_OK);
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success");
 	gfal_posix_check_error();

}


void test__gfal_posix_chmod_read_local(){
	// create local file
	const char * msg = "hello";
	char nfile[500];
	strcpy(nfile, "file://");
	FILE* f = fopen(TEST_GFAL_LOCAL_FILE_CHMOD_READ, "w+");
	if(f == NULL){
		assert_true_with_message(FALSE, " file must be created");
		return;
	}
	fwrite(msg, sizeof(char), 5, f);
	fclose(f);
	errno = 0;
	strcat(nfile,TEST_GFAL_LOCAL_FILE_CHMOD_READ);
	int res= gfal_chmod(nfile, 0);
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success %d %d %d",res, errno, gfal_posix_code_error());		
	gfal_posix_clear_error();
	
	res = gfal_access(nfile, R_OK); // must not be accessible
	assert_true_with_message( res == -1 && errno == EACCES && gfal_posix_code_error() == EACCES, "must be a non accessible file");		
	gfal_posix_clear_error();
	
	res= gfal_chmod(nfile, 0777);
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a valid re-modification");		
	gfal_posix_clear_error();
	
	res = gfal_access(nfile, R_OK); // must be accessible
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a re-modification");		
	gfal_posix_clear_error();
}



void test__gfal_posix_chmod_write_lfn(){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	mock_catalog(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;
	will_respond(lfc_mock_chmod, 0, want_string(path, TEST_LFC_MOD_WRITE_FILE+4), want(mode, 0));
	will_respond(lfc_mock_access, EACCES, want_string(path, TEST_LFC_MOD_WRITE_FILE+4), want(mode, W_OK));
	will_respond(lfc_mock_chmod, ENOENT, want_string(path, TEST_LFC_MOD_UNEXIST_FILE+4), want(mode, 0));
	will_respond(lfc_mock_chmod, 0, want_string(path, TEST_LFC_MOD_WRITE_FILE+4), want(mode, 0666));
	will_respond(lfc_mock_access, 0, want_string(path, TEST_LFC_MOD_WRITE_FILE+4), want(mode, W_OK));
	always_return(lfc_mock_chmod, EINVAL);
#endif	
	int res = gfal_chmod(TEST_LFC_MOD_WRITE_FILE, 0);	// reduce the right to the file to 0
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success %d %d %d",res, errno, gfal_posix_code_error());
 	gfal_posix_check_error();
	res = gfal_access(TEST_LFC_MOD_WRITE_FILE, W_OK);
	assert_true_with_message( res == -1 && errno == EACCES && gfal_posix_code_error() == EACCES, "must be a non accessible file");		
	gfal_posix_clear_error();
		
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_LFC_MOD_UNEXIST_FILE, 0);
	assert_true_with_message( res == -1 && errno == ENOENT && gfal_posix_code_error() == ENOENT, "must be a non accessible file");		
	gfal_posix_clear_error();


	res = gfal_chmod("google.com", 0);
	assert_true_with_message( res == -1 && errno == EPROTONOSUPPORT && gfal_posix_code_error() == EPROTONOSUPPORT, "must be a non accessible file");		
	gfal_posix_clear_error();
	
	res = gfal_chmod(TEST_LFC_MOD_WRITE_FILE, 0666);		// reset the right of the file
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success %d %d %d",res, errno, gfal_posix_code_error());
 	gfal_posix_check_error();
 	
	res = gfal_access(TEST_LFC_MOD_WRITE_FILE, W_OK);
	assert_true_with_message( res == 0 && errno == 0 && gfal_posix_code_error() == 0, "must be a success %d %d %d",res, errno, gfal_posix_code_error());
 	gfal_posix_check_error();

}
