/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (C) 2009 by CERN
 * All rights reserved
 *
 * Inspired by MinUnit -- a minimal unit testing framework for C
 * (http://www.jera.com/techinfo/jtns/jtn002.html)
 */
#ifndef GFAL_UNIT_TESTSUITE_H
#define GFAL_UNIT_TESTSUITE_H

#include <stdio.h>

#define GFAL_TEST_ASSERT(test) \
    do { \
        printf("\t Test at %s:%d... ", __FILE__, __LINE__); \
        if (!(test)) { \
            printf("FAILED\n"); \
            exit(-1); \
        } \
        printf("OK\n"); \
    } while (0)

#define GFAL_TEST_EQUAL(expected, value) \
    do { \
        printf("\t Test at %s:%d... ", __FILE__, __LINE__); \
        if ((expected) != (value)) { \
            printf("FAILED. Expected: %d, Got: %d\n", (expected), (value)); \
            exit(-1); \
        } \
        printf("OK\n"); \
    } while (0)

#define GFAL_TEST_RUN(test) \
    do { \
        printf("\nRunning: %s...\n", #test); \
        test(); \
        __gfal_tests_run++; \
       } while (0)

extern int __gfal_tests_run;

#endif /* #ifndef GFAL_UNIT_TESTSUITE_H */
