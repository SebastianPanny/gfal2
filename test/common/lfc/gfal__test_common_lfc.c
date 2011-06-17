/*
 *  lfc test file 
 * 
 * */
 
 
#include <check.h>
#include <glib.h>
#include "lfc/gfal_common_lfc.h"
#include "lfc/lfc_ifce_ng.h"
#include "gfal_common_internal.h"
#include "../../unit_test_constants.h"
#include "../../mock/gfal_lfc_mock_test.h"




START_TEST(test_gfal_common_lfc_define_env)
{
	char* old_host = getenv("LFC_HOST");
	char* old_port = getenv("LFC_PORT");
	GError * tmp_err=NULL;
	int ret =0;
	char* resu = NULL;
	
	setenv("LFC_HOST", "google.com",1);
	setenv("LFC_PORT", "4465488896645546564",1);

	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle == NULL){
		fail(" handle must be initiated properly ");
		return;
	}
	resu = gfal_setup_lfchost(handle, &tmp_err);
	if(resu){
		fail(" must fail, port invalid");
		return;	
	}
	errno = 0;
	g_clear_error(&tmp_err);
	tmp_err=NULL;
	setenv("LFC_PORT", "2000",1);	
	resu = gfal_setup_lfchost(handle, &tmp_err);	// re-test with good port number
	if(resu == NULL){
		fail(" must be a success, LFC_HOST & LFC_PORT defined");
		gfal_release_GError(&tmp_err);
		return;
	}
	char *new_host = getenv("LFC_HOST");
	char *new_port = getenv("LFC_PORT");
	ret = strcmp(new_port, "2000") | strcmp(new_host, "google.com");
	if(ret){
		fail("must be the same string");
	}
	
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
END_TEST

START_TEST(test_gfal_common_lfc_resolve_sym)
{
	GError* err = NULL;
	struct lfc_ops* st = gfal_load_lfc("liblfc.so", &err);
	if(st == NULL || err){
		fail(" must be a valid resolution");
		return;
	}
	if(st->sstrerror == NULL){
		fail("must be a valid symbol");
	}
	char* str = NULL;
	str = st->sstrerror(0);
	if(str ==NULL){
		fail(" must be a valid string");
		
	}
	free(st);
}
END_TEST



