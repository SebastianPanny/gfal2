/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (C) 2009 by CERN
 * All rights reserved
 */
#include "gfal_utils.h"
#include "gfal_unit_testsuite.h"

char * gfal_test__gfal_count_elements_of_string_array()
{
    char * res = NULL;
    char * fixture_1[] = {"1", "2", NULL};
    char * fixture_2[] = {NULL};
    GFAL_TEST_EQUAL(2, gfal_count_elements_of_string_array(fixture_1));
    GFAL_TEST_EQUAL(0, gfal_count_elements_of_string_array(fixture_2));
    return NULL;
}
