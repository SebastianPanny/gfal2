
/* unit test for posix mkdir func */


#include <check.h>
#include <time.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include <glib.h>
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>


START_TEST(test__mkdir_posix_lfc_simple)
{
	struct stat st;
	int ret = gfal_mkdir(TEST_LFC_EEXIST_MKDIR, 0664);
	if( ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){
		fail(" must be an existing dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;
	}
	gfal_posix_clear_error();
	errno ==0;
	
	char filename[2048];
	time_t tt;
	time(&tt);
	char *t = ctime(&tt);
	strcpy(filename, TEST_LFC_BASE_FOLDER_URL_MKDIR1);
	strcat(filename, t); // generate a new unique dir identifier&
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		fail(" must be a valid create dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		fail(" bad right on the new created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}
	
	ret = gfal_mkdir(TEST_LFC_UNACCESS_MKDIR, 06640);
	if( ret ==0 || errno != EACCES || gfal_posix_code_error() != EACCES){
		fail(" must be a non-access dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;			
	}
	gfal_posix_clear_error();
	errno ==0;	
	
}
END_TEST
