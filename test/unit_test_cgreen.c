/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 





/**
 * Unit tests for gfal ased on the cgreen library
 * @author : Devresse Adrien
 * @version : 0.0.1
 */

#include <cgreen/cgreen.h>
#include <stdio.h>
#include <stdlib.h>
#include "unit_test_constants.h"
#include "common/gfal_common.h"
#include "common/gfal_common_errverbose.h"
#include "common/gfal__test_verbose.h"
#include "common/gfal__test_catalog.h"
#include "common/voms/gfal__test_voms.h"
#include "common/gfal__test_common_srm.h"
#include "common/gfal__test_common_srm_access.h"
#include "common/mds/gfal__test_common_mds.h"
#include "common/lfc/gfal__test_common_lfc.h"
#include "common/gfal__test_common_srm_no_glib.h"
#include "common/gfal__test_common_dir_file_descriptor.h"


TestSuite * verbose_suite (void)
{
   TestSuite *s1 = create_test_suite();
  // verbose test case /
   add_test(s1, test_verbose_set_get);
   return s1;
 }
 
 
TestSuite * catalog_suite (void)
{
	TestSuite *s2 = create_test_suite();
	// verbose test case /
	add_test (s2, test_get_cat_type);
	add_test(s2, test_catalog_access_file);
	add_test(s2, test_catalog_url_checker);
	add_test(s2, test_catalog_guid_resolve);
	add_test(s2, test__catalog_stat);
	add_test(s2, test__catalog_lstat);
	return s2;
 }
 
 
TestSuite* voms_Suite(){
	TestSuite* tc_voms = create_test_suite();
	add_test (tc_voms, test_voms_parse_args);
	add_test (tc_voms, test_voms_info_is_null);
	add_test (tc_voms, test_voms_info_test_vo);
	add_test (tc_voms,  test_voms_get_userdnG);
	return tc_voms;	
}
 
 
TestSuite* srm_Suite(){
	TestSuite *tc_srm = create_test_suite();
	add_test(tc_srm, test_create_srm_handle);
	add_test(tc_srm, test__gfal_convert_full_surl);
	add_test(tc_srm, test_gfal_get_hostname_from_surl);
	add_test(tc_srm, test_gfal_full_endpoint_checkG);
	add_test(tc_srm, test_gfal_get_endpoint_and_setype_from_bdiiG);
	add_test(tc_srm, test_gfal_select_best_protocol_and_endpointG);
	add_test(tc_srm, test_gfal_srm_determine_endpoint_full_endpointG);
	add_test(tc_srm, test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdiiG);
	add_test(tc_srm, test_gfal_srm_determine_endpoint_not_fullG);
	add_test(tc_srm, test_gfal_srm_getTURLS_one_success);
	add_test(tc_srm, test_gfal_srm_getTURLS_bad_urls);
	add_test(tc_srm, test_gfal_srm_getTURLS_pipeline_success);
	return tc_srm;

}


TestSuite* no_glib_suite(){
	TestSuite* tc_srm_no_glib = create_test_suite();
	add_test(tc_srm_no_glib, test_srm_api_no_glib_full);
	return tc_srm_no_glib;
}

TestSuite* mds_suite(){
	TestSuite *tc_mds= create_test_suite();
	add_test(tc_mds, test_check_bdii_endpoints_srm);
	add_test(tc_mds, gfal__test_get_lfchost_bdii);
	add_test(tc_mds, gfal__test_get_lfchost_bdii_with_nobdii);	
	return tc_mds;
}

TestSuite* lfc_suite(){
	TestSuite *tc_lfc= create_test_suite();
	add_test(tc_lfc, test_gfal_common_lfc_resolve_sym);
	add_test(tc_lfc, test_gfal_common_lfc_define_env);
	add_test(tc_lfc, test_gfal_common_lfc_init);
	add_test(tc_lfc,  test__gfal_common_lfc_statg);	
	add_test(tc_lfc, test__gfal_common_lfc_rename);
	add_test(tc_lfc, test_gfal_common_lfc_access);
	add_test(tc_lfc, test_gfal_common_lfc_no_exist);
	add_test(tc_lfc, test_gfal_common_lfc_check_filename);
	add_test(tc_lfc, test_gfal_common_lfc_getSURL);
	add_test(tc_lfc, test_gfal_common_lfc_access_guid_file_exist);
	return tc_lfc;
}

TestSuite* filedesc_suite(){
	TestSuite *tc_filedesc = create_test_suite();
	add_test(tc_filedesc, test__dir_file_descriptor_low);
	add_test(tc_filedesc, test__dir_file_descriptor_high);
	return tc_filedesc;
}

