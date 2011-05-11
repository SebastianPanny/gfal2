
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
		fail(" must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, F_OK);
	if(ret != -1 || errno != ENOENT){
		fail("must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access("google.com", F_OK);
	if(ret != -1 || errno != EPROTONOSUPPORT){
		fail(" must be a syntax error %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();

}
END_TEST


START_TEST(test_access_posix_guid_read)
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_GUID_ONLY_READ_ACCESS, R_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}	
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, R_OK);
	if(ret != -1 || errno != ENOENT){
		fail("must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();	
	ret = gfal_access(TEST_GUID_NO_READ_ACCESS, R_OK);
	if(ret != -1 || errno != EACCES){
		fail("must be an unaccessible file %s", strerror(errno));
		gfal_posix_release_error();
		return;		
	}	
	
	gfal_posix_clear_error();
	
}
END_TEST


START_TEST(test_access_posix_guid_write)
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_GUID_WRITE_ACCESS, W_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}	
	ret = gfal_access(TEST_GUID_NOEXIST_ACCESS, W_OK);
	if(ret >=0 || errno != ENOENT){
		fail("must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_GUID_NO_WRITE_ACCESS, W_OK);
	if(ret >=0 || errno != EACCES){
		fail("must be an non-writable file %s", strerror(errno));
		gfal_posix_release_error();
		return;		
	}	
	gfal_posix_clear_error();	
}
END_TEST


START_TEST(test_access_posix_lfn_exist)
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_LFC_ONLY_READ_ACCESS, F_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_LFC_NOEXIST_ACCESS, F_OK);
	if(ret >=0 || errno != ENOENT){
		fail("must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access("google.com", F_OK);
	if(ret >=0 || errno != EPROTONOSUPPORT){
		fail(" must be a syntax error %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();
}
END_TEST


START_TEST(test_access_posix_lfn_read)
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_LFC_ONLY_READ_ACCESS, R_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_LFC_NOEXIST_ACCESS, R_OK);
	if(ret >=0 || errno != ENOENT){
		fail("must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_LFC_NO_READ_ACCESS, R_OK);
	if(ret >= 0 || errno != EACCES){
		fail(" must be a non-readable %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();	
	
}
END_TEST


START_TEST(test_access_posix_lfn_write)
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_LFC_WRITE_ACCESS, W_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_LFC_NOEXIST_ACCESS, W_OK);
	if(ret >=0 || errno != ENOENT){
		fail("must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_LFC_NO_WRITE_ACCESS, W_OK);
	if(ret >=0 || errno != EACCES){
		fail(" must be a non-readable %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();	
	
}
END_TEST



START_TEST(test_access_posix_srm_exist)
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_SRM_ONLY_READ_ACCESS, F_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_SRM_NOEXIST_ACCESS, F_OK);
	if(ret >=0 || errno != ENOENT){
		fail("must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access("google.com", F_OK);
	if(ret >=0 || errno != EPROTONOSUPPORT){
		fail(" must be a syntax error %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();
}
END_TEST



START_TEST(test_access_posix_srm_read)
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_SRM_ONLY_READ_ACCESS, R_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_SRM_NOEXIST_ACCESS, R_OK);
	if(ret >=0 || errno != ENOENT){
		fail("must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_SRM_NO_READ_ACCESS, R_OK);
	if(ret >=0 || errno != EACCES){
		fail(" must be an invalid read access error %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();
}
END_TEST


START_TEST(test_access_posix_srm_write)
{
	// test exist guid
	int ret;

	ret = gfal_access(TEST_SRM_WRITE_ACCESS, W_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	ret = gfal_access(TEST_SRM_NOEXIST_ACCESS, W_OK);
	if(ret >=0  || errno != ENOENT){
		fail("must be a non exist guid %s", strerror(errno));
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_access(TEST_SRM_NO_WRITE_ACCESS, W_OK);
	if(ret >=0 || errno != EACCES){
		fail(" must be an invalid write access %s", strerror(errno));		
		gfal_posix_release_error();
	}
	gfal_posix_clear_error();
}
END_TEST

