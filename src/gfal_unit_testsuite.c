/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
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
#include <stdio.h>
#include "gfal_testsuite.h"

int __gfal_tests_run = 0;

/* Example test suite
 *
 * 1. Create a test function. Signature should follow the example.
 *
 * char * test_foo()
 * {
 *     GFAL_TEST_ASSERT(1);
 *     return 0;
 * }
 *
 * Use the GFAL_TEST_ASSERT macro. Its parameter must be a boolean value. If
 * it is false, the test suite stops immediately, with an error message.
 *
 * List all the test functions here, in this file.
 *
 * 2. Add the test function to the lcg_all_tests, in the following way:
 *
 * static char * gfal_all_tests()
 * {
 *     GFAL_TEST_RUN(test_foo);
 *     return NULL;
 * }
 *
 * 3. Do not forget updating the Makefile.am, if you add new sources...
 *
 * That is it.
 */

/*List of test functions */
char * gfal_test__srmv2_check_srm_root();
char * gfal_test__protocol_list_handling();
char * gfal_test__gfal_count_elements_of_string_array();
char * gfal_test__gfal_parseturl();
char * gfal_test__gfal_consolidate_multiple_characters();
char * gfal_test__gfal_add_strings();
char * gfal_test__srm2_2_ifce();
char * gfal_test__gfal_strip_string();
char * gfal_test__mds_ifce();

/*Register the test functions here */
static char * gfal_all_tests()
{
    GFAL_TEST_RUN(gfal_test__srmv2_check_srm_root);
    GFAL_TEST_RUN(gfal_test__mds_ifce);
    GFAL_TEST_RUN(gfal_test__gfal_consolidate_multiple_characters);
    GFAL_TEST_RUN(gfal_test__gfal_count_elements_of_string_array);
    GFAL_TEST_RUN(gfal_test__protocol_list_handling);
    GFAL_TEST_RUN(gfal_test__gfal_parseturl);
    GFAL_TEST_RUN(gfal_test__gfal_add_strings);
    GFAL_TEST_RUN(gfal_test__gfal_strip_string);
    GFAL_TEST_RUN(gfal_test__srm2_2_ifce);
    return NULL;
}

int main(int argc, char **argv)
{
    printf("Starting GFAL unit test suite...\n\n");
    gfal_all_tests();
    printf("\nALL TESTS PASSED.\n");
    printf("Test functions executed: %d\n", __gfal_tests_run);
    return 0;
}
