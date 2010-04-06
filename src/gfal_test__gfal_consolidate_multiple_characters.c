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

char * gfal_test__gfal_consolidate_multiple_characters()
{
    GFAL_TEST_EQUAL(0, (int) gfal_consolidate_multiple_characters(NULL, 'x', 0));
    GFAL_TEST_EQUAL(0, strcmp("abcd", gfal_consolidate_multiple_characters("abcd", 'a', 0)));
    GFAL_TEST_EQUAL(0, strcmp("abcd", gfal_consolidate_multiple_characters("abcd", 'x', 0)));
    GFAL_TEST_EQUAL(0, strcmp("abcd", gfal_consolidate_multiple_characters("abcd", '0', 0)));
    GFAL_TEST_EQUAL(0, strcmp("", gfal_consolidate_multiple_characters("", '0', 0)));
    GFAL_TEST_EQUAL(0, strcmp("", gfal_consolidate_multiple_characters("", 'a', 0)));
    GFAL_TEST_EQUAL(0, strcmp("a", gfal_consolidate_multiple_characters("a", 'a', 0)));
    GFAL_TEST_EQUAL(0, strcmp("a", gfal_consolidate_multiple_characters("aa", 'a', 0)));
    GFAL_TEST_EQUAL(0, strcmp("a", gfal_consolidate_multiple_characters("aaa", 'a', 0)));
    GFAL_TEST_EQUAL(0, strcmp("aaa", gfal_consolidate_multiple_characters("aaa", 'x', 0)));
    GFAL_TEST_EQUAL(0, strcmp("xa", gfal_consolidate_multiple_characters("xaaa", 'a', 0)));
    GFAL_TEST_EQUAL(0, strcmp("xay", gfal_consolidate_multiple_characters("xaaay", 'a', 0)));
    GFAL_TEST_EQUAL(0, strcmp("ay", gfal_consolidate_multiple_characters("aaay", 'a', 0)));
    GFAL_TEST_EQUAL(0, strcmp("aaaxay", gfal_consolidate_multiple_characters("aaaxaaay", 'a', 3)));
    GFAL_TEST_EQUAL(0, strcmp("aaxay", gfal_consolidate_multiple_characters("aaaxaaay", 'a', 1)));
    GFAL_TEST_EQUAL(0, strcmp("aaaxaaay", gfal_consolidate_multiple_characters("aaaxaaay", 'a', 100)));
    return NULL;
}
