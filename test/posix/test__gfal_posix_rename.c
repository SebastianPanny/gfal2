
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


START_TEST(test__gfal_posix_rename_catalog)
{
	int res = gfal_access(TEST_LFC_RENAME_VALID_SRC, F_OK);
	if(res !=0){
		fail("source file not present on the lfc");
		gfal_posix_release_error();
		return;
	}
	res = gfal_rename(TEST_LFC_RENAME_VALID_SRC, TEST_LFC_RENAME_VALID_DEST);
	if(res !=0){
		fail(" this move file must be a success");
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(TEST_LFC_RENAME_VALID_DEST, F_OK);
	if(res != 0){
		fail("dest file must be present if corrctly moved");
		gfal_posix_release_error();
		return;
	}
	res = gfal_rename( TEST_LFC_RENAME_VALID_DEST, TEST_LFC_RENAME_VALID_SRC);
	if(res !=0){
		fail(" this move file must be a success");
		gfal_posix_release_error();
		return;
	}
	res = gfal_access(TEST_LFC_RENAME_VALID_SRC, F_OK);
	if(res !=0){
		fail("the soruce file must be in the init position");
		gfal_posix_release_error();
		return;
	}
}
END_TEST

START_TEST(test__gfal_posix_move_dir_catalog)
{
	int res = gfal_access(TEST_LFC_MOVABLE_DIR_SRC, F_OK);
	if(res !=0){
		fail("source dir not present on the lfc");
		gfal_posix_release_error();
		return;
	}
	res = gfal_rename(TEST_LFC_MOVABLE_DIR_SRC, TEST_LFC_MOVABLE_DIR_DEST);
	if(res !=0){
		fail(" this move dir must be a success");
		gfal_posix_release_error();
		return;
	}	
	res = gfal_access(TEST_LFC_MOVABLE_DIR_DEST, F_OK);
	if(res !=0){
		fail("dest dir not present on the lfc");
		gfal_posix_release_error();
		return;
	}	
	res = gfal_rename(TEST_LFC_MOVABLE_DIR_DEST, TEST_LFC_MOVABLE_DIR_SRC );
	if(res !=0){
		fail(" this move dir must be a success");
		gfal_posix_release_error();
		return;
	}		
	res = gfal_access(TEST_LFC_MOVABLE_DIR_SRC, F_OK);
	if(res !=0){
		fail("source dir not present on the lfc after move");
		gfal_posix_release_error();
		return;
	}		
}
END_TEST


START_TEST(test__gfal_posix_rename_url_check)
{
	int res = gfal_rename( TEST_LFC_NOEXIST_ACCESS, TEST_LFC_RENAME_VALID_DEST);
	if(res ==0 || errno != ENOENT || gfal_posix_code_error() != ENOENT){
		fail("this move dir cannot succed, src url does not exist");
		gfal_posix_release_error();
		return;
	}
		
	res = gfal_rename( TEST_LFC_NOEXIST_ACCESS, "google.com");
	if(res ==0 || errno != EPROTONOSUPPORT || gfal_posix_code_error() != EPROTONOSUPPORT){
		fail("unknow protocol, must fail");
		gfal_posix_release_error();
		return;
	}	
}
END_TEST
