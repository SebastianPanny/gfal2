/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (C) 2009 by CERN
 * All rights reserved
 *
 * Inspired by MinUnit -- a minimal unit testing framework for C
 * (http://www.jera.com/techinfo/jtns/jtn002.html)
 */
#ifndef SRM2_2_IFCE_IMPL_H
#define SRM2_2_IFCE_IMPL_H

/**
 * Normalize a SURL: remove multiple slashes (exept for "srm://"), and add a single slash 
 * to the end of the SURL. For example, SURL
 *
 * srm://server:port/dir1//dir2
 *
 * will be
 *
 * srm://server:port/dir1/dir2/
 *
 * @param surl The SURL to be normalized
 *
 * @return The dyamically allocated normalized SURL.
 */
char* srmv2_normalize_surl_(const char* surl);

#endif /* #define SRM2_2_IFCE_IMPL_H */

