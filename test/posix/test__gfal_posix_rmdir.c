
/* unit test for posixrmdir func */


#include <check.h>
#include <time.h>
#include <string.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include <glib.h>
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>


START_TEST(test__rmdir_posix_lfc_simple)
{
	int ret = -1;
	
	ret = gfal_mkdir(NULL,0777);
	if( ret == 0 || gfal_posix_code_error() != EFAULT){
		fail(" must be a NULL path %d", gfal_posix_code_error());
		gfal_posix_release_error();
		return;		
	}
	gfal_posix_clear_error();
	ret = gfal_mkdir(TEST_LFC_RMDIR_CREATED,0777);
	if( ret !=0 && gfal_posix_code_error() != EEXIST){
		fail(" must be a valid dir creation %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_LFC_RMDIR_CREATED);
	if(ret != 0){
		fail(" must be a valid dir deletion %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_LFC_RMDIR_CREATED);
	if(ret == 0 || gfal_posix_code_error() != ENOENT){
		fail(" must be an invalid deletion, this dir is not existing : %d ", gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();	
}
END_TEST


START_TEST(test__rmdir_posix_lfc_existingfile)
{
	int ret = -1;
	
	ret = gfal_rmdir(TEST_LFC_RMDIR_EEXIST);
	if( ret == 0 || gfal_posix_code_error() != EEXIST){
		fail(" must be an invalid deletion, this dir is not empty : %d ", gfal_posix_code_error());
		gfal_posix_release_error();
		return;		
	}	
		
}
END_TEST


START_TEST(test__rmdir_posix_lfc_slash)
{
	int ret = -1;
	
	gfal_posix_clear_error();
	ret = gfal_mkdir(TEST_LFC_RMDIR_CREATED_SLASH,0777);
	if( ret !=0 && gfal_posix_code_error() != EEXIST){
		fail(" must be a valid dir creation %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_LFC_RMDIR_CREATED_SLASH);
	if(ret != 0){
		fail(" must be a valid dir deletion %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_LFC_RMDIR_CREATED_SLASH);
	if(ret == 0 || gfal_posix_code_error() != ENOENT){
		fail(" must be an invalid deletion, this dir is not existing : %d ", gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();	
}
END_TEST





// srm part

START_TEST(test__rmdir_posix_srm_simple)
{
	int ret = -1;
	
	gfal_posix_clear_error();
	ret = gfal_mkdir(TEST_SRM_RMDIR_CREATED,0777);
	if( ret !=0 && gfal_posix_code_error() != EEXIST){
		fail(" must be a valid dir creation %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_SRM_RMDIR_CREATED);
	if(ret != 0){
		fail(" must be a valid dir deletion %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_SRM_RMDIR_CREATED);
	if(ret == 0 || gfal_posix_code_error() != ENOENT){
		fail(" must be an invalid deletion, this dir is not existing : %d ", gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();	
}
END_TEST


START_TEST(test__rmdir_posix_srm_existingfile)
{
	int ret = -1;
	
	ret = gfal_rmdir(TEST_LOCAL_RMDIR_EEXIST);
	if( ret == 0 || gfal_posix_code_error() != EEXIST){
		fail(" must be an invalid deletion, this dir is not empty : %d ", gfal_posix_code_error());
		gfal_posix_release_error();
		return;		
	}	
		
}
END_TEST


START_TEST(test__rmdir_posix_srm_slash)
{
	int ret = -1;
	
	gfal_posix_clear_error();
	ret = gfal_mkdir(TEST_SRM_RMDIR_CREATED_SLASH,0777);
	if( ret !=0 && gfal_posix_code_error() != EEXIST){
		fail(" must be a valid dir creation %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_SRM_RMDIR_CREATED_SLASH);
	if(ret != 0){
		fail(" must be a valid dir deletion %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_SRM_RMDIR_CREATED_SLASH);
	if(ret == 0 || gfal_posix_code_error() != ENOENT){
		fail(" must be an invalid deletion, this dir is not existing : %d ", gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();	
}
END_TEST


// local part

START_TEST(test__rmdir_posix_local_simple)
{
	int ret = -1;
	
	gfal_posix_clear_error();
	ret = gfal_mkdir(TEST_LOCAL_RMDIR_CREATED,0777);
	if( ret !=0 && gfal_posix_code_error() != EEXIST){
		fail(" must be a valid dir creation %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_LOCAL_RMDIR_CREATED);
	if(ret != 0){
		fail(" must be a valid dir deletion %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_LOCAL_RMDIR_CREATED);
	if(ret == 0 || gfal_posix_code_error() != ENOENT){
		fail(" must be an invalid deletion, this dir is not existing : %d ", gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();	
}
END_TEST


START_TEST(test__rmdir_posix_local_existingfile)
{
	int ret = -1;
	
	system(TEST_LOCAL_RMDIR_EEXIST_COMMAND);
	ret = gfal_rmdir(TEST_LOCAL_RMDIR_EEXIST);
	if( ret == 0 || gfal_posix_code_error() != EEXIST){
		fail(" must be an invalid deletion, this dir is not empty : %d ", gfal_posix_code_error());
		gfal_posix_release_error();
		return;		
	}	
		
}
END_TEST



START_TEST(test__rmdir_posix_local_slash)
{
	int ret = -1;
	
	gfal_posix_clear_error();
	ret = gfal_mkdir(TEST_LOCAL_RMDIR_CREATED_SLASH,0777);
	if( ret !=0 && gfal_posix_code_error() != EEXIST){
		fail(" must be a valid dir creation %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_LOCAL_RMDIR_CREATED_SLASH);
	if(ret != 0){
		fail(" must be a valid dir deletion %d %d", ret, gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_rmdir(TEST_LOCAL_RMDIR_CREATED_SLASH);
	if(ret == 0 || gfal_posix_code_error() != ENOENT){
		fail(" must be an invalid deletion, this dir is not existing : %d ", gfal_posix_code_error());
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();	
}
END_TEST
