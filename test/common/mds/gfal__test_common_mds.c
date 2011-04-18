

/* unit test for common_mds*/


#include <check.h>
#include "gfal_common.h"



START_TEST (test_check_bdii_endpoints_srm)
{
	char **se_types;
	char **se_endpoints;
	GError * err=NULL;
	int ret = gfal_mds_get_se_types_and_endpoints ("grid-cert-03.roma1.infn.it", &se_types, &se_endpoints, &err);
	fprintf(stderr, " msg %s ", se_endpoints[0]);
	fail_unless(ret==0);

}
END_TEST



