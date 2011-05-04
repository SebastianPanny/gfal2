

/* unit test for common_catalog */


#include <check.h>
#include "gfal_constants.h"
#include "gfal_common_catalog.h"





START_TEST (test_get_cat_type)
{
	char* cat_type;
	cat_type= gfal_get_cat_type(NULL);
	fail_unless(strncmp(cat_type, GFAL_DEFAULT_CATALOG_TYPE, 50) == 0);
	free(cat_type);
}
END_TEST


START_TEST (test_env_var)
{
	char* cat_type;
	char* old_env;
	old_env = getenv ("LCG_CATALOG_TYPE");
	setenv("LCG_CATALOG_TYPE", "CATAX",1);
	cat_type= gfal_get_cat_type(NULL);
	fail_unless(strncmp(cat_type, "CATAX", 50) == 0);
	if(old_env != NULL)
		setenv("LCG_CATALOG_TYPE", old_env,1);	// restore old env
	free(cat_type);
	free(old_env);
}
END_TEST
