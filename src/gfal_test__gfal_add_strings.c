/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (C) 2009 by CERN
 * All rights reserved
 */
#include <string.h>
#include "gfal_utils.h"
#include "gfal_testsuite.h"

char * gfal_test__gfal_add_strings()
{
    const char* s1 = "s1";
    const char* s2 = "s2";

    GFAL_TEST_EQUAL_STRING("s1s2", gfal_add_strings(s1, s2));
    GFAL_TEST_EQUAL_STRING(s2, gfal_add_strings("", s2));
    GFAL_TEST_EQUAL_STRING(s1, gfal_add_strings(s1, ""));
    GFAL_TEST_EQUAL_STRING("", gfal_add_strings("", ""));

    return NULL;
}
