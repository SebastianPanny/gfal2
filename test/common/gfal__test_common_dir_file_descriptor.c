
/* unit test for file descriptor */


#include <check.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "gfal_common_internal.h"
#include "../unit_test_constants.h"
#include "gfal_common_dir_handle.h"
#include "gfal_common_filedescriptor.h"


START_TEST(test__dir_file_descriptor_low)
{
	GError* tmp_err=NULL;
	gpointer pfile = (gpointer) (long)rand();
	gfal_file_descriptor_handle  h = gfal_file_descriptor_handle_create(NULL);
	if( h == NULL){
		fail(" fail must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	gpointer res = gfal_get_file_desc(h, 10, &tmp_err);
	if( res || !tmp_err){
		fail(" no file desc must be in the container");
		return;
	}
	g_clear_error(&tmp_err);
    int key = gfal_add_new_file_desc(h, pfile, &tmp_err);
    if( key ==0 || tmp_err){
		fail(" must be a valid key creation");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	res = gfal_get_file_desc(h, key, &tmp_err);
	if( res != pfile || tmp_err){
		fail(" must be a valid descriptor addition");
		gfal_release_GError(&tmp_err);
		return;
	}	
	
	gboolean b = gfal_remove_file_desc(h, key+1, &tmp_err);
	if(b || !tmp_err){
		fail(" must be an invalid deletion");
		return;
	}
	g_clear_error(&tmp_err);
	 b = gfal_remove_file_desc(h, key, &tmp_err);
	if(!b || tmp_err){
		fail(" must be a valid deletion");
		gfal_release_GError(&tmp_err);
		return;
	}	
	
}
END_TEST
