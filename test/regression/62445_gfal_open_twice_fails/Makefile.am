# Copyright (c) Members of the EGEE Collaboration. 2004-2008.
# See http://www.eu-egee.org/partners/ for details on the copyright
# holders.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
# Version info: $Id: Makefile.am,v 1.3 2009/12/09 13:18:13 molnarzs Exp $
# Release: $Name:  $

AM_CFLAGS = -g -fPIC -D_LARGEFILE64_SOURCE \
    -DVERSION=\"$(VERSION)-$(RELEASE)\" $(GSOAP_CFLAGS) \
    -I$(top_srcdir)/src -I$(top_srcdir)/test -I../../src \
    -I$(SRM_IFCE_LOCATION)/include

bin_PROGRAMS = \
	62445_gfal_open_twice_fails

62445_gfal_open_twice_fails_SOURCES= \
	62445_gfal_open_twice_fails.c

62445_gfal_open_twice_fails_LDADD = $(top_builddir)/src/libgfal.la
