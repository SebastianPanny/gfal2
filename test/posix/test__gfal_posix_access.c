
/* unit test for posix access func */


#include <check.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>



START_TEST(test_access_posix_guid_exist)
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_GUID_VALID_ACCESS, F_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid %s", strerror(ret));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, F_OK);
	if(ret != ENOENT){
		fail("must be a non exist guid %s", strerror(ret));
		gfal_posix_release_error();
		return;
	}

	ret = gfal_access("google.com", F_OK);
	if(ret != EINVAL){
		fail(" must be a syntax error %s", strerror(ret));		
		gfal_posix_release_error();
	}

}
END_TEST
