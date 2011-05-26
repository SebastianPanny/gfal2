
/* unit test for posix mkdir func */


#include <check.h>
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


START_TEST(test__mkdir_posix_lfc_simple)
{
	struct stat st;
	int ret = gfal_mkdir(TEST_LFC_EEXIST_MKDIR, 0664);
	if( ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){
		fail(" must be an existing dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;
	}
	gfal_posix_clear_error();
	errno ==0;
	
	char filename[2048];
	time_t tt;
	time(&tt);
	char *t = ctime(&tt);
	strcpy(filename, TEST_LFC_BASE_FOLDER_URL_MKDIR1);
	strcat(filename, t); // generate a new unique dir identifier&
	ret = gfal_mkdir(filename, 0664);
	if(ret != 0 || errno !=0 || gfal_posix_code_error() != 0){
		fail(" must be a valid create dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		fail(" bad right on the new created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}
	
	ret = gfal_mkdir(TEST_LFC_UNACCESS_MKDIR, 06640);
	if( ret ==0 || errno != EACCES || gfal_posix_code_error() != EACCES){
		fail(" must be a non-access dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;			
	}
	gfal_posix_clear_error();
	errno ==0;	
	
}
END_TEST


START_TEST(test__mkdir_posix_lfc_rec)
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
		fail(" must be a valid create rec dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		fail(" bad right on the new rec created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}
	
}
END_TEST

START_TEST(test__mkdir_posix_lfc_rec_with_slash)
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
		fail(" must be a valid create rec dir with / %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		fail(" bad right on the new rec created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}

	ret = gfal_mkdir(filename, 0664);	
	if(ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){ // try to recreate on the same call, must fail
		fail(" must be a failed creation %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
}
END_TEST


START_TEST(test__mkdir_posix_local_simple)
{
	struct stat st;
	system(TEST_LOCAL_MKDIR_EXIST_COMMAND);
	int ret = gfal_mkdir(TEST_LOCAL_MKDIR_EXIST_FILE, 0664);
	if( ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){
		fail(" must be an existing dir %d %d %d", ret, errno, gfal_posix_code_error());
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
		fail(" must be a valid create dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		fail(" bad right on the new created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}
	
	ret = gfal_mkdir(TEST_LOCAL_UNACCESS_MKDIR, 06640);
	if( ret ==0 || errno != EACCES || gfal_posix_code_error() != EACCES){
		fail(" must be a non-access dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;			
	}
	gfal_posix_clear_error();
	errno ==0;	
	
}
END_TEST


START_TEST(test__mkdir_posix_local_rec)
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
		fail(" must be a valid create rec dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		fail(" bad right on the new rec created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}
	
}
END_TEST


START_TEST(test__mkdir_posix_local_rec_with_slash)
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
		fail(" must be a valid create rec dir with / %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_stat(filename,&st);
	if( ret != 0 || st.st_mode != 040664){
		fail(" bad right on the new rec created directory %d %o ", ret,st.st_mode );
		gfal_posix_clear_error();
		return;			
	}

	ret = gfal_mkdir(filename, 0664);	
	if(ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){ // try to recreate on the same call, must fail
		fail(" must be a failed creation %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
}
END_TEST




START_TEST(test__mkdir_posix_srm_simple)
{
	struct stat st;
	int ret =-1;
	ret = gfal_mkdir(TEST_SRM_EEXIST_MKDIR, 0664);
	if( ret != 0 ){
		fail(" must be an existing dir %d %d %d", ret, errno, gfal_posix_code_error()); //---------> EEXIST ->  0 for srm
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
		fail(" must be a valid create dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	
	ret = gfal_access(filename, F_OK);
	if( ret != 0 ){
		fail(" directory must exist %d %o ", ret);
		gfal_posix_clear_error();
		return;			
	}
	
	ret = gfal_mkdir(TEST_SRM_UNACCESS_MKDIR, 0644);
	if( ret ==0 || errno != EACCES || gfal_posix_code_error() != EACCES){
		fail(" must be a non-access dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;			
	}
	gfal_posix_clear_error();
	errno ==0;	
	
}
END_TEST



START_TEST(test__mkdir_posix_srm_rec)
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
		fail(" must be a valid create rec dir %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	ret = gfal_access(filename, F_OK);
	if( ret != 0 ){
		fail(" directory must exist %d %o ", ret);
		gfal_posix_clear_error();
		return;			
	}	

}
END_TEST



START_TEST(test__mkdir_posix_srm_rec_with_slash)
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
		fail(" must be a valid create rec dir with / %d %d %d", ret, errno, gfal_posix_code_error());
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
	ret = gfal_access(filename, F_OK);
	if( ret != 0 ){
		fail(" directory must exist %d %o ", ret);
		gfal_posix_clear_error();
		return;			
	}	

/*
	ret = gfal_mkdir(filename, 0664);	
	if(ret == 0 || errno != EEXIST || gfal_posix_code_error() != EEXIST){ // try to recreate on the same call, must fail
		fail(" must be a failed creation %d %d %d", ret, errno, gfal_posix_code_error()); 								--> SRM Call return 0 when EEXIST call
		gfal_posix_clear_error();
		return;		
	}
	gfal_posix_clear_error();
	errno ==0;
*/	
}
END_TEST


