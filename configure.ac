# Copyright (c) Members of the EGEE Collaboration. 2008.
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
# org.glite.data.gfal configure.ac
#
# Authors: Paolo Tedesco <paolo.tedesco@cern.ch>
# Version info: $Id: configure.ac,v 1.2 2008/04/29 14:12:56 ptedesco Exp $
# Release: $Name:  $

AC_PREREQ(2.57)
# version 0.0.0 will be replaced by AC_GLITE_VERSION
AC_INIT([GLite Data Grid File Access Library], [0.0.0])
# auxiliary build tools directory
AC_CONFIG_AUX_DIR([./project])
AC_CONFIG_SRCDIR([src/Makefile.am])
AM_INIT_AUTOMAKE

# Notices.
AC_COPYRIGHT([Copyright (c) 2008 The EU EGEE Project
See LICENSE file for details
])
AC_REVISION([$Revision: 1.2 $])

# Check for the basic compile environment.
AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_LIBTOOL

# Check for the basic compile environment.
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# some default values
if [ $(uname -m) = 'x86_64' ]; then
    GLOBUS_FLAVOUR_DEFAULT=gcc64dbg
    GLOBUS_FLAVOUR_PTHR_DEFAULT=gcc64dbgpthr
else
    GLOBUS_FLAVOUR_DEFAULT=gcc32dbg
    GLOBUS_FLAVOUR_PTHR_DEFAULT=gcc32dbgpthr
fi

################################################################################
#                               Arguments
################################################################################

# --enable arguments
#-------------------------------------------------------------------------------
AC_ARG_ENABLE(
	[debug],
	[AS_HELP_STRING([--enable-debug],[Produce binaries with debugging symbols.])],
	[CFLAGS="-g -DDEBUG -D_DEBUG $CFLAGS"])

AC_ARG_ENABLE(
	[wall],
	[AS_HELP_STRING([--enable-wall],[Compile with all warnings enabled.])],
	[CFLAGS="-Wall $CFLAGS"])

AC_ARG_ENABLE(
	[rfio-support],
	[AS_HELP_STRING([--enable-wall],[Compile with all warnings enabled.])],
	[CFLAGS="-Wall $CFLAGS"])

# Set GLite Properties
AC_GLITE
AC_GLITE_VERSION

# globus 
AC_GLOBUS

# gsoap
AC_GSOAP
AM_CONDITIONAL([USE_GSOAP_2_7_10],[test "x$GSOAP_VERSION_NUM" = "x020710"])

#cgsi-gsoap
AC_CGSI_GSOAP

#swig 
AC_SWIG

# --with arguments
#-------------------------------------------------------------------------------

# lfc-location
AC_ARG_WITH([lfc-location],
    [AS_HELP_STRING([--with-lfc-location=VAL],[LFC location])],
    [LFC_LOCATION=${withval}],
    [LFC_LOCATION="/opt/lcg"])
AC_MSG_RESULT([LFC_LOCATION is ${LFC_LOCATION}])
AC_SUBST(LFC_LOCATION)

# dpm-location
AC_ARG_WITH([dpm-location],
    [AS_HELP_STRING([--with-dpm-location=VAL],[DPM location])],
    [DPM_LOCATION=${withval}],
    [DPM_LOCATION="/opt/lcg"])
AC_MSG_RESULT([DPM_LOCATION is ${DPM_LOCATION}])
AC_SUBST(DPM_LOCATION)

# dcap-location
AC_ARG_WITH([dcap-location],
    [AS_HELP_STRING([--with-dcap-location=VAL],[DCAP location])],
    [DCAP_LOCATION=${withval}],
    [DCAP_LOCATION="/opt/lcg"])
AC_MSG_RESULT([DCAP_LOCATION is ${DCAP_LOCATION}])
AC_SUBST(DCAP_LOCATION)

# voms-location
AC_ARG_WITH([voms-location],
    [AS_HELP_STRING([--with-voms-location=VAL],[VOMS location])],
    [VOMS_LOCATION=${withval}],
    [VOMS_LOCATION="/opt/lcg"])
AC_MSG_RESULT([VOMS_LOCATION is ${VOMS_LOCATION}])
VOMS_CFLAGS="-I${VOMS_LOCATION}/include -I${GLOBUS_LOCATION}/include/${GLOBUS_FLAVOUR}"
AC_MSG_RESULT([VOMS_CFLAGS is ${VOMS_CFLAGS}])
VOMS_THR_LIBS="-L${VOMS_LOCATION}/lib -L${VOMS_LOCATION}/lib64 -lvomsc_${GLOBUS_THR_FLAVOR}"
VOMS_NOTHR_LIBS="-L${VOMS_LOCATION}/lib -L${VOMS_LOCATION}/lib64 -lvomsc_${GLOBUS_NOTHR_FLAVOR}"
AC_MSG_RESULT([VOMS_THR_LIBS is ${VOMS_THR_LIBS}])
AC_MSG_RESULT([VOMS_NOTHR_LIBS is ${VOMS_NOTHR_LIBS}])
AC_SUBST(VOMS_LOCATION)
AC_SUBST(VOMS_CFLAGS)
AC_SUBST(VOMS_THR_LIBS)
AC_SUBST(VOMS_NOTHR_LIBS)

################################################################################

# check support for python
AM_PATH_PYTHON([2.3])
AC_PYTHON([2.3])
AC_MSG_RESULT([PYTHON_CFLAGS is ${PYTHON_CFLAGS}])

# swig
AC_SWIG([1.3.0])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([memmove memset mkdir putenv rmdir strcasecmp strchr strdup strerror strncasecmp strrchr strstr strtol])

#Conditions for Gsoap on-the-fly paches
AC_MSG_NOTICE([GSOAP_VERSION is $GSOAP_VERSION])
AC_MSG_NOTICE([GSOAP_VERSION_NUM is $GSOAP_VERSION_NUM])

AC_MSG_NOTICE([checking gsoap version to apply on-the-fly patches])
AM_CONDITIONAL([USE_GSOAP_2_7_10],[test "x$GSOAP_VERSION_NUM" = "x020710"])

AC_CONFIG_HEADERS([src/autogen/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])

AC_OUTPUT

