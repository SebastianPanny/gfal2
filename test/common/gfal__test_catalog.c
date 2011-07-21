

/* unit test for common_catalog */


#include <cgreen/cgreen.h>
#include <stdio.h>
#include <stdlib.h>
#include "gfal_constants.h"
#include "gfal_common_catalog.h"
#include "gfal_common_errverbose.h"
#include "lfc/lfc_ifce_ng.h"
#include "../unit_test_constants.h"
#include "../mock/gfal_lfc_mock_test.h"


struct lfc_ops* find_lfc_ops(gfal_handle handle, GError** err){
	int n = handle->catalog_opt.catalog_number;
	int i;
	gboolean found=FALSE;
	for(i=0; i < n; ++i){
		if(strcmp(handle->catalog_opt.catalog_list[i].getName(), "lfc_plugin") == 0){
			found = TRUE;
			break;
		}
	}
	if(!found){
		g_set_error(err, 0, EINVAL, "[gfal] [mock] unable to load and replace the ops ");
		return NULL;
	}
	handle->catalog_opt.catalog_list[i].handle = calloc(1, sizeof(struct lfc_ops));
	struct lfc_ops* ops = (struct lfc_ops*) handle->catalog_opt.catalog_list[i].handle; 	
	return ops;
}

// mocking function internal to gfal
void test_mock_lfc(gfal_handle handle, GError** err){

#if USE_MOCK
	struct lfc_ops* ops = find_lfc_ops(handle, err); 
	ops->lfc_endpoint = NULL;
	ops->handle = handle;
	gfal_lfc_regex_compile(&(ops->rex), err);
	ops->statg = &lfc_mock_statg;
	ops->rename = &lfc_mock_rename;
	ops->serrno = &lfc_mock_C__serrno;
	ops->access = &lfc_mock_access;
	ops->sstrerror = &strerror;
	ops->getreplica = &lfc_mock_getreplica;
	ops->getlinks= &lfc_mock_getlinks;
	ops->lstat= &lfc_mock_lstatg;
	ops->chmod = &lfc_mock_chmod;
	ops->mkdirg = &lfc_mock_mkdir;
	ops->starttrans= &lfc_mock_starttrans;
	ops->endtrans= &lfc_mock_endtrans;
	ops->aborttrans= &lfc_mock_aborttrans;
#endif
	return;
}


void test_get_cat_type()
{
	char* cat_type;
	cat_type= gfal_get_cat_type(NULL);
	assert_true_with_message(strncmp(cat_type, GFAL_DEFAULT_CATALOG_TYPE, 50) == 0, " must be true");
	free(cat_type);
}



void test_env_var()
{
	char* cat_type;
	char* old_env;
	old_env = getenv ("LCG_CATALOG_TYPE");
	setenv("LCG_CATALOG_TYPE", "CATAX",1);
	cat_type= gfal_get_cat_type(NULL);
	assert_true_with_message(strncmp(cat_type, "CATAX", 50) == 0, " must be true");
	if(old_env != NULL)
		setenv("LCG_CATALOG_TYPE", old_env,1);	// restore old env
	free(cat_type);
	free(old_env);
}


void test_catalog_access_file()
{
	GError* tmp_err=NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	assert_true_with_message(handle != NULL, " must init properly");
	if(handle == NULL)	
		return;
	gfal_catalogs_instance(handle, &tmp_err);
	assert_true_with_message(tmp_err== NULL, " must be instance properly");
	if(tmp_err)
		return;
	test_mock_lfc(handle, &tmp_err);
	assert_true_with_message(tmp_err== NULL, " must be mocked properly");
	if(tmp_err)
			return;
#if USE_MOCK
	will_respond(lfc_mock_access, 0, want_string(path, TEST_LFC_VALID_ACCESS+4), want(mode, F_OK));
	will_respond(lfc_mock_access, ENOENT, want_string(path, TEST_LFC_NOEXIST_ACCESS+4), want(mode, F_OK));
	always_return(lfc_mock_access, EINVAL);
#endif
	int ret = gfal_catalogs_accessG(handle, TEST_LFC_VALID_ACCESS, F_OK, &tmp_err);
	assert_true_with_message(ret == 0, " must be a valid access");

	
	ret = gfal_catalogs_accessG(handle, TEST_LFC_NOEXIST_ACCESS, F_OK, &tmp_err);	
	assert_true_with_message(ret!=0 && tmp_err!=0 && tmp_err->code == ENOENT, " must be a non-existing file %d %ld %d", (int)ret, (long)tmp_err, (int) (tmp_err)?(tmp_err->code):0);
	
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
}



