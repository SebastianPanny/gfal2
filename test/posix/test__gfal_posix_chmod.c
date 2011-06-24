
/* unit test for posix access func */


#include <cgreen/cgreen.h>
#include <unistd.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>

#define TEST_GFAL_LOCAL_FILE_CHMOD_READ  "/tmp/testchmodread0011"


void test__gfal_posix_chmod_read_lfn(){
	
	int res = gfal_chmod(TEST_LFC_MOD_READ_FILE, 0);	// reduce the right to the file to 0
	if( res != 0){
		assert_true_with_message(FALSE, " must be a success");
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(TEST_LFC_MOD_READ_FILE, R_OK);
	if(res ==0 || errno!=EACCES || gfal_posix_code_error() != EACCES){	
		assert_true_with_message(FALSE, " must be a failure : read right is removed %d %d %d ",res,errno, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
		
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_LFC_MOD_UNEXIST_FILE, 0);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT ){
		assert_true_with_message(FALSE, " must report an error");
		return;
	}
	gfal_posix_clear_error();
	res = gfal_chmod("google.com", 0);
	if( res == 0 || gfal_posix_code_error() != EPROTONOSUPPORT || errno != EPROTONOSUPPORT ){
		assert_true_with_message(FALSE, " must be a incorrect protocole");
		return;
	}
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_LFC_MOD_READ_FILE, 0555);		// reset the right of the file
	if(res !=0 ){
		gfal_posix_release_error();
		assert_true_with_message(FALSE, " must report an error");
		return;
	}
	res = gfal_access(TEST_LFC_MOD_READ_FILE, R_OK);
	if(res != 0){
		gfal_posix_release_error();
		assert_true_with_message(FALSE, " must be a valid access to the file with new right %d %d",errno, gfal_posix_code_error());
		return;
	}

}


void test__gfal_posix_chmod_read_guid(){
	
	int res = gfal_chmod(TEST_GUID_MODE_READ_FILE, 0);	// reduce the right to the file to 0
	if( res != 0){
		assert_true_with_message(FALSE, " must be a success");
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(TEST_GUID_MODE_READ_FILE, R_OK);
	if(res ==0 || errno!=EACCES || gfal_posix_code_error() != EACCES){	
		assert_true_with_message(FALSE, " must be a failure : read right is removed %d %d %d ",res,errno, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
		
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_GUID_NOEXIST_ACCESS, 0);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT ){
		assert_true_with_message(FALSE, " must report an error");
		return;
	}
	gfal_posix_clear_error();
	res = gfal_chmod("google.com", 0);
	if( res == 0 || gfal_posix_code_error() != EPROTONOSUPPORT || errno != EPROTONOSUPPORT ){
		assert_true_with_message(FALSE, " must be a incorrect protocole");
		return;
	}
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_GUID_MODE_READ_FILE, 0555);		// reset the right of the file
	if(res !=0 ){
		gfal_posix_release_error();
		assert_true_with_message(FALSE, " must report an error");
		return;
	}
	res = gfal_access(TEST_GUID_MODE_READ_FILE, R_OK);
	if(res != 0){
		gfal_posix_release_error();
		assert_true_with_message(FALSE, " must be a valid access to the file with new right %d %d",errno, gfal_posix_code_error());
		return;
	}

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
	
	strcat(nfile,TEST_GFAL_LOCAL_FILE_CHMOD_READ);
	int res= gfal_chmod(nfile, 0);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid chmod 0 on the file : %s %d %d %d", nfile, res, errno, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	res = gfal_access(nfile, R_OK); // must not be accessible
	if(res ==0 || errno!=EACCES || gfal_posix_code_error() != EACCES){	
		assert_true_with_message(FALSE, " must be a failure : read right is removed %d %d %d ",res,errno, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	res= gfal_chmod(nfile, 0777);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid chmod 555 on the file : %s %d %d", nfile, errno, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(nfile, R_OK); // must be accessible
	if(res != 0){	
		assert_true_with_message(FALSE, " must be a failure : must be a readable file");
		gfal_posix_release_error();
		return;
	}	
}



void test__gfal_posix_chmod_write_lfn(){
	
	int res = gfal_chmod(TEST_LFC_MOD_WRITE_FILE, 0);	// reduce the right to the file to 0
	if( res != 0){
		assert_true_with_message(FALSE, " must be a success");
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(TEST_LFC_MOD_WRITE_FILE, W_OK);
	if(res ==0 || errno!=EACCES || gfal_posix_code_error() != EACCES){	
		assert_true_with_message(FALSE, " must be a failure : write right is removed %d %d %d ",res,errno, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
		
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_LFC_MOD_UNEXIST_FILE, 0);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT ){
		assert_true_with_message(FALSE, " must report an error");
		return;
	}
	gfal_posix_clear_error();
	res = gfal_chmod("google.com", 0);
	if( res == 0 || gfal_posix_code_error() != EPROTONOSUPPORT || errno != EPROTONOSUPPORT ){
		assert_true_with_message(FALSE, " must be a incorrect protocole");
		return;
	}
	gfal_posix_clear_error();
	res = gfal_chmod(TEST_LFC_MOD_WRITE_FILE, 0666);		// reset the right of the file
	if(res !=0 ){
		gfal_posix_release_error();
		assert_true_with_message(FALSE, " must report an error");
		return;
	}
	res = gfal_access(TEST_LFC_MOD_WRITE_FILE, W_OK);
	if(res != 0){
		gfal_posix_release_error();
		assert_true_with_message(FALSE, " must be a valid write access to the file with new rights %d %d",errno, gfal_posix_code_error());
		return;
	}

}
