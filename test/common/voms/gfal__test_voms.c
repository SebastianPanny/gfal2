/* unit test for common_errverbose */


#include <check.h>
#include "voms/gfal_voms_if.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

#define VO_TEST	"dteam"
#define DN_TEST "/DC=ch/DC=cern/OU=Organic Units/OU=Users/CN=adevress/CN=714425/CN=Adrien Devresse"
#define FQAN_TEST "/dteam"


// check if initiated properly
// need a voms proxy environnement
START_TEST (test_voms_parse_args)
{
	GError* err=NULL;
	int r =gfal_parse_vomsdataG(&err);

	if(r != 0){
		gfal_release_GError(&err);
			fail( "return must be null if success\n");
	}
}
END_TEST

// check if  same resutl after 2 times 
START_TEST (test_voms_info_is_null)
{
	GError* err=NULL;
	int r =gfal_parse_vomsdataG(&err);
	if(r != 0){
		gfal_release_GError(&err);
			fail("return must be null if success\n");
	}
}
END_TEST


// check if  same resutl after 2 times 
START_TEST (test_voms_info_test_vo)
{
	GError* err=NULL;
	char* ret =gfal_get_voG(&err);
	if(ret == NULL){
		gfal_release_GError(&err);
		fail( "return must be not null if success\n");
	}
	if(ret == NULL || err){
		fail(" must not report a null vo");
		return;
	}
	if(strncmp(ret, VO_TEST,255) != 0){
		fail(" must be the defined vo");	
		free(ret);	
		return;
	}
	free(ret);
}
END_TEST


// check if  same resutl after 2 times 
START_TEST (test_voms_get_userdnG)
{
	GError* err=NULL;
	char* ret =gfal_get_userdnG(&err);
	if(ret == NULL){
		gfal_release_GError(&err);
		fail( "return must be not null if success\n");
	}
	if(ret == NULL || err){
		fail(" must not report a null vo");
		return;
	}
	if(strncmp(ret, DN_TEST,2048) != 0){
		fail(" must be the defined vo");		
		return;
	}
	free(ret);
}
END_TEST


// check if  same resutl after 2 times 
START_TEST (test_voms_get_fqan)
{
	GError* err=NULL;
	GList* ret =gfal_get_fqanG(&err);
	if(ret == NULL){
		gfal_release_GError(&err);
		fail( "return must be not null if success\n");
	}
	int i;
	const int len = g_list_length (ret);
	for(i=0; i < len; i++){
		char* str= (char*) ret->data;
		fail_unless(  strstr(str, FQAN_TEST) != NULL, " must contain the vo");
		ret = g_list_next(ret);
	}
}
END_TEST
