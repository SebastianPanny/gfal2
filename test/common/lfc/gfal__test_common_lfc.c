/*
 *  lfc test file 
 * 
 * */
 
 
#include <check.h>
#include <glib.h>
#include "lfc/gfal_common_lfc.h"



START_TEST(test_gfal_common_lfc_define_env)
{
	char* old_host = getenv("LFC_HOST");
	char* old_port = getenv("LFC_PORT");
	GError * tmp_err=NULL;
	
	setenv("LFC_HOST", "google.com",1);
	setenv("LFC_PORT", "4465488896645546564",1);

	int ret = gfal_setup_lfchost(&tmp_err);
	if(ret == 0){
		fail(" must fail, port invalid");	
	}
	errno = 0;
	g_clear_error(&tmp_err);
	setenv("LFC_PORT", "2000",1);	
	ret = gfal_setup_lfchost(&tmp_err);	// re-test with good port number
	if(ret){
		fail(" must be a success");
		
	}
	char *new_host = getenv("LFC_HOST");
	char *new_port = getenv("LFC_PORT");
	ret = strcmp(new_port, "2000") | strcmp(new_host, "google.com");
	if(ret){
		fail("must be the same string");
	}
	
	setenv("LFC_HOST", old_host,1);	
	setenv("LFC_PORT", old_port,1);	
}
END_TEST


START_TEST(test_gfal_common_lfc_init)
{
	/*GError * tmp_err=NULL;
	gfal_catalogs_instance(&tmp_err);
	if(tmp_err){
		gfal_release_GError(&tmp_err);*/
		fail(" error must init properly");
	/*	return;
	}
	gfal_catalogs_delete();	*/
	
}
END_TEST


