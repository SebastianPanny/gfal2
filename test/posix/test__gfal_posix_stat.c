
/* unit test for posix access func */


#include <check.h>
#include <unistd.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>


START_TEST(test__gfal_posix_stat_lfc)
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_stat(TEST_GFAL_LFC_FILE_STAT_OK, &buff);
	if(res != 0){
		fail(" must be a valid stat");
		gfal_posix_release_error();
		return;
	}
	
	if(buff.st_mode != TEST_GFAL_LFC_FILE_STAT_MODE_VALUE 
		|| buff.st_uid != TEST_GFAL_LFC_FILE_STAT_UID_VALUE
		|| buff.st_gid != TEST_GFAL_LFC_FILE_STAT_GID_VALUE
		 || buff.st_size != TEST_GFAL_LFC_FILE_STAT_SIZE_VALUE){
		fail(" this is not the correct value for the lfc stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
		//g_printerr(" %o %o ", TEST_GFAL_LFC_FILE_STAT_MODE_VALUE , buff.st_mode);
		gfal_posix_release_error();
		return;			
	}
	
	gfal_posix_clear_error();
	res = gfal_stat(TEST_GFAL_LFC_FILE_STAT_NONEXIST, &buff);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT){
		fail(" must be a invalid stat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}
END_TEST




START_TEST(test__gfal_posix_stat_guid)
{
	struct stat buff;
	memset(&buff,0, sizeof(struct stat));
	int res = gfal_stat(TEST_GFAL_GUID_FILE_STAT_OK, &buff);
	if(res != 0){
		fail(" must be a valid stat");
		gfal_posix_release_error();
		return;
	}
	
	if(buff.st_mode != TEST_GFAL_LFC_FILE_STAT_MODE_VALUE 
		|| buff.st_uid != TEST_GFAL_LFC_FILE_STAT_UID_VALUE
		|| buff.st_gid != TEST_GFAL_LFC_FILE_STAT_GID_VALUE
		 || buff.st_size != TEST_GFAL_LFC_FILE_STAT_SIZE_VALUE){
		fail(" this is not the correct value for the lfc stat mode %o, uid %d, gid %d, size %d", 
								buff.st_mode, buff.st_uid, buff.st_gid, buff.st_size);
		gfal_posix_release_error();
		return;			
	}
	
	res = gfal_stat(TEST_GUID_NOEXIST_ACCESS, &buff);
	if(res == 0 || gfal_posix_code_error() != ENOENT || errno != ENOENT){
		fail(" must be a invalid stat %d %d %d", res, gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	
}
END_TEST