START_TEST(test_gfal_common_lfc_init)
{
	GError * tmp_err=NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		fail(" error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = mock_lfc_interface(handle, &tmp_err);
	if(tmp_err){
		fail(" must not fail, valid value");
		return;
	}
	struct lfc_ops* op = (struct lfc_ops*) i.handle;
	free(op->lfc_endpoint);		// manual deletion
	free(op);
	gfal_handle_freeG(handle);
}
END_TEST


START_TEST(test_gfal_common_lfc_access){
	GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		fail(" error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = mock_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		fail(" must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	ret = i.accessG(i.handle, TEST_LFC_VALID_ACCESS, R_OK, &tmp_err);
	if(ret !=0){
		fail(" must be a valid access");
		gfal_release_GError(&tmp_err);
		return;
	}
	g_clear_error(&tmp_err);
	ret = i.accessG(i.handle, TEST_LFC_VALID_ACCESS, W_OK, &tmp_err);	
	if(ret == 0 || tmp_err->code != EACCES){
		fail(" must fail, unable to write this file");
		g_printerr(" file access report : %s ", strerror(ret));
		gfal_release_GError(&tmp_err);
		return;
	}
	g_clear_error(&tmp_err);
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);
	gfal_handle_freeG(handle);	
}
END_TEST


START_TEST(test_gfal_common_lfc_no_exist)
{
	GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		fail(" error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = mock_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		fail(" must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	ret = i.accessG(i.handle, TEST_LFC_NOEXIST_ACCESS, F_OK, &tmp_err);
	if(ret ==0 || tmp_err->code != ENOENT){
		fail(" must fail, this file not exist");
		gfal_release_GError(&tmp_err);
		return;
	}
	g_clear_error(&tmp_err);	
	ret = i.accessG(i.handle, TEST_LFC_VALID_ACCESS, F_OK, &tmp_err);
	if(ret !=0){
		fail("must be a success, file is present");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);		
	gfal_handle_freeG(handle);
}
END_TEST


START_TEST(test_gfal_common_lfc_check_filename)
{
GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		fail(" error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = mock_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		fail(" must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	gboolean b = i.check_catalog_url(i.handle, TEST_LFC_VALID_ACCESS, GFAL_CATALOG_ACCESS, &tmp_err);
	if(!b || tmp_err){
		fail(" must be a valid lfn url");
		return;
	}
	b = i.check_catalog_url(i.handle, TEST_LFC_NOEXIST_ACCESS, GFAL_CATALOG_ACCESS, &tmp_err);
	if(!b || tmp_err){
		fail(" must be a valid lfn url 2");
		return;
	}
	b = i.check_catalog_url(i.handle, TEST_LFC_URL_SYNTAX_ERROR, GFAL_CATALOG_ACCESS, &tmp_err);
	if(b){
		fail(" must an invalid lfn url 3");
		return;
	}
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);
	gfal_handle_freeG(handle);	
}
END_TEST

START_TEST(test_gfal_common_lfc_getSURL)
{
	GError * tmp_err=NULL;
	char** ret =NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		fail("error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = mock_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		fail("must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}	
	ret = i.getSURLG(i.handle, TEST_LFC_VALID_ACCESS, &tmp_err);
	if(ret == NULL || tmp_err){
		g_printerr(" errno : %s ", strerror(tmp_err->code));
		fail("must be a successfull convert");
		gfal_release_GError(&tmp_err);
		return;
	}
	char** p = ret;
	while(*p != NULL){
		fail_if( strncmp(*p,"srm://",6) !=0, " begin of the surl is incorrect : %s ", p);
		p++;
	}
	g_strfreev(ret);
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);	
	gfal_handle_freeG(handle);	
	
}
END_TEST


START_TEST(test_gfal_common_lfc_access_guid_file_exist)
{
	GError * tmp_err=NULL;
	char* ret =NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		fail("error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = mock_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		fail("must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	ret = i.resolve_guid(i.handle, TEST_GUID_NOEXIST_ACCESS, &tmp_err);
	if(ret !=NULL || tmp_err->code != ENOENT){
		fail("must fail, this file not exist");
		gfal_release_GError(&tmp_err);
		return;
	}
	free(ret);
	g_clear_error(&tmp_err);
	ret = i.resolve_guid(i.handle, TEST_GUID_VALID_ACCESS, &tmp_err);
	if(ret == NULL || tmp_err){
		g_printerr(" errno : %s ", strerror(tmp_err->code));
		fail("must be a success, file is present");
		gfal_release_GError(&tmp_err);
		return;
	}
	free(ret);
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);	
	gfal_handle_freeG(handle);
}
END_TEST


START_TEST(test__gfal_common_lfc_rename)
{
	GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		fail("error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = mock_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		fail("must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}	
	ret = i.renameG(i.handle, TEST_LFC_RENAME_VALID_SRC, TEST_LFC_RENAME_VALID_DEST, &tmp_err);
	if( ret < 0 || tmp_err){
		fail(" must be a success on the first rename");
		gfal_release_GError(&tmp_err);
		return;
	}
	ret = i.renameG(i.handle, TEST_LFC_RENAME_VALID_DEST, TEST_LFC_RENAME_VALID_SRC, &tmp_err);
	if( ret < 0 || tmp_err){
		fail(" must be a success on the second rename ");
		gfal_release_GError(&tmp_err);
		return;
	}
	struct lfc_ops* op = (struct lfc_ops*) i.handle; // manual deletion
	free(op->lfc_endpoint);
	free(op);	
	gfal_handle_freeG(handle);
}
END_TEST




START_TEST(test__gfal_common_lfc_statg_mock)
{
	GError * tmp_err=NULL;
	int ret =-1;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle==NULL){
		fail("error must be initiated");
		gfal_release_GError(&tmp_err);
		return;
	}
	gfal_catalog_interface i = mock_lfc_interface(handle, &tmp_err);	
	if(tmp_err){
		fail("must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	struct stat buff;	
	ret = i.statG(i.handle, lfc_valid_val(), &buff , &tmp_err);
	if( ret < 0 || tmp_err){
		fail(" must be a success on the lfc valid %d ", ret);
		gfal_release_GError(&tmp_err);
		return;
	}
	fail_if( buff.st_gid != lfc_valid_gid(), "must be a valid gid");
	fail_if(buff.st_uid != lfc_valid_uid(), "must be a valid uid");
	fail_if(buff.st_mode != lfc_valid_mode(), "must be a valid uid");
	memset(&buff,0, sizeof(struct stat));
		
	ret = i.statG(i.handle, lfc_enoent_val(), &buff , &tmp_err);
	if( ret == 0 || !tmp_err || tmp_err->code != ENOENT){
		fail(" must be a non existing file ");
		gfal_release_GError(&tmp_err);
		return;
	}
	g_clear_error(&tmp_err);
		
	ret = i.statG(i.handle, lfc_eacces_val(), &buff , &tmp_err);
	if( ret == 0 || !tmp_err || tmp_err->code != EACCES){
		fail(" must be a non existing accessible file ");
		gfal_release_GError(&tmp_err);
		return;
	}		
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
}
END_TEST

