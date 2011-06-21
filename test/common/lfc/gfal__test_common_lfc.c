/*
 *  lfc test file 
 * 
 * */
 
 
#include <cgreen/cgreen.h>
#include <glib.h>
#include <dlfcn.h>
#include <lfc_api.h>
#include "lfc/gfal_common_lfc.h"
#include "lfc/lfc_ifce_ng.h"
#include "gfal_common_internal.h"
#include "../../unit_test_constants.h"
#include "../../mock/gfal_lfc_mock_test.h"


// mocking function internal to gfal
gfal_catalog_interface get_lfc_interface(gfal_handle handle, GError** err){
	gfal_catalog_interface i;
	void* dl = dlopen("libgfal_plugin_lfc.so", RTLD_LAZY);
	if(!dl){
		g_set_error(err, 0, EINVAL, "unable to find the gfal plugin lfc");
		return i;
	}
	gfal_catalog_interface (*constructor)(gfal_handle,GError**) = (gfal_catalog_interface (*)(gfal_handle,GError**)) dlsym(dl, "gfal_plugin_init");	
	if(constructor==NULL){
		g_set_error(err, 0, EINVAL, "unable to resolve constructor in lfc plugin");
		return i;		
	}
	i = constructor(handle ,err);
#if USE_MOCK
	i.handle = calloc(1, sizeof(struct lfc_ops));
	struct lfc_ops* ops = (struct lfc_ops*) i.handle; 
	ops->lfc_endpoint = NULL;
	ops->statg = &lfc_mock_statg;
	ops->rename = &lfc_mock_rename;
	ops->serrno = &lfc_last_err;
	ops->sstrerror = &strerror;
#endif
	return i;
}



void test_gfal_common_lfc_define_env()
{
	char* old_host = getenv("LFC_HOST");
	char* old_port = getenv("LFC_PORT");
	GError * tmp_err=NULL;
	int ret =0;
	char* resu = NULL;
	
	setenv("LFC_HOST", "google.com",1);
	setenv("LFC_PORT", "4465488896645546564",1);

	gfal_handle handle = gfal_initG(&tmp_err);

	assert_true_with_message(handle != NULL, " handle must be initiated properly ");
	if(!handle)
		return;
		
	resu = gfal_setup_lfchost(handle, &tmp_err);
	assert_true_with_message(!resu, " must fail, port invalid");
	errno = 0;
	g_clear_error(&tmp_err);
	tmp_err=NULL;
	setenv("LFC_PORT", "2000",1);	
	resu = gfal_setup_lfchost(handle, &tmp_err);	// re-test with good port number
	
	assert_true_with_message(resu != NULL, " must be a success, LFC_HOST & LFC_PORT defined");
	if(tmp_err)
		gfal_release_GError(&tmp_err);

	char *new_host = getenv("LFC_HOST");
	char *new_port = getenv("LFC_PORT");
	ret = strcmp(new_port, "2000") | strcmp(new_host, "google.com");

	assert_true_with_message(!ret, "must be the same string");
	
	
	if(old_host)
		setenv("LFC_HOST", old_host,1);	
	else
		unsetenv("LFC_HOST");
	if(old_port)
		setenv("LFC_PORT", old_port,1);
	else
		unsetenv("LFC_PORT");
	free(resu);
	gfal_handle_freeG(handle);
}


void test_gfal_common_lfc_resolve_sym()
{
	GError* err = NULL;
	struct lfc_ops* st = gfal_load_lfc("liblfc.so", &err);
	assert_true_with_message(st || !err, " must be a valid resolution, liblfc not found");
	if(st == NULL)
		return;
	
	assert_true_with_message(st->sstrerror != NULL , "must be a valid symbol");
	
	char* str = NULL;
	str = st->sstrerror(0);
	assert_true_with_message(str != NULL, " must be a valid string");
	free(st);
}




