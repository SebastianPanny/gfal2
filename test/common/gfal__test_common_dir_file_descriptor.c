
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
#include "gfal_types.h"


START_TEST(test__dir_file_descriptor_low)
{
	GError* tmp_err=NULL;
	gpointer pfile = (gpointer) (long)rand();
	gfal_fdesc_container_handle  h = gfal_file_descriptor_handle_create(NULL);
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



START_TEST(test__dir_file_descriptor_high)
{
	GError* tmp_err= NULL;
	const int id_module = rand(), id_module2 = rand();
	const gpointer desc = (gpointer)(long) rand(), desc2 = (gpointer)(long) rand();
	
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle == NULL){
		fail(" error while init");
		return;
	}
	
	gfal_fdesc_container_handle h =  gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);
	if( h == NULL){
		fail(" fail must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	gfal_file_handle d = gfal_dir_handle_bind(h, 10, &tmp_err);
	if( d != NULL || !tmp_err){
		fail(" fail, must an invalid bind");
		return;
	}
	g_clear_error(&tmp_err);
	
	int key = gfal_dir_handle_create(h,  id_module, desc, &tmp_err);
	if( key == 0 || tmp_err){
		fail(" fail, must be a valid creation");
		gfal_release_GError(&tmp_err);
		return;
	}

	int key2 = gfal_dir_handle_create(h,  id_module2, desc2, &tmp_err);
	if( key == 0 || tmp_err){
		fail(" fail, must be a valid creation 2");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	d = gfal_dir_handle_bind(h, key, &tmp_err);
	if( d == NULL || d->module_id != id_module || d->fdesc != desc){
		fail(" fail, must be a valid get");
		gfal_release_GError(&tmp_err);
		return;	
	} 	
	d = gfal_dir_handle_bind(h, key2, &tmp_err);
	if( d == NULL || d->module_id != id_module2 || d->fdesc != desc2){
		fail(" fail, must be a valid get");
		gfal_release_GError(&tmp_err);
		return;	
	} 		

	gboolean b = gfal_remove_file_desc(h, key+key2, &tmp_err);
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
	b = gfal_remove_file_desc(h, key+key2, &tmp_err);
	if(b || !tmp_err){
		fail(" must be an invalid deletion");
		return;
	}
	g_clear_error(&tmp_err);
	
	gfal_handle_freeG(handle);	
}
END_TEST
