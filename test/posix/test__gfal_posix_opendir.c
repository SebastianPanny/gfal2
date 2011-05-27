

/* unit test for opendir/readdir/closedir func */


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



START_TEST(test__opendir_posix_local_simple)
{
	gfal_posix_clear_error();
	DIR* d = gfal_opendir(TEST_LOCAL_OPENDIR_OPEN_INVALID);
	if(d!=NULL|| gfal_posix_code_error() != ENOENT || errno != ENOENT){
		fail(" error, must be a non existing dir %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	d = gfal_opendir(TEST_LOCAL_OPENDIR_OPEN_NOACCESS);
	if(d!=NULL|| gfal_posix_code_error() != EACCES || errno != EACCES){
		fail(" error, must be a non accessible dir  %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}		
	gfal_posix_clear_error();	
	d = gfal_opendir(TEST_LOCAL_OPENDIR_OPEN);
	if(d==NULL|| gfal_posix_code_error() != 0){
		fail(" error, must be a valid open");
		gfal_posix_release_error();
		return;
	}
	
	int ret = gfal_closedir(d);
	if( ret != 0 || gfal_posix_code_error() != 0 || errno != 0){
		fail(" must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_closedir(d);
	if( ret == 0 || gfal_posix_code_error() == 0 || errno == 0){
		fail(" must be a non existing dir descriptor closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}
END_TEST
