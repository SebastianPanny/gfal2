/*
 * Copyright (C) 2004 by CERN
 * All rights reserved
 */


#ifndef _TEST_HELPERS_H
#define _TEST_HELPERS_H

int helper_make_guid(char *);
int helper_make_test_root();

int helper_make_lfn(char *, const char*);
int helper_remove_lfn(const char*);
int helper_remove_surl(const char*);

#endif