void test_catalog_url_checker()
{
	GError* tmp_err=NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle == NULL){
		assert_true_with_message(FALSE, " must init properly");
		gfal_release_GError(&tmp_err);
		return;
	}
	int ret = gfal_catalogs_accessG(handle, TEST_LFC_URL_SYNTAX_ERROR, F_OK, &tmp_err);
	if(ret ==0 || tmp_err->code != EPROTONOSUPPORT){
		assert_true_with_message(FALSE, " must be an invalid protocol");
		gfal_release_GError(&tmp_err);
		return;
	}	
	g_clear_error(&tmp_err);	
	gfal_handle_freeG(handle);	
}






void test_catalog_guid_resolve()
{
	GError* tmp_err=NULL;
	int i1;
	gfal_handle handle = gfal_initG(&tmp_err);
	assert_true_with_message(handle!=NULL, " must init properly");
	gfal_release_GError(&tmp_err);
	if(handle == NULL)
		return;
	
	gfal_catalogs_instance(handle, &tmp_err);
	assert_true_with_message(tmp_err==NULL, " must be a valid instance call");
	if(tmp_err)
		return;
	test_mock_lfc(handle, &tmp_err);
	assert_true_with_message(tmp_err==NULL, " must be a valid mock call");
	if(tmp_err)
		return;		
#if USE_MOCK
	will_respond(lfc_mock_getlinks, ENOENT, want_string(guid, TEST_GUID_NOEXIST_ACCESS+5), want(path,NULL), want_non_null(nbentries), want_non_null(linkinfos));
	define_linkinfos= calloc(sizeof(struct lfc_linkinfo),3);
	define_numberlinkinfos=3;
	for(i1=0; i1< define_numberlinkinfos; ++i1)
		g_strlcpy(define_linkinfos[i1].path, "lfn:/test/obiwankenobi", 2048);
	will_respond(lfc_mock_getlinks, 0, want_string(guid, TEST_GUID_VALID_ACCESS+5), want(path, NULL), want_non_null(nbentries), want_non_null(linkinfos));	
	always_return(lfc_mock_getlinks, EINVAL);
#endif

	char* ret = gfal_catalog_resolve_guid(handle, TEST_GUID_NOEXIST_ACCESS, &tmp_err);
	assert_true_with_message(ret==NULL && tmp_err!= NULL, " must be a non-valid guid");
	free(ret);
	g_clear_error(&tmp_err);	
	ret = gfal_catalog_resolve_guid(handle, TEST_GUID_VALID_ACCESS, &tmp_err);
	assert_true_with_message(ret!=NULL && tmp_err == NULL, " must be a success convertion");

	free(ret);
	g_clear_error(&tmp_err);

	gfal_handle_freeG(handle);
	
	
}



void test__catalog_stat()
{
	struct stat resu;
	memset(&resu, 0, sizeof(struct stat));
	GError* tmp_err=NULL;
	
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle == NULL){
		assert_true_with_message(FALSE, " must init properly");
		gfal_release_GError(&tmp_err);
		return;
	}	
	
	gfal_catalogs_instance(handle, &tmp_err);
	assert_true_with_message(tmp_err==NULL, " must be a valid instance call");
	if(tmp_err)
		return;
	test_mock_lfc(handle, &tmp_err);
	assert_true_with_message(tmp_err==NULL, " must be a valid mock call");
	if(tmp_err)
		return;	
		
#if USE_MOCK
	struct lfc_filestatg f;
	f.filemode = TEST_GFAL_LFC_FILE_STAT_MODE_VALUE;
	f.uid = TEST_GFAL_LFC_FILE_STAT_UID_VALUE ;
	f.gid=TEST_GFAL_LFC_FILE_STAT_GID_VALUE ;
	will_respond(lfc_mock_statg, 0, want_string(lfn, TEST_GFAL_LFC_FILE_STAT_OK+4));
	defined_filestatg = &f;
	will_respond(lfc_mock_statg, ENOENT, want_string(lfn, TEST_GFAL_LFC_FILE_STAT_NONEXIST+4));
	always_return(lfc_mock_statg, EINVAL);
