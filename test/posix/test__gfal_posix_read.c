
/* unit test for posix open func */


#include <cgreen/cgreen.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>






static void test_generic_read_simple(char* url_exist, const char* filename){
	char buff[2048];
	int ret = -1;
	errno=0;
	int fd = gfal_open(url_exist, O_RDONLY, 555);
	assert_true_with_message(fd >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid open %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_check_error();
	
	ret = gfal_read(fd, buff, strlen(TEST_SRM_FILE_CONTENT)+1);
	assert_true_with_message(ret >0 && gfal_posix_code_error()==0 && errno==0, " must be a valid read %d %d %d", ret, gfal_posix_code_error(), errno);
	assert_true_with_message(strcmp(filename, buff)==0, " must be the content of the file");
	gfal_posix_check_error();
	ret = gfal_close(fd);
	assert_true_with_message(fd !=0 && ret==0 && gfal_posix_code_error()==0 && errno==0, " must be a valid close %d %d %d", ret, gfal_posix_code_error(), errno);
	gfal_posix_check_error();	
	ret = gfal_close(fd);
	assert_true_with_message( ret==-1 && gfal_posix_code_error()==EBADF && errno==EBADF, " must be a bad descriptor %d %d %d", ret, gfal_posix_code_error(), errno);

}



void test_read_posix_local_simple()
{
	system(TEST_LOCAL_OPEN_CREATE_COMMAND);

	test_generic_read_simple(TEST_LOCAL_OPEN_EXIST, TEST_LOCAL_READ_CONTENT);

}


void test_read_posix_lfc_simple()
{
	test_generic_read_simple(TEST_LFC_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);

}

void test_read_posix_guid_simple(){
	test_generic_read_simple(TEST_GUID_ONLY_READ_ACCESS, TEST_SRM_FILE_CONTENT);	
	
}






