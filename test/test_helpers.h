/*
 * Copyright (C) 2004 by CERN
 * All rights reserved
 */


#ifndef _TEST_HELPERS_H
#define _TEST_HELPERS_H


#define ERRBUFSZ 1024

void helper_make_guid(char *);
void helper_make_test_root();

void helper_make_surl(char *, const char *);
void helper_make_lfn(char *, const char*);
int helper_remove_lfn(const char*);
int helper_remove_surl(const char*);

#endif
