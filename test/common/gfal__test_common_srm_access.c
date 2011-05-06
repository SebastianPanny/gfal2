
/* unit test for common_srm */


#include <check.h>
#include "gfal_common_srm_access.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "gfal_common_internal.h"
#include "../unit_test_constants.h"


START_TEST (test_create_srm_access_check_file)
{
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	if(handle == NULL){
		fail(" handle is not properly allocated");
		return;
	}

	int check = gfal_srm_accessG(handle, TEST_SRM_VALID_SURL_EXAMPLE1, F_OK, &err);
	if(check != 0 || err){
		fail(" must be a valid surl and readable file \n");
		gfal_release_GError(&err);
		return;
	}
	check = gfal_srm_accessG(handle, TEST_SRM_INVALID_SURL_EXAMPLE2, F_OK, &err);
	if(check != ENOENT || err){
		fail(" must be an invalid surl ");
		gfal_release_GError(&err);
		return;
	}
	gfal_handle_freeG(handle);
}
END_TEST

