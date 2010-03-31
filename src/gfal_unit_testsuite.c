/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (C) 2009 by CERN
 * All rights reserved
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
char * gfal_test__protocol_list_handling();
char * gfal_test__gfal_count_elements_of_string_array();
char * gfal_test__gfal_parseturl();
char * gfal_test__gfal_consolidate_multiple_characters();
char * gfal_test__gfal_add_strings();
char * gfal_test__srm2_2_ifce();
char * gfal_test__gfal_strip_string();

/*Register the test functions here */
static char * gfal_all_tests()
{
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
