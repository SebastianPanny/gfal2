/*
 * Copyright (C) 2003 by CERN
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "gfal_api.h"
#include "gfal_testsuite.h"

/* TEST_DIRECTORY environmant variable must be set: this is a SURL, pointing to a test 
 * directory to be created. In GFAL regression tests, 
 * execute_test.sh calling this Python script sets it. */

int main(int argc, char **argv)
{
    char *dir_to_create = NULL;
    struct stat statbuf;
    int res = -1;

    dir_to_create = getenv("TEST_DIRECTORY");
  printf("%s\n",  dir_to_create);

    GFAL_TEST_ASSERT(dir_to_create != NULL);
    /* Remove the directory (if it was there), and check its non-existence */
    gfal_rmdir(dir_to_create);
    errno = 0;
    res = gfal_stat(dir_to_create, &statbuf);    
    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(ENOENT, errno);
    /* Try to create the directory, and check its existance */
    res = gfal_mkdir(dir_to_create, 0755);
    GFAL_TEST_EQUAL(0, res);
    GFAL_TEST_EQUAL(0, errno);
    res = gfal_stat(dir_to_create, &statbuf); 
    GFAL_TEST_EQUAL(0, res);
    GFAL_TEST_EQUAL(0, errno);
    /* Clean up */
    //gfal_rmdir(dir_to_create);
	
    return 0;
}
