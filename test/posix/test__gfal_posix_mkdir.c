
/* unit test for posix mkdir func */


#include <cgreen/cgreen.h>
#include <time.h>
#include <string.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include <glib.h>
#include "../unit_test_constants.h"
#include <stdio.h>
#include "gfal_posix_api.h"
#include <errno.h>


void test__mkdir_posix_lfc_simple()
{
	char filename[2048];
	time_t tt;
	time(&tt);
	char *t = ctime(&tt);
	strcpy(filename, TEST_LFC_BASE_FOLDER_URL_MKDIR1);
	strcat(filename, t); // generate a new unique dir identifier&
#if USE_MOCK
	GError* mock_err=NULL;
	gfal_handle handle = gfal_posix_instance();
	gfal_catalogs_instance(handle,NULL);
	test_mock_lfc(handle, &mock_err);
	if( gfal_check_GError(&mock_err))
		return;

	define_mock_filestatg(040664, 1,2);
	always_return(lfc_mock_endtrans,0);
	always_return(lfc_mock_starttrans,0);
	will_respond(lfc_mock_mkdir, EEXIST, want_string(path, TEST_LFC_EEXIST_MKDIR+4), want_non_null(guid), want(mode, 0664));
	will_respond(lfc_mock_mkdir, 0, want_string(path, filename+4), want_non_null(guid), want(mode, 0664));
	will_respond(lfc_mock_statg, 0, want_string(path, filename+4), want(mode, 0664));
	will_respond(lfc_mock_mkdir, EACCES, want_string(path, TEST_LFC_UNACCESS_MKDIR+4), want_non_null(guid), want(mode, 0664));
	always_return(lfc_mock_mkdir, EINVAL);
	always_return(lfc_mock_statg, EINVAL);
#endif
	struct stat st;
	int ret = gfal_mkdir(TEST_LFC_EEXIST_MKDIR, 0664);
	assert_true_with_message(ret ==-1 && errno==EEXIST && gfal_posix_code_error()==EEXIST, " must be an already exist file");
	gfal_posix_clear_error();
	errno ==0;
	

	ret = gfal_mkdir(filename, 0664);
	assert_true_with_message( ret ==0 && errno==0 && gfal_posix_code_error() ==0, " must be a valid mkdir");
	gfal_posix_check_error();
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	assert_true_with_message(ret ==0 && st.st_mode== 040664, " must be the correct right and dir must exist");
	
	ret = gfal_mkdir(TEST_LFC_UNACCESS_MKDIR, 0664);
	assert_true_with_message(ret == -1 && errno== EACCES && gfal_posix_code_error()== EACCES, " must be a bad access right");
	gfal_posix_clear_error();
	errno ==0;	
	
}



void test__mkdir_posix_lfc_rec()
{
	struct stat st;
	int ret,i;
	
	char filename[2048];
	
	g_strlcpy(filename, TEST_LFC_BASE_FOLDER_URL_MKDIR1,2048); // copy the base filenameurl
	time_t tt;
	time(&tt);	
	for(i=0; i< 5; i++){
		char buffer[2048];
		snprintf(buffer,2048, "%stest%ld%d/", filename, tt, i); // concat string to construct a full url
		strcpy(filename, buffer);
	}
	*(filename + strlen(filename)-1)='\0';
	
	//g_printerr(" filename : %s ", filename);
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		assert_true_with_message(FALSE, " must be a valid create rec dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		assert_true_with_message(FALSE, " bad right on the new rec created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}
	
}


