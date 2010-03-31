/*
 * Copyright (C) 2003 by CERN
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
