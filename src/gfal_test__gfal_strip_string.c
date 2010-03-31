
/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (C) 2009 by CERN
 * All rights reserved
 */
#include <string.h>
#include "gfal_utils.h"
#include "gfal_testsuite.h"

char * gfal_test__gfal_strip_string()
{
    const char chr = '/';
    const char* s1 = "";
    const char* s2 = "str";
    const char* s3 = "/";
    const char* s4 = "s1/s2";
    const char* s5 = "/s1/s2";
    const char* s6 = "/s1";
    const char* s7 = "/s1/";
    const char* s8 = "s1/s2/";
    const char* s9 = "s1/s2//";
    
    GFAL_TEST_EQUAL_STRING("", gfal_strip_string(s1, chr));
    GFAL_TEST_EQUAL_STRING("str", gfal_strip_string(s2, chr));
    GFAL_TEST_EQUAL_STRING("/", gfal_strip_string(s3, chr));
    GFAL_TEST_EQUAL_STRING("s1/", gfal_strip_string(s4, chr));
    GFAL_TEST_EQUAL_STRING("/s1/", gfal_strip_string(s5, chr));
    GFAL_TEST_EQUAL_STRING("/", gfal_strip_string(s6, chr));
    GFAL_TEST_EQUAL_STRING("/s1/", gfal_strip_string(s7, chr));
    GFAL_TEST_EQUAL_STRING("s1/s2/", gfal_strip_string(s8, chr));
    GFAL_TEST_EQUAL_STRING("s1/s2//", gfal_strip_string(s9, chr));

    return NULL;
}
