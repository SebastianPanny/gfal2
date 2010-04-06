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
 */

#include <stdio.h>
#include <fcntl.h>
#include "gfal_api.h"
#include "gfal_testsuite.h"

/* GFAL_OPEN_TWICE_FAILS_TEST_FILE"TEST_DIRECTORY environmant variable must be set: 
   this is SURL, pointing to a test file already created. */
int main(int argc, char **argv)
{
    int fd = -2;
    char* test_file = getenv("GFAL_OPEN_TWICE_FAILS_TEST_FILE");

    GFAL_TEST_ASSERT(test_file != NULL);
    fd = gfal_open(test_file, O_RDONLY, 0644);
    GFAL_TEST_ASSERT(fd > 0);
    gfal_close(fd);
    fd = -2;
    fd = gfal_open(test_file, O_RDONLY, 0644);
    GFAL_TEST_ASSERT(fd > 0);
    
    return 0;
}