void test_gfal_common_lfc_init()
{
	GError * tmp_err=NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	
	assert_true_with_message(handle != NULL, " error must be initiated %s");
	if(handle==NULL){
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = get_lfc_interface(handle, &tmp_err);
	if(tmp_err){
		assert_true_with_message(FALSE, " must not fail, valid value");
		return;
	}
	gfal_handle_freeG(handle);
}



void test_gfal_common_lfc_access(){
	GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		assert_true_with_message(FALSE, " error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = get_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		assert_true_with_message(FALSE, " must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	ret = i.accessG(i.handle, TEST_LFC_VALID_ACCESS, R_OK, &tmp_err);
	if(ret !=0){
		assert_true_with_message(FALSE, " must be a valid access");
		gfal_release_GError(&tmp_err);
		return;
	}
	g_clear_error(&tmp_err);
	ret = i.accessG(i.handle, TEST_LFC_VALID_ACCESS, W_OK, &tmp_err);	
	if(ret == 0 || tmp_err->code != EACCES){
		assert_true_with_message(FALSE, " must fail, unable to write this file");
		gfal_release_GError(&tmp_err);
		return;
	}
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);	
}



void test_gfal_common_lfc_no_exist()
{
	GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		assert_true_with_message(FALSE, " error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = get_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		assert_true_with_message(FALSE, " must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	ret = i.accessG(i.handle, TEST_LFC_NOEXIST_ACCESS, F_OK, &tmp_err);
	if(ret ==0 || tmp_err->code != ENOENT){
		assert_true_with_message(FALSE, " must fail, this file not exist");
		gfal_release_GError(&tmp_err);
		return;
	}
	g_clear_error(&tmp_err);	
	ret = i.accessG(i.handle, TEST_LFC_VALID_ACCESS, F_OK, &tmp_err);
	if(ret !=0){
		assert_true_with_message(FALSE, "must be a success, file is present");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);		
	gfal_handle_freeG(handle);
}



void test_gfal_common_lfc_check_filename()
{
GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		assert_true_with_message(FALSE, " error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = get_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		assert_true_with_message(FALSE, " must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	gboolean b = i.check_catalog_url(i.handle, TEST_LFC_VALID_ACCESS, GFAL_CATALOG_ACCESS, &tmp_err);
	if(!b || tmp_err){
		assert_true_with_message(FALSE, " must be a valid lfn url");
		return;
	}
	b = i.check_catalog_url(i.handle, TEST_LFC_NOEXIST_ACCESS, GFAL_CATALOG_ACCESS, &tmp_err);
	if(!b || tmp_err){
		assert_true_with_message(FALSE, " must be a valid lfn url 2");
		return;
	}
	b = i.check_catalog_url(i.handle, TEST_LFC_URL_SYNTAX_ERROR, GFAL_CATALOG_ACCESS, &tmp_err);
	if(b){
		assert_true_with_message(FALSE, " must an invalid lfn url 3");
		return;
	}
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);
	gfal_handle_freeG(handle);	
}


void test_gfal_common_lfc_getSURL()
{
	GError * tmp_err=NULL;
	char** ret =NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		assert_true_with_message(FALSE, "error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = get_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		assert_true_with_message(FALSE, "must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}	
	ret = i.getSURLG(i.handle, TEST_LFC_VALID_ACCESS, &tmp_err);
	if(ret == NULL || tmp_err){
		g_printerr(" errno : %s ", strerror(tmp_err->code));
		assert_true_with_message(FALSE, "must be a successfull convert");
		gfal_release_GError(&tmp_err);
		return;
	}
	char** p = ret;
	while(*p != NULL){
		assert_false_with_message( strncmp(*p,"srm://",6) !=0, " begin of the surl is incorrect : %s ", p);
		p++;
	}
	g_strfreev(ret);
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);	
	gfal_handle_freeG(handle);	
	
}



void test_gfal_common_lfc_access_guid_file_exist()
{
	GError * tmp_err=NULL;
	char* ret =NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		assert_true_with_message(FALSE, "error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = get_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		assert_true_with_message(FALSE, "must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	ret = i.resolve_guid(i.handle, TEST_GUID_NOEXIST_ACCESS, &tmp_err);
	if(ret !=NULL || tmp_err->code != ENOENT){
		assert_true_with_message(FALSE, "must fail, this file not exist");
		gfal_release_GError(&tmp_err);
		return;
	}
	free(ret);
	g_clear_error(&tmp_err);
	ret = i.resolve_guid(i.handle, TEST_GUID_VALID_ACCESS, &tmp_err);
	if(ret == NULL || tmp_err){
		g_printerr(" errno : %s ", strerror(tmp_err->code));
		assert_true_with_message(FALSE, "must be a success, file is present");
		gfal_release_GError(&tmp_err);
		return;
	}
	free(ret);
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);	
	gfal_handle_freeG(handle);
}



void test__gfal_common_lfc_rename()
{
	GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	assert_true_with_message(handle!=NULL, "error must be initiated %s", gfal_string_GError(&tmp_err) );
	if(!handle)
		return;
	gfal_catalog_interface i = get_lfc_interface(handle, &tmp_err);	
	assert_true_with_message(tmp_err == NULL, "must be a valid init %s", gfal_string_GError(&tmp_err) );
	if(tmp_err)
		return;

#if USE_MOCK
	will_respond(lfc_mock_rename, 0, want_string(oldpath, TEST_LFC_RENAME_VALID_SRC+4), want_string(newpath, TEST_LFC_RENAME_VALID_DEST+4));
	will_respond(lfc_mock_rename, 0, want_string(oldpath, TEST_LFC_RENAME_VALID_DEST+4), want_string(newpath, TEST_LFC_RENAME_VALID_SRC+4));
	will_respond(lfc_mock_rename, ENOENT, want_string(oldpath, TEST_LFC_RENAME_VALID_DEST+4), want_string(newpath, TEST_LFC_RENAME_VALID_DEST+4));
	always_return(lfc_mock_rename, EINVAL);
#endif	

	
	ret = i.renameG(i.handle, TEST_LFC_RENAME_VALID_SRC, TEST_LFC_RENAME_VALID_DEST, &tmp_err);
	assert_true_with_message(ret >= 0 && tmp_err == NULL, " must be a success on the first rename %d %ld %s", ret,tmp_err);
	g_clear_error(&tmp_err);
	ret = i.renameG(i.handle, TEST_LFC_RENAME_VALID_DEST, TEST_LFC_RENAME_VALID_SRC, &tmp_err);
	assert_false_with_message(ret >= 0 && tmp_err == NULL, " must be a success on the second rename %d %ld ", ret,tmp_err);
	g_clear_error(&tmp_err);
	ret = i.renameG(i.handle, TEST_LFC_RENAME_VALID_DEST, TEST_LFC_RENAME_VALID_DEST, &tmp_err);
	assert_true_with_message(ret < 0 && tmp_err != NULL && tmp_err->code == ENOENT, " must be a success on the second rename %d %ld ", ret,tmp_err->code);
	g_clear_error(&tmp_err);	
	gfal_handle_freeG(handle);
}





void test__gfal_common_lfc_statg()
{
	GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		assert_true_with_message(FALSE, "error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = get_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		assert_true_with_message(FALSE, "must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}

#if USE_MOCK
	struct lfc_filestatg f;
	f.filemode = TEST_GFAL_LFC_FILE_STAT_MODE_VALUE;
	f.uid = TEST_GFAL_LFC_FILE_STAT_UID_VALUE ;
	f.gid=TEST_GFAL_LFC_FILE_STAT_GID_VALUE ;
	will_respond(lfc_mock_statg, 0, want_string(lfn, TEST_LFC_VALID_ACCESS+4));
	defined_filestatg = &f;
	will_respond(lfc_mock_statg, ENOENT, want_string(lfn, TEST_LFC_NOEXIST_ACCESS+4));
	will_respond(lfc_mock_statg, EACCES, want_string(lfn,  TEST_LFC_ONLY_READ_ACCESS+4));
	always_return(lfc_mock_statg, EINVAL);
#endif	
	
	struct stat buff;
	ret = i.statG(i.handle, TEST_LFC_VALID_ACCESS, &buff , &tmp_err);
	assert_true_with_message(ret >= 0 && tmp_err == NULL, " must be a success on the lfc valid %d ", ret);

	assert_false_with_message( buff.st_gid != TEST_GFAL_LFC_FILE_STAT_GID_VALUE , "must be a valid gid");
	assert_false_with_message(buff.st_uid != TEST_GFAL_LFC_FILE_STAT_UID_VALUE , "must be a valid uid");
	assert_false_with_message(buff.st_mode != TEST_GFAL_LFC_FILE_STAT_MODE_VALUE, "must be a valid mode");
	memset(&buff,0, sizeof(struct stat));
	g_clear_error(&tmp_err);	
	ret = i.statG(i.handle, TEST_LFC_NOEXIST_ACCESS, &buff , &tmp_err);
	assert_true_with_message( ret != 0 && tmp_err && tmp_err->code == ENOENT, "must be a non existing file : :%s ", gfal_string_GError(&tmp_err));
	g_clear_error(&tmp_err);
		
	ret = i.statG(i.handle, TEST_LFC_ONLY_READ_ACCESS, &buff , &tmp_err);
	assert_true_with_message(ret != 0 && tmp_err && tmp_err->code == EACCES, " must be a non existing accessible file : %d %ld %s ", ret, tmp_err, gfal_string_GError(&tmp_err));
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
}


