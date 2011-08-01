

/* unit test for opendir/readdir/closedir func */


#include <cgreen/cgreen.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <glib.h>
#include <lfc_api.h>

#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include "gfal_posix_internal.h"
#include "../unit_test_constants.h"
#include "gfal_posix_api.h"
#include "../mock/gfal_srm_mock_test.h"

void test_mock_lfc_opendir(){
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);

	if( gfal_check_GError(&mock_err))
		return;	
#endif
}
void test_mock_lfc_opendir_enoent_eaccess_valid(char* enoent, char* eaccess, char* valid){
#if USE_MOCK
	test_mock_lfc_opendir();
	will_respond(lfc_mock_opendir, -ENOENT, want_string(path, enoent+4));
	will_respond(lfc_mock_opendir, -EACCES, want_string(path, eaccess+4));	
	will_respond(lfc_mock_opendir, 5, want_string(path, valid+4));
	will_respond(lfc_mock_closedir, 0, want_non_null(dir));	
	//will_respond(lfc_mock_closedir, EBADF, want_non_null(dir)); -> filter by map
	always_respond(lfc_mock_opendir, EINVAL);	
#endif
}

void test_mock_lfc_readdir(char* valid, GList* str_list){
#if USE_MOCK
	test_mock_lfc_opendir();
	will_respond(lfc_mock_opendir, 5, want_string(path, valid+4));
	will_respond(lfc_mock_closedir, 0, want_non_null(dir));	
	do{
		struct Cns_direnstat* p = malloc(sizeof(struct Cns_direnstat)*1+ 256*sizeof(char));
		memset(p, 0,sizeof(struct Cns_direnstat));
		g_strlcpy(p->d_name,str_list->data, 256 );
		will_respond(lfc_mock_readdirx, p, want_non_null(dir));
	}while( (str_list= g_list_next(str_list)) != NULL);
	will_respond(lfc_mock_readdirx, 0, want_non_null(dir));
	always_respond(lfc_mock_opendir, EINVAL);	
#endif
}




void test__opendir_generic_test_simple(char* enoent, char* eaccess, char* valid){
	gfal_posix_clear_error();
	DIR* d = gfal_opendir(enoent);
	assert_true_with_message(d== NULL && gfal_posix_code_error() == ENOENT && errno == ENOENT, " error, must be a non existing dir %ld %d %d",d,gfal_posix_code_error(), errno);

	gfal_posix_clear_error();
	d = gfal_opendir(eaccess);
	assert_true_with_message(d== NULL && gfal_posix_code_error() == EACCES && errno == EACCES, "error, must be a non accessible dir  %ld %d %d",d,gfal_posix_code_error(), errno);
	gfal_posix_clear_error();	

	d = gfal_opendir(valid);
	assert_true_with_message(d !=NULL && gfal_posix_code_error() == 0 && errno == 0, "error, must be a non accessible dir  %ld %d %d",d,gfal_posix_code_error(), errno);

	int ret = gfal_closedir(d);
	assert_true_with_message(ret ==0 && gfal_posix_code_error() == 0 && errno ==0, " must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);
	
	ret = gfal_closedir(d);
	assert_true_with_message(ret !=0 && gfal_posix_code_error() != 0 && errno !=0, " must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);	
	gfal_posix_clear_error();		
}

void test__readdir_generic_test_simple(char * folder, GList* list_dir){
	DIR* d = gfal_opendir(folder);
	assert_true_with_message(d != NULL && gfal_posix_code_error() == 0 && errno == 0, " error, must be an existing dir %ld %d %d",d,gfal_posix_code_error(), errno);
	if(gfal_posix_check_error())
		return;
	
	int count=0;
	GList* tmpl = list_dir;
	struct dirent* dirs;
	do{
		 dirs = gfal_readdir(d);
		if(dirs != NULL){
			assert_true_with_message( strcmp(dirs->d_name, tmpl->data) ==0, " must be the same string : %s %s ", dirs->d_name, tmpl->data);
			tmpl = g_list_next(tmpl);
			count++;
		}else{
			assert_true_with_message(dirs == NULL && gfal_posix_code_error() == 0 && errno ==0, " error, must be the end of the dir list %ld %d %d",dirs,gfal_posix_code_error(), errno);
		}
	} while(dirs != NULL);
	assert_true_with_message(g_list_length(list_dir) == count, " must return the good number of directories");

	int ret = gfal_closedir(d);
	assert_true_with_message(ret ==0 && gfal_posix_code_error() ==0 && errno==0, " must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);	
	
	dirs = gfal_readdir(d);
	assert_true_with_message(dirs == NULL && gfal_posix_code_error() ==EBADF && errno==EBADF, " must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);	
	gfal_posix_clear_error();		
	
}

void test__opendir_posix_local_simple()
{
	test__opendir_generic_test_simple(TEST_LOCAL_OPENDIR_OPEN_INVALID, TEST_LOCAL_OPENDIR_OPEN_NOACCESS, TEST_LOCAL_OPENDIR_OPEN);	
}




void test__opendir_posix_lfc_simple()
{
	test_mock_lfc_opendir_enoent_eaccess_valid(TEST_LFC_OPENDIR_OPEN_INVALID, TEST_LFC_OPENDIR_OPEN_NOACCESS, TEST_LFC_OPENDIR_OPEN);
	test__opendir_generic_test_simple(TEST_LFC_OPENDIR_OPEN_INVALID, TEST_LFC_OPENDIR_OPEN_NOACCESS, TEST_LFC_OPENDIR_OPEN);	
}


void test__readdir_posix_local_simple()
{
	gfal_posix_clear_error();
	system(TEST_LOCAL_READDIR_CREATE_COMMAND);
	GList* list_dir = g_list_append(NULL, TEST_LOCAL_READDIR_1);
	list_dir = g_list_append(list_dir, "..");
	list_dir = g_list_append(list_dir, TEST_LOCAL_READDIR_3);
	list_dir = g_list_append(list_dir, TEST_LOCAL_READDIR_2);
	list_dir = g_list_append(list_dir, TEST_LOCAL_READDIR_4);	
	list_dir = g_list_append(list_dir, ".");
	
	test__readdir_generic_test_simple(TEST_LOCAL_READDIR_VALID, list_dir);	
}




void test__readdir_posix_lfc_simple()
{
	gfal_posix_clear_error();
	GList* list_dir = g_list_append(NULL, TEST_LFC_READDIR_1);
	list_dir = g_list_append(list_dir, TEST_LFC_READDIR_2);
	list_dir = g_list_append(list_dir, TEST_LFC_READDIR_3);
	list_dir = g_list_append(list_dir, TEST_LFC_READDIR_4);
	test_mock_lfc_readdir(TEST_LFC_READDIR_VALID, list_dir);
	test__readdir_generic_test_simple(TEST_LFC_READDIR_VALID, list_dir);
}



void test__opendir_posix_srm_simple_mock()
{
	test__opendir_generic_test_simple(TEST_SRML_OPENDIR_ENOENT, TEST_SRML_OPENDIR_EACCESS, TEST_SRM_OPENDIR_OPEN);
}



void test__readdir_posix_srm_simple_mock()
{
	gfal_posix_clear_error();
	GList* list_dir = g_list_append(NULL, TEST_SRM_READDIR_1);
	list_dir = g_list_append(list_dir, TEST_SRM_READDIR_2);
	list_dir = g_list_append(list_dir, TEST_SRM_READDIR_3);
	list_dir = g_list_append(list_dir, TEST_SRM_READDIR_4);
	test__readdir_generic_test_simple(TEST_SRM_READDIR_VALID, list_dir);
}	




