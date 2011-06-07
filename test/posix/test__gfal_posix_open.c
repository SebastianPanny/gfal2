
/* unit test for posix open func */


#include <check.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>



START_TEST(test_open_posix_all_simple)
{
	int ret = gfal_open(NULL, O_RDONLY, 555);
	if(ret > 0 || gfal_posix_code_error() != EFAULT || errno != EFAULT ){
		fail(" must be a failure, null path", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	
}
END_TEST

START_TEST(test_open_posix_local_simple)
{
	system(TEST_LOCAL_OPEN_CREATE_COMMAND);
	int ret = -1;
	int fd = gfal_open(TEST_LOCAL_OPEN_EXIST, O_RDONLY, 555);
	if(fd <=0 || gfal_posix_code_error() != 0 || errno != 0 ){
		fail(" must be a valid file descriptor %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	ret = gfal_close(fd);
	if(ret !=0 || gfal_posix_code_error() != 0 || errno != 0 ){
		fail(" must be a valid close");
		gfal_posix_release_error();
		return;
	}
	ret = gfal_close(fd);
	if(ret ==0 || gfal_posix_code_error() != EBADF || errno != EBADF){
		fail(" must be an non existant file descriptor  %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;		
	}
	gfal_posix_clear_error();
	fd = gfal_open(TEST_LOCAL_OPEN_NOEXIST, O_RDONLY, 555);
	if(fd >0 || gfal_posix_code_error() != ENOENT || errno != ENOENT ){
		fail(" must be a non existing file %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}		
	gfal_posix_clear_error();
	fd = gfal_open(TEST_LOCAL_OPEN_NOACCESS, O_RDONLY, 555);
	if(fd >0 || gfal_posix_code_error() != EACCES || errno != EACCES ){
		fail("must be a non accessible file %d %d %d", ret, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
}
END_TEST


START_TEST(test_open_posix_lfc_simple)
{

	fail(" not implemented");	
	
}
END_TEST


START_TEST(test_open_posix_srm_simple)
{

	fail(" not implemented");	
	
}
END_TEST
