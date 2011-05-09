
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
	ret = gfal_access("guid:ae571282-81ea-41af-ac3c-8b1a084bfc82", F_OK);
	if(ret != 0){
		fail(" must be a valid access to the guid");
		gfal_posix_print_error();
		return;
	}
	ret = gfal_access("guid:ae595782-81be-40af-ac3c-8b1a084bfc82", F_OK);
	if(ret != ENOENT){
		fail(" must be a non exist guid");
		gfal_posix_print_error();
		return;
	}
	
	ret = gfal_access("google.com", F_OK);
	if(ret != EINVAL){
		fail(" must be a syntax error");		
		gfal_posix_print_error();
	}

	
}
END_TEST
