
/* unit test for posix open func */


#include <cgreen/cgreen.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>



void test_open_posix_all_simple()
{
	int ret = gfal_open(NULL, O_RDONLY, 555);
	assert_true_with_message( ret < 0 && gfal_posix_code_error() == EFAULT && errno==EFAULT, " must be a EFAULT response");
	gfal_posix_clear_error();
	
}


void test_open_posix_local_simple()
{
	system(TEST_LOCAL_OPEN_CREATE_COMMAND);
	int ret = -1;
	int fd = gfal_open(TEST_LOCAL_OPEN_EXIST, O_RDONLY, 555);
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
	fd = gfal_open(TEST_LOCAL_OPEN_NOEXIST, O_RDONLY, 555);
	if(fd >0 || gfal_posix_code_error() != ENOENT || errno != ENOENT ){
		assert_true_with_message(FALSE, " must be a non existing file %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}		
	gfal_posix_clear_error();
	fd = gfal_open(TEST_LOCAL_OPEN_NOACCESS, O_RDONLY, 555);
	if(fd >0 || gfal_posix_code_error() != EACCES || errno != EACCES ){
		assert_true_with_message(FALSE, "must be a non accessible file %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();	
}



void test_open_posix_lfc_simple()
{
	int ret = -1;
	int fd = gfal_open(TEST_LFC_OPEN_EXIST, O_RDONLY, 555);
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
	fd = gfal_open(TEST_LFC_OPEN_NOEXIST, O_RDONLY, 555);
	if(fd >0 || gfal_posix_code_error() != ENOENT || errno != ENOENT ){
		assert_true_with_message(FALSE, " must be a non existing file %d %d %d", fd, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}		
	gfal_posix_clear_error();
	fd = gfal_open(TEST_LFC_OPEN_NOACCESS, O_RDONLY, 555);
	if(fd >0 || gfal_posix_code_error() != EACCES || errno != EACCES ){
		assert_true_with_message(FALSE, "must be a non accessible file %d %d %d", fd, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
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