#endif	

	int ret = gfal_catalog_statG(handle, TEST_GFAL_LFC_FILE_STAT_OK, &resu, &tmp_err);
	assert_true_with_message(ret==0 && tmp_err==NULL, " must be a success statg");
	assert_true_with_message(resu.st_mode == TEST_GFAL_LFC_FILE_STAT_MODE_VALUE &&
		resu.st_gid== TEST_GFAL_LFC_FILE_STAT_GID_VALUE
		&& resu.st_uid==TEST_GFAL_LFC_FILE_STAT_UID_VALUE, " invalid params in the  statg resu %o %d %d", resu.st_mode, resu.st_gid,resu.st_uid );	
	g_clear_error(&tmp_err);
	//g_printerr(" extract from the stat struct : right : %o, owner : %d, group : %d, size : %lu", resu.st_mode, resu.st_uid, resu.st_gid, resu.st_size);	
	ret = gfal_catalog_statG(handle, TEST_GFAL_LFC_FILE_STAT_NONEXIST, &resu, &tmp_err);
	assert_true_with_message(ret!= 0 && tmp_err!= NULL && tmp_err->code ==ENOENT, " must be a failure %d %d", ret, tmp_err);

	
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
	
}



void test__catalog_lstat()
{
	struct stat resu;
	memset(&resu, 0, sizeof(struct stat));
	GError* tmp_err=NULL;
	
	gfal_handle handle = gfal_initG(&tmp_err);
	assert_true_with_message(handle != NULL, " must init properly");
	if(handle == NULL)
		return;

	gfal_catalogs_instance(handle, &tmp_err);
	assert_true_with_message(tmp_err==NULL, " must be a valid instance call");
	if(tmp_err)
		return;
	test_mock_lfc(handle, &tmp_err);
	assert_true_with_message(tmp_err==NULL, " must be a valid mock call");
	if(tmp_err)
		return;	
	
#if USE_MOCK
	struct lfc_filestat f;
	f.filemode = TEST_GFAL_LFC_FILE_STAT_MODE_VALUE;
	f.uid = TEST_GFAL_LFC_FILE_STAT_UID_VALUE ;
	f.gid=TEST_GFAL_LFC_FILE_STAT_GID_VALUE ;
	will_respond(lfc_mock_lstatg, 0, want_string(lfn, TEST_GFAL_LFC_FILE_STAT_OK+4));
	defined_filestat = &f;
	will_respond(lfc_mock_lstatg, 0, want_string(lfn, TEST_GFAL_LFC_LINK_STAT_OK+4));
	defined_filestat = &f;
	will_respond(lfc_mock_lstatg, ENOENT, want_string(lfn, TEST_GFAL_LFC_FILE_STAT_NONEXIST+4));
	always_return(lfc_mock_lstatg, EINVAL);
#endif	
	

	int ret = gfal_catalog_lstatG(handle, TEST_GFAL_LFC_FILE_STAT_OK, &resu, &tmp_err);
	assert_true_with_message(ret==0 && tmp_err==NULL, " must be a success convertion");


	ret = gfal_catalog_lstatG(handle, TEST_GFAL_LFC_LINK_STAT_OK, &resu, &tmp_err);
	assert_true_with_message(ret==0 && tmp_err==NULL, " must be a success convertion");

	
	//g_printerr(" extract from the stat struct : right : %o, owner : %d, group : %d, size : %lu", resu.st_mode, resu.st_uid, resu.st_gid, resu.st_size);	
	ret = gfal_catalog_lstatG(handle, TEST_GFAL_LFC_FILE_STAT_NONEXIST, &resu, &tmp_err);
	assert_true_with_message(ret != 0 && tmp_err!=NULL && tmp_err->code == ENOENT, " must be a failure");
	
	
	g_clear_error(&tmp_err);
	gfal_handle_freeG(handle);
	
}