/*
Suite* posix_suite (void)
{
  Suite *s = suite_create ("Posix :");
  TestSuite* tc_access = create_test_suite();
  add_test(tc_access, test_access_posix_guid_exist);  
  add_test(tc_access, test_access_posix_guid_read);
  add_test(tc_access, test_access_posix_guid_write);
  add_test(tc_access, test_access_posix_lfn_exist);
  add_test(tc_access, test_access_posix_lfn_read);
  add_test(tc_access, test_access_posix_lfn_write);
  add_test(tc_access, test_access_posix_srm_exist);
  add_test(tc_access, test_access_posix_srm_read);
  add_test(tc_access, test_access_posix_srm_write);
  suite_add_tcase(s, tc_access);
  TestSuite* tc_chmod = create_test_suite();
  add_test(tc_chmod, test__gfal_posix_chmod_read_lfn);
  add_test(tc_chmod, test__gfal_posix_chmod_read_guid);
  add_test(tc_chmod, test__gfal_posix_chmod_read_local);
  add_test(tc_chmod, test__gfal_posix_chmod_write_lfn);
  suite_add_tcase(s, tc_chmod);
  TestSuite* tc_rename = create_test_suite();
  add_test(tc_rename, test__gfal_posix_rename_catalog);
  add_test(tc_rename, test__gfal_posix_move_dir_catalog);
  add_test(tc_rename, test__gfal_posix_rename_url_check);
  add_test(tc_rename, test__gfal_posix_rename_local);
  suite_add_tcase(s, tc_rename);
  TestSuite* tc_stat = create_test_suite();
  add_test(tc_stat, test__gfal_posix_stat_lfc);
  add_test(tc_stat, test__gfal_posix_stat_guid);
  add_test(tc_stat, test__gfal_posix_stat_local);
  add_test(tc_stat, test__gfal_posix_stat_srm);
  add_test(tc_stat, test__gfal_posix_lstat_lfc);
  add_test(tc_stat, test__gfal_posix_lstat_guid);
  add_test(tc_stat, test__gfal_posix_lstat_local);
  add_test(tc_stat, test__gfal_posix_lstat_srm);
  suite_add_tcase(s, tc_stat);
  TestSuite* tc_mkdir = create_test_suite();
  add_test(tc_mkdir, test__mkdir_posix_lfc_simple);
  add_test(tc_mkdir, test__mkdir_posix_lfc_rec);
  add_test(tc_mkdir, test__mkdir_posix_lfc_rec_with_slash);
  add_test(tc_mkdir, test__mkdir_posix_local_simple);
  add_test(tc_mkdir, test__mkdir_posix_local_rec);
  add_test(tc_mkdir, test__mkdir_posix_local_rec_with_slash);
  add_test(tc_mkdir, test__mkdir_posix_srm_simple);
  add_test(tc_mkdir, test__mkdir_posix_srm_rec);
  add_test(tc_mkdir, test__mkdir_posix_srm_rec_with_slash);
  suite_add_tcase(s, tc_mkdir);
  TestSuite* tc_rmdir = create_test_suite();
  add_test(tc_rmdir, test__rmdir_posix_lfc_simple);
  add_test(tc_rmdir, test__rmdir_posix_lfc_existingfile);
  add_test(tc_rmdir, test__rmdir_posix_lfc_slash);
  add_test(tc_rmdir, test__rmdir_posix_srm_simple);
  add_test(tc_rmdir, test__rmdir_posix_srm_existingfile);
  add_test(tc_rmdir, test__rmdir_posix_srm_slash);
  add_test(tc_rmdir, test__rmdir_posix_local_simple);
  add_test(tc_rmdir, test__rmdir_posix_local_existingfile);
  add_test(tc_rmdir, test__rmdir_posix_local_slash);
  suite_add_tcase(s, tc_rmdir);
  TestSuite* tc_opendir = create_test_suite();
  add_test(tc_opendir, test__opendir_posix_local_simple);
  add_test(tc_opendir, test__opendir_posix_lfc_simple);
  add_test(tc_opendir, test__readdir_posix_local_simple);
  add_test(tc_opendir, test__readdir_posix_lfc_simple);
  add_test(tc_opendir, test__opendir_posix_srm_simple_mock);
  add_test(tc_opendir, test__readdir_posix_srm_simple_mock);
  add_test(tc_opendir, test__readdir_posix_srm_empty_mock);
  suite_add_tcase(s, tc_opendir);
  TestSuite* tc_open = create_test_suite();
  add_test(tc_open, test_open_posix_all_simple);
  add_test(tc_open, test_open_posix_local_simple);
  add_test(tc_open, test_open_posix_lfc_simple);
  add_test(tc_open, test_open_posix_srm_simple);
  add_test(tc_open, test_open_posix_guid_simple);
  suite_add_tcase(s, tc_open);
  return s;
}
*/

/*
Suite* posix_real_suite (void)
{
  Suite *s = suite_create ("Posix REAL tests:");
  TCase* tc_opendir = create_test_suite();
  suite_add_tcase(s, tc_opendir);
  return s;
}*/


int main (int argc, char** argv)
{
	fprintf(stderr, " tests : %s ", getenv("LD_LIBRARY_PATH"));
	TestSuite *global = create_test_suite();
	add_suite(global, verbose_suite());
	add_suite(global, catalog_suite());
	add_suite(global, voms_Suite());
	add_suite(global, srm_Suite());
	//add_suite(global, no_glib_suite());
	add_suite(global, lfc_suite());
	add_suite(global, mds_suite());
	//add_suite(global, filedesc_suite());
    if (argc > 1) {
        return run_single_test(global, argv[1], create_text_reporter());
    }
	run_test_suite(global, create_text_reporter());
	return 0;
}

