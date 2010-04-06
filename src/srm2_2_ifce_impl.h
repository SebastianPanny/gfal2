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

