
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


void test__gfal_posix_stat_lfc()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_stat(TEST_GFAL_LFC_FILE_STAT_OK, &buff);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid stat");
		gfal_posix_release_error();
		return;
	}
	
	if(buff.st_mode != TEST_GFAL_LFC_FILE_STAT_MODE_VALUE 
		|| buff.st_uid != TEST_GFAL_LFC_FILE_STAT_UID_VALUE
		|| buff.st_gid != TEST_GFAL_LFC_FILE_STAT_GID_VALUE
		 || buff.st_size != TEST_GFAL_LFC_FILE_STAT_SIZE_VALUE){
		assert_true_with_message(FALSE, " this is not the correct value for the lfc stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
		//g_printerr(" %o %o ", TEST_GFAL_LFC_FILE_STAT_MODE_VALUE , buff.st_mode);
		gfal_posix_release_error();
		return;			
	}
	
	gfal_posix_clear_error();
	res = gfal_stat(TEST_GFAL_LFC_FILE_STAT_NONEXIST, &buff);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT){
		assert_true_with_message(FALSE, " must be a invalid stat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}





void test__gfal_posix_stat_guid()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_stat(TEST_GFAL_GUID_FILE_STAT_OK, &buff);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid stat");
		gfal_posix_release_error();
		return;
	}
	
	if(buff.st_mode != TEST_GFAL_LFC_FILE_STAT_MODE_VALUE 
		|| buff.st_uid != TEST_GFAL_LFC_FILE_STAT_UID_VALUE
		|| buff.st_gid != TEST_GFAL_LFC_FILE_STAT_GID_VALUE
		 || buff.st_size != TEST_GFAL_LFC_FILE_STAT_SIZE_VALUE){
		assert_true_with_message(FALSE, " this is not the correct value for the lfc stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
		gfal_posix_release_error();
		return;			
	}
	
	res = gfal_stat(TEST_GUID_NOEXIST_ACCESS, &buff);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT){
		assert_true_with_message(FALSE, " must be a invalid stat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();		
}



void test__gfal_posix_stat_local()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	
	// create a fil of the given size
	system(TEST_GFAL_LOCAL_STAT_COMMAND);
	
	int res = gfal_stat(TEST_GFAL_LOCAL_STAT_OK, &buff);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid stat");
		gfal_posix_release_error();
		return;
	}
	
	if(buff.st_mode != TEST_GFAL_LOCAL_FILE_STAT_MODE_VALUE 
		 || buff.st_size != TEST_GFAL_LOCAL_FILE_STAT_SIZE_VALUE){
		assert_true_with_message(FALSE, " this is not the correct value for the local stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
		gfal_posix_release_error();
		return;			
	}
	
	res = gfal_stat(TEST_GFAL_LOCAL_STAT_NONEXIST, &buff);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT){
		assert_true_with_message(FALSE, " must be a invalid stat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();		
}



void test__gfal_posix_stat_srm()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	
	
	int res = gfal_stat(TEST_GFAL_SRM_FILE_STAT_OK, &buff);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid stat");
		gfal_posix_release_error();
		return;
	}
	
	if(buff.st_mode != TEST_GFAL_SRM_FILE_STAT_MODE_VALUE 
		|| buff.st_uid != TEST_GFAL_SRM_FILE_STAT_UID_VALUE
		|| buff.st_gid != TEST_GFAL_SRM_FILE_STAT_GID_VALUE
		 || buff.st_size != TEST_GFAL_SRM_FILE_STAT_SIZE_VALUE){
		assert_true_with_message(FALSE, " this is not the correct value for the srm stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
		gfal_posix_release_error();
		return;			
	}	

	
	res = gfal_stat(TEST_GFAL_LOCAL_STAT_NONEXIST, &buff);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT){
		assert_true_with_message(FALSE, " must be a invalid stat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();	
	
}




void test__gfal_posix_lstat_lfc()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_lstat(TEST_GFAL_LFC_FILE_STAT_OK, &buff);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid lstat");
		gfal_posix_release_error();
		return;
	}
	
	if(buff.st_mode != TEST_GFAL_LFC_FILE_STAT_MODE_VALUE 
		|| buff.st_uid != TEST_GFAL_LFC_FILE_STAT_UID_VALUE
		|| buff.st_gid != TEST_GFAL_LFC_FILE_STAT_GID_VALUE
		 || buff.st_size != TEST_GFAL_LFC_FILE_STAT_SIZE_VALUE){
		assert_true_with_message(FALSE, " this is not the correct value for the lfc lstat on file mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
		//g_printerr(" %o %o ", TEST_GFAL_LFC_FILE_STAT_MODE_VALUE , buff.st_mode);
		gfal_posix_release_error();
		return;			
	}
	
	gfal_posix_clear_error();
	
	res = gfal_lstat(TEST_GFAL_LFC_LINK_STAT_OK, &buff);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid lstat on a link");
		gfal_posix_release_error();
		return;
	}

	if(buff.st_mode != TEST_GFAL_LFC_LINK_STAT_MODE_VALUE 
		|| buff.st_uid != TEST_GFAL_LFC_LINK_STAT_UID_VALUE
		|| buff.st_gid != TEST_GFAL_LFC_LINK_STAT_GID_VALUE
		 || buff.st_size != TEST_GFAL_LFC_LINK_STAT_SIZE_VALUE){
		assert_true_with_message(FALSE, " this is not the correct value for the lfc lstat on link mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
		//g_printerr(" %o %o ", TEST_GFAL_LFC_FILE_STAT_MODE_VALUE , buff.st_mode);
		gfal_posix_release_error();
		return;			
	}
	
	res = gfal_lstat(TEST_GFAL_LFC_FILE_STAT_NONEXIST, &buff);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT){
		assert_true_with_message(FALSE, " must be a invalid lstat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}




void test__gfal_posix_lstat_guid()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_lstat(TEST_GFAL_GUID_FILE_STAT_OK, &buff);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid lstat");
		gfal_posix_release_error();
		return;
	}
	
	if(buff.st_mode != TEST_GFAL_LFC_FILE_STAT_MODE_VALUE 
		|| buff.st_uid != TEST_GFAL_LFC_FILE_STAT_UID_VALUE
		|| buff.st_gid != TEST_GFAL_LFC_FILE_STAT_GID_VALUE
		 || buff.st_size != TEST_GFAL_LFC_FILE_STAT_SIZE_VALUE){
		assert_true_with_message(FALSE, " this is not the correct value for the lfc lstat on file mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
		//g_printerr(" %o %o ", TEST_GFAL_LFC_FILE_STAT_MODE_VALUE , buff.st_mode);
		gfal_posix_release_error();
		return;			
	}
	
	gfal_posix_clear_error();
	

	res = gfal_lstat(TEST_GUID_NOEXIST_ACCESS, &buff);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT){
		assert_true_with_message(FALSE, " must be a invalid lstat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}



void test__gfal_posix_lstat_local()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	system(TEST_GFAL_LOCAL_STAT_COMMAND);
	system(TEST_GFAL_LOCAL_LINK_COMMAND);
	int res = gfal_lstat(TEST_GFAL_LOCAL_STAT_OK, &buff);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid lstat on a file");
		gfal_posix_release_error();
		return;
	}
	
	gfal_posix_clear_error();
	res = gfal_lstat(TEST_GFAL_LOCAL_LINK_OK, &buff);
	if(res != 0){
		assert_true_with_message(FALSE, " must be a valid lstat on a link");
		gfal_posix_release_error();
		return;
	}	

	res = gfal_lstat(TEST_GFAL_LOCAL_STAT_NONEXIST, &buff);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT){
		assert_true_with_message(FALSE, " must be a invalid lstat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}




void test__gfal_posix_lstat_srm()
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));

	int res = gfal_lstat(TEST_GFAL_SRM_FILE_STAT_OK, &buff);
	if(res == 0 || gfal_posix_code_error() != EPROTONOSUPPORT || errno != EPROTONOSUPPORT){
		assert_true_with_message(FALSE, " must be a invalid lstat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}
