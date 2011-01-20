/*
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
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 */

/*
 * @(#)$RCSfile: gfal_utils.c,v $ $Revision: 1.1 $ $Date: 2009/10/08 15:32:39 $ CERN Remi Mollon
 */
#include "gfal_utils.h"
#include <assert.h>

int gfal_count_elements_of_string_array(char** a)
{
    int ret = -1;
    assert(a);
    for (ret = 0; a[ret] != 0; ++ret) ;
    return ret;
}

char* gfal_strip_string(const char* str, const char chr)
{
    char *res = 0;

    assert(str);

    if (str) {
        char *pos = strrchr(str, chr);
        int size = -1;

        if (pos) {
            /* +1: to include the last character as well */
            size = pos - str + 1; 
        } else {
            size = strlen(str);
        }

        res = (char*) malloc(size + 1);
        strncpy(res, str, size);
        res[size] = 0;
    }

    return res;
}

