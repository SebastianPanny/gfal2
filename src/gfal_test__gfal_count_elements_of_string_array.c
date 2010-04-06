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
#include "gfal_utils.h"
#include "gfal_testsuite.h"

char * gfal_test__gfal_count_elements_of_string_array()
{
    char * res = NULL;
    char * fixture_1[] = {"1", "2", NULL};
    char * fixture_2[] = {NULL};
    GFAL_TEST_EQUAL(2, gfal_count_elements_of_string_array(fixture_1));
    GFAL_TEST_EQUAL(0, gfal_count_elements_of_string_array(fixture_2));
    return NULL;
}
