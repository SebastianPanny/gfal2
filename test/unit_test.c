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
 
 
// constant to define for succefull unit test



/**
 * Unit tests for gfal
 * @author : Devresse Adrien
 * @version : 0.0.1
 */

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "common/gfal_common.h"
#include "common/gfal__test_verbose.c"
#include "common/gfal__test_catalog.c"
#include "common/voms/gfal__test_voms.c"
#include "common/gfal__test_common_srm.c"
#include "common/mds/gfal__test_common_mds.c"
#include "common/lfc/gfal__test_common_lfc.c"
#include "common/gfal__test_common_srm_no_glib.c"

Suite* common_suite (void)
{
  Suite *s = suite_create ("Common");

  /* verbose test case */
  TCase *tc_core = tcase_create ("Verbose");
  tcase_add_test (tc_core, test_verbose_set_get);
  suite_add_tcase (s, tc_core);
  /* catalog */
  TCase *tc_cata = tcase_create ("Catalog");
  tcase_add_test (tc_cata, test_get_cat_type);
  suite_add_tcase (s, tc_cata);
  /* voms */
  TCase *tc_voms = tcase_create ("Voms");
  tcase_add_test (tc_voms, test_voms_parse_args);
  tcase_add_test (tc_voms, test_voms_info_is_null);
  tcase_add_test (tc_voms,test_voms_info_test_vo);
  tcase_add_test (tc_voms,  test_voms_get_userdnG);
  tcase_add_test (tc_voms, test_voms_get_fqan);
  suite_add_tcase (s, tc_voms);
  TCase *tc_srm = tcase_create("SRM");
  tcase_add_test(tc_srm, test_create_srm_handle);
  tcase_add_test(tc_srm, test_glist_to_surls);
  tcase_add_test(tc_srm, test_gfal_get_async_1);
  tcase_add_test(tc_srm, test_gfal_check_surl);
  tcase_add_test(tc_srm, test_gfal_full_endpoint_check);
  tcase_add_test(tc_srm, test_gfal_auto_get_srm_endpoint_full_endpoint);
  tcase_add_test(tc_srm, test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdii);
  tcase_add_test(tc_srm, test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdii_negative);
  tcase_add_test(tc_srm, test_gfal_auto_get_srm_endpoint_no_full_with_bdii);
  tcase_add_test(tc_srm, test_gfal_get_fullendpoint);
  tcase_add_test(tc_srm, test_gfal_get_hostname_from_surl);
  tcase_add_test(tc_srm, test_gfal_get_endpoint_and_setype_from_bdii);
  tcase_add_test(tc_srm, test_gfal_select_best_protocol_and_endpoint);
  tcase_add_test(tc_srm, gfal_get_asyncG_empty_req);
  tcase_add_test(tc_srm, gfal_get_asyncG_empty_old_nonexist_surl);
  tcase_add_test(tc_srm, test_gfal_is_finished);
  tcase_add_test(tc_srm, test_gfal_waiting_async);
  tcase_add_test(tc_srm, test_gfal_get_async_resultsG);
  tcase_add_test(tc_srm, test_gfal_get_async_resultsG_empty);
  tcase_add_test(tc_srm, test_full_gfal_get_request);
  tcase_add_test(tc_srm, test_gfal_async_results_errcode);
  tcase_add_test(tc_srm, test_full_gfal_get_request_multi);  
  suite_add_tcase (s, tc_srm);
  TCase* tc_srm_no_glib = tcase_create("SRM_NO_GLIB");
  tcase_add_test(tc_srm_no_glib, test_srm_api_no_glib_full);
  suite_add_tcase(s, tc_srm_no_glib);
  TCase *tc_mds= tcase_create("MDS");
  tcase_add_test(tc_mds, test_check_bdii_endpoints_srm);
  tcase_add_test(tc_mds, gfal__test_get_lfchost_bdii);
  tcase_add_test(tc_mds, gfal__test_get_lfchost_bdii_with_nobdii);
  suite_add_tcase(s, tc_mds);
  TCase *tc_lfc= tcase_create("LFC");
  tcase_add_test(tc_lfc, test_gfal_common_lfc_define_env);
  tcase_add_test(tc_lfc, test_gfal_common_lfc_init);
  tcase_add_test(tc_lfc, test_gfal_common_lfc_resolve_sym);
  suite_add_tcase(s, tc_lfc);

  return s;
}




int main (int argc, char** argv)
{
  fprintf(stderr, " tests : %s ", getenv("LD_LIBRARY_PATH"));
  int number_failed;
  Suite *s = common_suite ();
  SRunner *sr = srunner_create (s);
  srunner_set_fork_status(sr, CK_NOFORK); // no fork mode for gdb
  srunner_run_all (sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed (sr);
  if( number_failed > 0){
	  fprintf(stderr, "Error occured while unit tests");
  }
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

}

