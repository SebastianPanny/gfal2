
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
#include "srm2_2_ifce.h"
#include "gfal_testsuite.h"

char * gfal_test__srmv2_check_srm_root()
{
    const char* good_url = "srm://server.cern.ch/";
    const char* bad_url_1 = "srm://server.cern.ch/file";
    const char* bad_url_2 = "fake://server.cern.ch/file";
    const char* bad_url_3 = "srm:/server.cern.ch/file";
    const char* bad_url_4 = "srm//server.cern.ch/file";
    const char* bad_url_5 = "srm://server.cern.ch";

    GFAL_TEST_EQUAL(1, srmv2_check_srm_root(good_url));
    GFAL_TEST_EQUAL(0, srmv2_check_srm_root(NULL));
    GFAL_TEST_EQUAL(0, srmv2_check_srm_root(bad_url_1));
    GFAL_TEST_EQUAL(0, srmv2_check_srm_root(bad_url_2));
    GFAL_TEST_EQUAL(0, srmv2_check_srm_root(bad_url_3));
    GFAL_TEST_EQUAL(0, srmv2_check_srm_root(bad_url_4));
    GFAL_TEST_EQUAL(0, srmv2_check_srm_root(bad_url_5));

    return NULL;
}