void test__mkdir_posix_lfc_rec_with_slash()
{
	struct stat st;
	int ret,i;
	
	char filename[2048];
	
	g_strlcpy(filename, TEST_LFC_BASE_FOLDER_URL_MKDIR1,2048); // copy the base filenameurl
	time_t tt;
	time(&tt);	
	for(i=100; i< 105; i++){
		char buffer[2048];
		snprintf(buffer,2048, "%stest%ld%d/", filename, tt, i); // concat string to construct a full url
		strcpy(filename, buffer);
	}
	*(filename + strlen(filename)-1)='/';
	
	//g_printerr(" filename : %s ", filename);
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		assert_true_with_message(FALSE, " must be a valid create rec dir with / %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		assert_true_with_message(FALSE, " bad right on the new rec created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}

	ret = gfal_mkdir(filename, 0664);	
	if(ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){ // try to recreate on the same call, must fail
		assert_true_with_message(FALSE, " must be a failed creation %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
}



void test__mkdir_posix_local_simple()
{
	struct stat st;
	system(TEST_LOCAL_MKDIR_EXIST_COMMAND);
	int ret = gfal_mkdir(TEST_LOCAL_MKDIR_EXIST_FILE, 0664);
	if( ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){
		assert_true_with_message(FALSE, " must be an existing dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;
	}
	gfal_posix_clear_error();
	errno ==0;
	
	char filename[2048];
	time_t tt;
	time(&tt);
	snprintf(filename, 2048, "%s%ld", TEST_LOCAL_BASE_FOLDER_URL_MKDIR1, tt);
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		assert_true_with_message(FALSE, " must be a valid create dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		assert_true_with_message(FALSE, " bad right on the new created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}
	
	ret = gfal_mkdir(TEST_LOCAL_UNACCESS_MKDIR, 06640);
	if( ret ==0 || errno != EACCES || gfal_posix_code_error() != EACCES){
		assert_true_with_message(FALSE, " must be a non-access dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;			
	}
	gfal_posix_clear_error();
	errno ==0;	
	
}



void test__mkdir_posix_local_rec()
{
	struct stat st;
	int ret,i;
	
	char filename[2048];
	
	g_strlcpy(filename, TEST_LOCAL_BASE_FOLDER_URL_MKDIR1,2048); // copy the base filenameurl
	time_t tt;
	time(&tt);		
	for(i=0; i< 5; i++){
		char buffer[2048];
		snprintf(buffer,2048, "%stest%ld%d/", filename, tt, i); // concat string to construct a full url
		strcpy(filename, buffer);
	}
	*(filename + strlen(filename)-1)='\0';
	
	//g_printerr(" filename : %s ", filename);
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		assert_true_with_message(FALSE, " must be a valid create rec dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		assert_true_with_message(FALSE, " bad right on the new rec created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}
	
}



void test__mkdir_posix_local_rec_with_slash()
{
	struct stat st;
	int ret,i;
	
	char filename[2048];
	char name[2048];
	
	g_strlcpy(filename, TEST_LOCAL_BASE_FOLDER_URL_MKDIR1,2048); // copy the base filenameurl
	time_t tt;
	time(&tt);	
	for(i=100; i< 105; i++){
		char buffer[2048];
		snprintf(buffer,2048, "%stest%ld%d/", filename, tt, i); // concat string to construct a full url
		strcpy(filename, buffer);
	}
	*(filename + strlen(filename)-1)='/';
	
	//g_printerr(" filename : %s ", filename);
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		assert_true_with_message(FALSE, " must be a valid create rec dir with / %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		assert_true_with_message(FALSE, " bad right on the new rec created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}

	ret = gfal_mkdir(filename, 0664);	
	if(ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){ // try to recreate on the same call, must fail
		assert_true_with_message(FALSE, " must be a failed creation %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
}





void test__mkdir_posix_srm_simple()
{
	struct stat st;
	int ret =-1;
	ret = gfal_mkdir(TEST_SRM_EEXIST_MKDIR, 0664);
	if( ret != 0 ){
		assert_true_with_message(FALSE, " must be an existing dir %d %d %d", ret, errno, gfal_posix_code_error()); //---------> EEXIST ->  0 for srm
		gfal_posix_clear_error();
		return;
	}
	gfal_posix_clear_error();
	errno ==0;
	
	char filename[2048];
	time_t tt;
	time(&tt);
	snprintf(filename, 2048, "%stest%ld", TEST_SRM_BASE_FOLDER_URL_MKDIR1, tt);
	//g_printerr(" filename %s ", filename);
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		assert_true_with_message(FALSE, " must be a valid create dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_access(filename, F_OK);
	if( ret != 0 ){
		assert_true_with_message(FALSE, " directory must exist %d %o ", ret);
		gfal_posix_clear_error();
		return;			
	}
	
	ret = gfal_mkdir(TEST_SRM_UNACCESS_MKDIR, 0644);
	if( ret ==0 || errno != EACCES || gfal_posix_code_error() != EACCES){
		assert_true_with_message(FALSE, " must be a non-access dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;			
	}
	gfal_posix_clear_error();
	errno ==0;	
	
}




void test__mkdir_posix_srm_rec()
{
	struct stat st;
	int ret,i;
	
	char filename[2048];
	
	g_strlcpy(filename, TEST_SRM_BASE_FOLDER_URL_MKDIR1,2048); // copy the base filenameurl
	time_t tt;
	time(&tt);		
	for(i=0; i< 5; i++){
		char buffer[2048];
		snprintf(buffer,2048, "%stest%ld%d/", filename, tt, i); // concat string to construct a full url
		strcpy(filename, buffer);
	}
	*(filename + strlen(filename)-1)='\0';
	
	//g_printerr(" filename : %s ", filename);
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		assert_true_with_message(FALSE, " must be a valid create rec dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	ret = gfal_access(filename, F_OK);
	if( ret != 0 ){
		assert_true_with_message(FALSE, " directory must exist %d %o ", ret);
		gfal_posix_clear_error();
		return;			
	}	

}




void test__mkdir_posix_srm_rec_with_slash()
{
	struct stat st;
	int ret,i;
	
	char filename[2048];
	char name[2048];
	
	g_strlcpy(filename, TEST_SRM_BASE_FOLDER_URL_MKDIR1,2048); // copy the base filenameurl
	time_t tt;
	time(&tt);	
	for(i=100; i< 105; i++){
		char buffer[2048];
		snprintf(buffer,2048, "%stest%ld%d/", filename, tt, i); // concat string to construct a full url
		strcpy(filename, buffer);
	}
	*(filename + strlen(filename)-1)='/';
	
	//g_printerr(" filename : %s ", filename);
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		assert_true_with_message(FALSE, " must be a valid create rec dir with / %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	ret = gfal_access(filename, F_OK);
	if( ret != 0 ){
		assert_true_with_message(FALSE, " directory must exist %d %o ", ret);
		gfal_posix_clear_error();
		return;			
	}	

/*
	ret = gfal_mkdir(filename, 0664);	
	if(ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){ // try to recreate on the same call, must fail
		assert_true_with_message(FALSE, " must be a failed creation %d %d %d", ret, errno, gfal_posix_code_error()); 								--> SRM Call return 0 when EEXIST call
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
*/	
}
