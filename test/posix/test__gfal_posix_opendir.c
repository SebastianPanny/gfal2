

/* unit test for opendir/readdir/closedir func */


#include <check.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "gfal_constants.h"
#include "gfal_prototypes.h"
#include "gfal_types.h"
#include <glib.h>
#include "../unit_test_constants.h"
#include "gfal_posix_api.h"
#include "../mock/gfal_srm_mock_test.h"




START_TEST(test__opendir_posix_local_simple)
{
	gfal_posix_clear_error();
	DIR* d = gfal_opendir(TEST_LOCAL_OPENDIR_OPEN_INVALID);
	if(d!=NULL|| gfal_posix_code_error() != ENOENT || errno != ENOENT){
		fail(" error, must be a non existing dir %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	d = gfal_opendir(TEST_LOCAL_OPENDIR_OPEN_NOACCESS);
	if(d!=NULL|| gfal_posix_code_error() != EACCES || errno != EACCES){
		fail(" error, must be a non accessible dir  %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}		
	gfal_posix_clear_error();	
	d = gfal_opendir(TEST_LOCAL_OPENDIR_OPEN);
	if(d==NULL|| gfal_posix_code_error() != 0){
		fail(" error, must be a valid open");
		gfal_posix_release_error();
		return;
	}
	
	int ret = gfal_closedir(d);
	if( ret != 0 || gfal_posix_code_error() != 0 || errno != 0){
		fail(" must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_closedir(d);
	if( ret == 0 || gfal_posix_code_error() == 0 || errno == 0){
		fail(" must be a non existing dir descriptor closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}
END_TEST



START_TEST(test__opendir_posix_lfc_simple)
{
	gfal_posix_clear_error();
	DIR* d = gfal_opendir(TEST_LFC_OPENDIR_OPEN_INVALID);
	if(d!=NULL|| gfal_posix_code_error() != ENOENT || errno != ENOENT){
		fail(" error, must be a non existing dir %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	gfal_posix_clear_error();
	d = gfal_opendir(TEST_LFC_OPENDIR_OPEN_NOACCESS);
	if(d!=NULL|| gfal_posix_code_error() != EACCES || errno != EACCES){
		fail(" error, must be a non accessible dir  %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}		
	gfal_posix_clear_error();	
	d = gfal_opendir(TEST_LFC_OPENDIR_OPEN);
	if(d==NULL|| gfal_posix_code_error() != 0){
		fail(" error, must be a valid open");
		gfal_posix_release_error();
		return;
	}
	
	int ret = gfal_closedir(d);
	if( ret != 0 || gfal_posix_code_error() != 0 || errno != 0){
		fail(" must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	
	ret = gfal_closedir(d);
	if( ret == 0 || gfal_posix_code_error() == 0 || errno == 0){
		fail(" must be a non existing dir descriptor closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();		
}
END_TEST

START_TEST(test__readdir_posix_local_simple)
{
	gfal_posix_clear_error();
	system(TEST_LOCAL_READDIR_CREATE_COMMAND);
	GList* list_dir = g_list_append(NULL, TEST_LOCAL_READDIR_1);
	list_dir = g_list_append(list_dir, TEST_LOCAL_READDIR_2);
	list_dir = g_list_append(list_dir, TEST_LOCAL_READDIR_3);
	list_dir = g_list_append(list_dir, TEST_LOCAL_READDIR_4);
	list_dir = g_list_append(list_dir, "..");
	list_dir = g_list_append(list_dir, ".");
	DIR* d = gfal_opendir(TEST_LOCAL_READDIR_VALID);
	if(d==NULL|| gfal_posix_code_error() != 0 || errno != 0){
		fail(" error, must be an existing dir %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	
	int count=0;
	struct dirent* dirs;
	do{
		 dirs = gfal_readdir(d);
		if(count < g_list_length(list_dir)){
			if(dirs==NULL|| gfal_posix_code_error() != 0 || errno != 0){
				fail(" error, must be a valid readdir %ld %d %d",dirs,gfal_posix_code_error(), errno);
				gfal_posix_release_error();
				return;
			}
			if( find_string(list_dir, dirs->d_name) == FALSE){
					fail(" error must be a valid dir name %s ",dirs->d_name );
					return;
			}	
			
			count +=1;
		}else{
			if(dirs != NULL || gfal_posix_code_error() != 0 || errno != 0){
				fail(" error, must be the end of the dir list %ld %d %d",dirs,gfal_posix_code_error(), errno);
				gfal_posix_release_error();
				return;				
			}
		}
	} while(dirs != NULL);
	if(count != g_list_length(list_dir)){
		fail(" must return the good number of directories");
		return;
	}

	int ret = gfal_closedir(d);
	if( ret != 0 || gfal_posix_code_error() != 0 || errno != 0){
		fail(" must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	dirs = gfal_readdir(d);
	if(dirs!=NULL|| gfal_posix_code_error() != EBADF || errno != EBADF){
		fail(" error, must be a bad descriptor file %ld %d %d",dirs,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}
END_TEST



START_TEST(test__readdir_posix_lfc_simple)
{
	gfal_posix_clear_error();
	GList* list_dir = g_list_append(NULL, TEST_LFC_READDIR_1);
	list_dir = g_list_append(list_dir, TEST_LFC_READDIR_2);
	list_dir = g_list_append(list_dir, TEST_LFC_READDIR_3);
	list_dir = g_list_append(list_dir, TEST_LFC_READDIR_4);
	DIR* d = gfal_opendir(TEST_LFC_READDIR_VALID);
	if(d==NULL|| gfal_posix_code_error() != 0 || errno != 0){
		fail(" error, must be an existing dir %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	
	int count=0;
	struct dirent* dirs;
	do{
		 dirs = gfal_readdir(d);
		if(count < g_list_length(list_dir)){
			if(dirs==NULL|| gfal_posix_code_error() != 0 || errno != 0){
				fail(" error, must be a valid readdir %ld %d %d",dirs,gfal_posix_code_error(), errno);
				gfal_posix_release_error();
				return;
			}
			if( find_string(list_dir, dirs->d_name) == FALSE){
					fail(" error must be a valid dir name %s ",dirs->d_name );
					return;
			}	
			
			count +=1;
		}else{
			if(dirs != NULL || gfal_posix_code_error() != 0 || errno != 0){
				fail(" error, must be the end of the dir list %ld %d %d",dirs,gfal_posix_code_error(), errno);
				gfal_posix_release_error();
				return;				
			}
		}
	} while(dirs != NULL);
	if(count != g_list_length(list_dir)){
		fail(" must return the good number of directories");
		return;
	}

	int ret = gfal_closedir(d);
	if( ret != 0 || gfal_posix_code_error() != 0 || errno != 0){
		fail(" must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}
	dirs = gfal_readdir(d);
	if(dirs!=NULL|| gfal_posix_code_error() != EBADF || errno != EBADF){
		fail(" error, must be a bad descriptor file %ld %d %d",dirs,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		return;
	}	
	gfal_posix_clear_error();	
}
END_TEST


START_TEST(test__opendir_posix_srm_simple_mock)
{
	mock_all_srm();
	mock_all_mds();

	gfal_posix_clear_error();
	DIR* d = gfal_opendir(srm_noent_dir());
	if(d!=NULL|| gfal_posix_code_error() != ENOENT || errno != ENOENT){
		fail(" error, must be a non existing dir %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}
	gfal_posix_clear_error();
	d = gfal_opendir(srm_noaccess_dir());
	if(d!=NULL|| gfal_posix_code_error() != EACCES || errno != EACCES){
		fail(" error, must be a non accessible dir  %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}		
	gfal_posix_clear_error();	
	d = gfal_opendir(srm_valid_dir());
	if(d==NULL|| gfal_posix_code_error() != 0){
		fail(" error, must be a valid open");
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}
	
	int ret = gfal_closedir(d);
	if( ret != 0 || gfal_posix_code_error() != 0 || errno != 0){
		fail(" must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}
	
	ret = gfal_closedir(d);
	if( ret == 0 || gfal_posix_code_error() == 0 || errno == 0){
		fail(" must be a non existing dir descriptor closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}	
	gfal_posix_clear_error();
	unmock_all_mds();
	unmock_all_srm();

}
END_TEST


START_TEST(test__readdir_posix_srm_simple_mock)
{
	mock_all_srm();
	mock_all_mds();

	int ret;
	gfal_posix_clear_error();
	DIR* d = gfal_opendir(srm_noent_dir()); // try to open a no existing dir
	if(d!=NULL|| gfal_posix_code_error() != ENOENT || errno != ENOENT){
		fail(" error, must be a non existing dir %ld %d %d",d,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}
	gfal_posix_clear_error();
	ret = gfal_closedir(d);		// try to close this dir, if success, error this file handle must not exist !
	if( ret == 0 || gfal_posix_code_error() == 0 || errno == 0){
		fail(" must be a non existing dir descriptor closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}		
	gfal_posix_clear_error();	
	d = gfal_opendir(srm_valid_dir());
	if(d==NULL|| gfal_posix_code_error() != 0){
		fail(" error, must be a valid open");
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}
	
	struct dirent* resu;
	int counter = 0;
	char** tab_resu = list_dir_srm();
	while( ( resu = gfal_readdir(d)) != NULL){
		fail_if( strcmp(resu->d_name, tab_resu[counter]) != 0, "fail, must be the same string");
		counter+=1;
	}
	fail_if(counter != count_dir_srm(), " bad number of file readed");
	
	ret = gfal_closedir(d);
	if( ret != 0 || gfal_posix_code_error() != 0 || errno != 0){
		fail(" must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}
	
	ret = gfal_closedir(d);
	if( ret == 0 || gfal_posix_code_error() == 0 || errno == 0){
		fail(" must be a non existing dir descriptor closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}	
	gfal_posix_clear_error();
	unmock_all_mds();
	unmock_all_srm();

}
END_TEST





START_TEST(test__readdir_posix_srm_empty_mock)
{
	mock_all_srm();
	mock_all_mds();

	int ret;
	gfal_posix_clear_error();	
	DIR* d = gfal_opendir(srm_valid_empty_dir());
	if(d==NULL|| gfal_posix_code_error() != 0){
		fail(" error, must be a valid open");
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}
	
	struct dirent* resu = gfal_readdir(d);
	fail_if(resu != NULL || gfal_posix_code_error() != 0 || errno != 0, " must return an empty dir");
	
	ret = gfal_closedir(d);
	if( ret != 0 || gfal_posix_code_error() != 0 || errno != 0){
		fail(" must be a valid closedir %d %d %d", ret,gfal_posix_code_error(), errno);
		gfal_posix_release_error();
		unmock_all_mds();
		unmock_all_srm();
		return;
	}
	
	gfal_posix_clear_error();
	unmock_all_mds();
	unmock_all_srm();

}
END_TEST







