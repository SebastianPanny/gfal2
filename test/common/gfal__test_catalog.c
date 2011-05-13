

/* unit test for common_catalog */


#include <check.h>
#include "gfal_constants.h"
#include "gfal_common_catalog.h"
#include "gfal_common_errverbose.h"
#include "../unit_test_constants.h"




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

START_TEST(test_catalog_access_file)
{
	GError* tmp_err=NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle == NULL){
		fail(" must init properly");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	int ret = gfal_catalogs_accessG(handle, TEST_LFC_VALID_ACCESS, F_OK, &tmp_err);
	if(ret !=0){
		fail(" must be a valid access");
		gfal_release_GError(&tmp_err);
		return;
	}
	ret = gfal_catalogs_accessG(handle, TEST_LFC_NOEXIST_ACCESS, F_OK, &tmp_err);	
	if(ret ==0 || tmp_err->code != ENOENT ){
		fail(" must be a non-existing file");
		gfal_release_GError(&tmp_err);
		return;
	}	
	gfal_handle_freeG(handle);
}
END_TEST


START_TEST(test_catalog_url_checker)
{
	GError* tmp_err=NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle == NULL){
		fail(" must init properly");
		gfal_release_GError(&tmp_err);
		return;
	}
	int ret = gfal_catalogs_accessG(handle, TEST_LFC_URL_SYNTAX_ERROR, F_OK, &tmp_err);
	if(ret ==0 || tmp_err->code != EPROTONOSUPPORT){
		fail(" must be an invalid protocol");
		gfal_release_GError(&tmp_err);
		return;
	}		
	gfal_handle_freeG(handle);	
}
END_TEST





START_TEST(test_catalog_guid_resolve)
{
	GError* tmp_err=NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle == NULL){
		fail(" must init properly");
		gfal_release_GError(&tmp_err);
		return;
	}
	char* ret = gfal_catalog_resolve_guid(handle, TEST_GUID_VALID_ACCESS, &tmp_err);
	if( ret == NULL || tmp_err){
		fail(" must be a success convertion");
		gfal_release_GError(&tmp_err);
		return;
	}
	ret = gfal_catalog_resolve_guid(handle, TEST_GUID_NOEXIST_ACCESS, &tmp_err);
	if(ret != NULL || !tmp_err){
		fail(" must be a non-valid guid");
		return;
	}
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
	
	
	
}
END_TEST


START_TEST(test__catalog_stat)
{
	struct stat resu;
	memset(&resu, 0, sizeof(struct stat));
	GError* tmp_err=NULL;
	
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle == NULL){
		fail(" must init properly");
		gfal_release_GError(&tmp_err);
		return;
	}	
	
	

	int ret = gfal_catalog_statG(handle, TEST_GFAL_LFC_FILE_STAT_OK, &resu, &tmp_err);
	if( ret != 0 || tmp_err){
		fail(" must be a success convertion");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	//g_printerr(" extract from the stat struct : right : %o, owner : %d, group : %d, size : %lu", resu.st_mode, resu.st_uid, resu.st_gid, resu.st_size);	
	ret = gfal_catalog_statG(handle, TEST_GFAL_LFC_FILE_STAT_NONEXIST, &resu, &tmp_err);
	if( ret == 0 || !tmp_err){
		fail(" must be a failure");
		gfal_release_GError(&tmp_err);
		return;
	}	
	
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
	
}
END_TEST



