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
