/*
 *  lfc test file 
 * 
 * */
 
 
#include <check.h>
#include <glib.h>
#include "lfc/gfal_common_lfc.h"



START_TEST(test_gfal_common_lfc_init)
{
	GError * tmp_err=NULL;
	gfal_catalogs_instance(&tmp_err);
	if(tmp_err){
		gfal_release_GError(&tmp_err);
		fail(" error must init properly");
		return;
	}
	gfal_catalogs_delete();	
	
}
END_TEST


