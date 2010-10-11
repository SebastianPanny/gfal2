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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include "gfal_api.h"
#include "gfal_testsuite.h"

/* TEST_DIRECTORY environment variable must be set: this is a SURL, pointing to a test 
 * directory to be created. In GFAL regression tests, execute_test.sh sets it. */

/* Test strategy: create directories with different slash configurations (more slashes 
 * between direcotry elements, more trailing slashes, etc.), and check if directory was 
 * really created. This check is based on "normalized" directory names (no trailing 
 * slashes, * etc.). */

static struct {
    char* dir_to_create;
    char* dir_one_more_level;
} self = {NULL, NULL};

static void delete_dirs_()
{
    int res = -2;
    struct stat statbuf;
    gfal_rmdir(self.dir_one_more_level);
    res = gfal_stat(self.dir_one_more_level, &statbuf);
    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(ENOENT, errno);
    gfal_rmdir(self.dir_to_create);
    res = gfal_stat(self.dir_to_create, &statbuf);
    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(ENOENT, errno);
}

static char* string_add_(const char* s1, const char* s2)
{
    char *res = (char*) malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(res, s1);
    strcat(res, s2);
    return res;
}

static void set_up()
{
    struct stat statbuf;
    int res = -1;
    self.dir_to_create = getenv("TEST_DIRECTORY");
    GFAL_TEST_ASSERT(self.dir_to_create != NULL);
    /*  Check the condition of "last caracter cannot be a '/'" */
    GFAL_TEST_ASSERT(self.dir_to_create[strlen(self.dir_to_create) - 1] != '/');
    /* allocate enough memory for <dir_to_create>/oneMoreLevel + slashes... */
    self.dir_one_more_level = string_add_(self.dir_to_create, "/oneMoreLevel");
    delete_dirs_();
    res = gfal_stat(self.dir_to_create, &statbuf);
    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(ENOENT, errno);
}

static void create_and_test_(const int line, const char* realdir, const char* testdir)
{
    int res = -1;
    struct stat statbuf;
   
    printf("Called from line %d\n", line);

    if (testdir == NULL)
        testdir = realdir;

    res = gfal_mkdir(testdir, 0755);
    GFAL_TEST_EQUAL(0, errno);
    res = gfal_stat(realdir, &statbuf);
    GFAL_TEST_EQUAL(0, res);
    GFAL_TEST_EQUAL(0, errno);
    delete_dirs_();
}

static void test_two_directory_levels_with_slashes()
{
    set_up();
    create_and_test_(__LINE__, self.dir_one_more_level, 
                     string_add_(self.dir_to_create, "/oneMoreLevel/"));
    create_and_test_(__LINE__, self.dir_one_more_level, 
                     string_add_(self.dir_to_create, "/oneMoreLevel"));
}

int main(int argc, char **argv)
{
  	test_two_directory_levels_with_slashes();
    return 0;
}

