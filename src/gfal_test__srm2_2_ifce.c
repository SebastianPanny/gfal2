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
 *
 * Inspired by MinUnit -- a minimal unit testing framework for C
 * (http://www.jera.com/techinfo/jtns/jtn002.html)
 */

#include "gfal_testsuite.h"
#include "srm2_2_ifce_impl.h"

static void test_srmv2_normalize_surl_()
{
    char* surl_1 = "srm://server:port/";
    char* surl_2 = "srm://server:port/dir1";
    char* surl_3 = "srm://server:port/dir1/";
    char* surl_4 = "srm://server:port/dir1//";
    char* surl_5 = "srm://server:port/dir1/dir2/";
    char* surl_6 = "srm://server:port/dir1/dir2";
    char* surl_7 = "srm://server:port/dir1//dir2/";
    char* surl_8 = "srm://server:port/dir1//dir2//";

    GFAL_TEST_EQUAL_STRING(surl_1, srmv2_normalize_surl_(surl_1));   
    GFAL_TEST_EQUAL_STRING(surl_3, srmv2_normalize_surl_(surl_2));   
    GFAL_TEST_EQUAL_STRING(surl_3, srmv2_normalize_surl_(surl_3));   
    GFAL_TEST_EQUAL_STRING(surl_3, srmv2_normalize_surl_(surl_4));   
    GFAL_TEST_EQUAL_STRING(surl_5, srmv2_normalize_surl_(surl_5));   
    GFAL_TEST_EQUAL_STRING(surl_5, srmv2_normalize_surl_(surl_6));   
    GFAL_TEST_EQUAL_STRING(surl_5, srmv2_normalize_surl_(surl_7));   
    GFAL_TEST_EQUAL_STRING(surl_5, srmv2_normalize_surl_(surl_8));   
}

char* gfal_test__srm2_2_ifce()
{
    test_srmv2_normalize_surl_();

    return 0;
}

