
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
