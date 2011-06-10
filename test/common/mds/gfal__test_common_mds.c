

/* unit test for common_mds*/


#include <check.h>
#include <glib.h>
#include "gfal_common.h"
#include "mds/gfal_common_mds.h"



START_TEST(test_check_bdii_endpoints_srm)
{
	char **se_types=NULL;
	char **se_endpoints=NULL;
	GError * err=NULL;
	int ret=-1;
	char* endpoints[] = { "grid-cert-03.roma1.infn.it",
						NULL };
	char** ptr = endpoints;
	while(*ptr != NULL){
		 ret = gfal_mds_get_se_types_and_endpoints (*ptr, &se_types, &se_endpoints, &err);
		if(ret != 0){
				fail(" ret of bdii with error");
				perror(" bdii err  ");
				return;
		}
		fail_unless(ret == 0 && se_types != NULL && se_endpoints != NULL);
		g_strfreev(se_types);
		g_strfreev(se_endpoints);
		//fprintf(stderr, " se_types : %s , se_endpoints", se_types[1], se_endpoints[1]);
		ptr++;
	}
	
	ret = gfal_mds_get_se_types_and_endpoints ("google.com", &se_types, &se_endpoints, &err);		
	fail_unless(ret != 0 &&  err->code == EINVAL , "must fail, invalid url");
	g_clear_error(&err);
	g_strfreev(se_types);
	g_strfreev(se_endpoints);
}
END_TEST


START_TEST(gfal__test_get_lfchost_bdii)
{
	GError* tmp_err =NULL;
	errno = 0;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(tmp_err){
		gfal_release_GError(&tmp_err);
		fail("Error while init handle");
		return;		
	}
	char* lfc = gfal_get_lfchost_bdii(handle, &tmp_err);
	if(!lfc){
		gfal_release_GError(&tmp_err);
		fail(" must return correct lfc value");
		return;
	}
	//g_printerr(" lfc name : %s ", lfc);
	free(lfc);
	gfal_handle_freeG(handle);	
}
END_TEST


START_TEST(gfal__test_get_lfchost_bdii_with_nobdii)
{
	GError* tmp_err =NULL;
	errno = 0;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(tmp_err){
		gfal_release_GError(&tmp_err);
		fail("Error while init handle");
		return;		
	}
	gfal_set_nobdiiG(handle, TRUE);
	char* lfc = gfal_get_lfchost_bdii(handle, &tmp_err);
	if(lfc || tmp_err->code!= EPROTONOSUPPORT ){
		gfal_release_GError(&tmp_err);
		fail(" must return an error, nobdii option checked");
		free(lfc);
		return;
	}
	//g_printerr(" lfc name : %s ", lfc);
	free(lfc);	
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
	
}
END_TEST


