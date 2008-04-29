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
# org.glite.data.gfal src/Makefile.am
#
# Authors: Paolo Tedesco <paolo.tedesco@cern.ch>
# Version info: $Id: Makefile.am,v 1.2 2008/04/29 14:12:56 ptedesco Exp $
# Release: $Name:  $

SOAPCPP2=$(GSOAP_LOCATION)/bin/soapcpp2
SOAPCPP2FLAGS=-c 
#	-c = generate c code

#WSDL2H=$(GSOAP_LOCATION)/bin/wsdl2h
WSDL2H=$(GSOAP_WSDL2H_LOCATION)/bin/wsdl2h
if USE_GSOAP_2_7_10
WSDL2HFLAGS=-c -e -y -I$(GSOAP_LOCATION)/include -z
else
WSDL2HFLAGS=-c -e -y
endif
#	-c      generate C source code
#	-e      don't qualify enum names
#	-y      generate typedef synonyms for structs and enums

SWIG=$(SWIG_BIN_PATH)/swig

#-------------------------------------------------------------------------------
# WDSLs, nsmap files
#-------------------------------------------------------------------------------

# edg-local-replica-catalog.wsdl
#-------------------------------------------------------------------------------
EDG_LOCAL_REPLICA_CATALOG_SOAP_FILES = lrcH.h lrcStub.h urcC.c lrcClient.c edg_local_replica_catalogSoapBinding.nsmap

$(EDG_LOCAL_REPLICA_CATALOG_SOAP_FILES): edg-local-replica-catalog.h
	$(SOAPCPP2) $(SOAPCPP2FLAGS) -p lrc edg-local-replica-catalog.h

edg-local-replica-catalog.h: edg-local-replica-catalog.wsdl
	$(WSDL2H) $(WSDL2HFLAGS) -w -t $(top_srcdir)/src/typemap-lrc.dat $< -o $@
	
# edg-replica-metadata-catalog.wsdl
#-------------------------------------------------------------------------------
EDG_REPLICA_METADATA_CATALOG_SOAP_FILES = rmcH.h rmcStub.h rmcC.c rmcClient.c edg_replica_metadata_catalogSoapBinding.nsmap

$(EDG_REPLICA_METADATA_CATALOG_SOAP_FILES): edg-replica-metadata-catalog.h
	$(SOAPCPP2) $(SOAPCPP2FLAGS) -p rmc edg-replica-metadata-catalog.h

edg-replica-metadata-catalog.h: edg-replica-metadata-catalog.wsdl
	$(WSDL2H) $(WSDL2HFLAGS) -w -t $(top_srcdir)/src/typemap-rmc.dat $< -o $@

# srm.v1.1.wsdl
#-------------------------------------------------------------------------------
SRM_V1_1_SOAP_FILES = srmH.h srmStub.h srmC.c srmClient.c ISRM.nsmap

$(SRM_V1_1_SOAP_FILES): srm.v1.1.h
	$(SOAPCPP2) $(SOAPCPP2FLAGS) -p srm srm.v1.1.h

srm.v1.1.h: srm.v1.1.wsdl
	$(WSDL2H) $(WSDL2HFLAGS) -w -t $(top_srcdir)/src/typemap-srmv1.dat $< -o $@

# srm.v2.2.wsdl
#-------------------------------------------------------------------------------
SRM_V2_2_SOAP_FILES = srmv2H.h srmv2Stub.h srmv2C.c srmv2Client.c srmSoapBinding.nsmap

$(SRM_V2_2_SOAP_FILES): srm.v2.2.h
	$(SOAPCPP2) $(SOAPCPP2FLAGS) -p srmv2 srm.v2.2.h

srm.v2.2.h: srm.v2.2.wsdl
	$(WSDL2H) $(WSDL2HFLAGS) -t $(top_srcdir)/src/typemap-srmv2.dat $< -o $@

# fixed nsmap files
#-------------------------------------------------------------------------------
FIXED_NSMAP_FILES = \
	edg_local_replica_catalogSoapBinding+.nsmap \
	edg_replica_metadata_catalogSoapBinding+.nsmap \
	srmSoapBinding+.nsmap

edg_local_replica_catalogSoapBinding+.nsmap: edg_local_replica_catalogSoapBinding.nsmap
	sed 's/struct Namespace namespaces/struct Namespace namespaces_lrc/' $? > $@

edg_replica_metadata_catalogSoapBinding+.nsmap: edg_replica_metadata_catalogSoapBinding.nsmap
	sed 's/struct Namespace namespaces/struct Namespace namespaces_rmc/' $? > $@

srmSoapBinding+.nsmap: srmSoapBinding.nsmap
	sed 's/struct Namespace namespaces/struct Namespace namespaces_srmv2/' $? > $@

#-------------------------------------------------------------------------------
# _PROGRAMS, _LIBRARIES, BUILT_SOURCES...
#-------------------------------------------------------------------------------

# distributed header files
dist_include_HEADERS = \
	gfal_api.h \
	gfal_constants.h

# executables
bin_PROGRAMS = \
	gfal_version \
	gfal_testchmod \
	gfal_testget \
	gfal_teststat \
	gfal_testcreatdir \
	gfal_testread \
	gfal_testunlink \
	gfal_testdir \
	gfal_testrw

# libraries
lib_LIBRARIES = \
	libgfal.a

# shared libraries
lib_LTLIBRARIES = \
	libgfal.la \
	libgfal_pthr.la 

# generated source files (to build before other programs and libraries)
BUILT_SOURCES = \
	$(EDG_LOCAL_REPLICA_CATALOG_SOAP_FILES) \
	$(EDG_REPLICA_METADATA_CATALOG_SOAP_FILES) \
	$(SRM_V1_1_SOAP_FILES) \
	$(SRM_V2_2_SOAP_FILES) \
	$(FIXED_NSMAP_FILES) \
	gfal_wrap.c \
	gfalthr_wrap.c

# python
pkgpython_PYTHON = gfal.py gfalthr.py
pkgpyexec_LTLIBRARIES = _gfal.la _gfalthr.la

_gfal_la_SOURCES = gfal.i lcg-typemaps.python.i gfal_wrap.c
_gfal_la_LDFLAGS = -module
_gfal_la_CFLAGS = $(PYTHON_CFLAGS)

gfal_wrap.c: gfal.i lcg-typemaps.python.i
	$(SWIG) -python -o $@ $<

_gfalthr_la_SOURCES = gfalthr.i lcg-typemaps.python.i gfalthr_wrap.c
_gfalthr_la_LDFLAGS = -module
_gfalthr_la_CFLAGS = $(PYTHON_CFLAGS)

gfalthr_wrap.c: gfalthr.i lcg-typemaps.python.i
	$(SWIG) -python -o $@ $<


GFAL_SOURCE_FILES = \
	checkprotolib.c \
	gfal.c \
	lrc_ifce.c \
	mds_ifce.c \
	rmc_ifce.c \
	sfn_ifce.c \
	srm2_2_ifce.c \
	srm_ifce.c \
	lfc_ifce.c \
	gridftp_ifce.c \
	srmClient.c \
	srmC.c

lrc_ifce.o: lrc_ifce.c lrcH.h lrcStub.h lrcC.c lrcClient.c edg_local_replica_catalogSoapBinding+.nsmap gfal_api.h
lrc_ifce_o_CFLAGS = -DWITH_NOGLOBAL
rmc_ifce.o: rmc_ifce.c rmcH.h rmcStub.h rmcC.c rmcClient.c edg_replica_metadata_catalogSoapBinding+.nsmap gfal_api.h
rmc_ifce_o_CFLAGS = -DWITH_NOGLOBAL
srm2_2_ifce.o: srm2_2_ifce.c srmv2H.h srmv2Stub.h srmSoapBinding+.nsmap gfal_api.h
srm2_2_ifce_o_CFLAGS = -DWITH_NOGLOBAL
srm_ifce.o: srm_ifce.c srmH.h srmStub.h ISRM.nsmap gfal_api.h

GFAL_FLAGS = \
	-I$(LFC_LOCATION)/include/lfc \
	-DGFAL_ENABLE_RFIO -I/usr/include/shift -I/usr/local/include/shift -I$(DPM_LOCATION)/include/dpm \
	-DGFAL_ENABLE_DCAP -I$(DCAP_LOCATION)/include \
	-DGFAL_SECURE \
	-fPIC -D_LARGEFILE64_SOURCE \
	-DVERSION=\"$(VERSION)\"

# libgfal.a
#-------------------------------------------------------------------------------
libgfal_a_SOURCES = $(GFAL_SOURCE_FILES)

libgfal_a_CFLAGS = \
	$(GLOBUS_NOTHR_CFLAGS) \
	$(GLITE_CFLAGS) \
	$(GFAL_FLAGS) \
	$(GSOAP_CFLAGS)

# libgfal.la
#-------------------------------------------------------------------------------
libgfal_la_SOURCES = $(GFAL_SOURCE_FILES)

libgfal_la_LIBADD = \
	-luuid -lc -lldap -ldl \
	$(GSOAP_LIBS) \
	$(CGSI_GSOAP_LIBS) \
	$(GLOBUS_GSS_NOTHR_LIBS) \
	$(GLOBUS_FTP_CLIENT_NOTHR_LIBS) \
	$(VOMS_NOTHR_LIBS)

libgfal_la_CFLAGS = \
	$(GLOBUS_NOTHR_CFLAGS) \
	$(GLITE_CFLAGS) \
	$(GFAL_FLAGS) \
	$(GSOAP_CFLAGS) \
	$(VOMS_CFLAGS)

# libgfal_pthr.la
#-------------------------------------------------------------------------------
libgfal_pthr_la_SOURCES = $(GFAL_SOURCE_FILES)

libgfal_pthr_la_LIBADD = \
	-luuid -lc -lldap -ldl \
	$(GSOAP_LIBS) \
	$(CGSI_GSOAP_LIBS) \
	$(GLOBUS_GSS_THR_LIBS) \
	$(GLOBUS_FTP_CLIENT_THR_LIBS)

libgfal_pthr_la_LDFLAGS = \
	$(GFAL_FLAGS) 

libgfal_pthr_la_CFLAGS = \
	$(GLOBUS_THR_CFLAGS) \
	$(GLITE_CFLAGS) \
	$(GFAL_FLAGS) \
	$(GSOAP_CFLAGS)

# gfal_version
#-------------------------------------------------------------------------------
gfal_version_SOURCES = \
	gfal_version.c

gfal_version_LDADD = -luuid -lc -lldap -ldl libgfal.la $(VOMS_NOTHR_LIBS) $(CGSI_GSOAP_LIBS) $(GSOAP_LIBS)

gfal_version_CFLAGS = $(VOMS_CFLAGS)

# test programs
#-------------------------------------------------------------------------------
gfal_testchmod_SOURCES = gfal_testchmod.c
gfal_testchmod_LDADD = -luuid -lc -lldap -ldl libgfal.la $(VOMS_NOTHR_LIBS) $(CGSI_GSOAP_LIBS) $(GSOAP_LIBS)
gfal_testchmod_CFLAGS = $(VOMS_CFLAGS)

gfal_testget_SOURCES = gfal_testget.c
gfal_testget_LDADD = -luuid -lc -lldap -ldl libgfal.la $(VOMS_NOTHR_LIBS) $(CGSI_GSOAP_LIBS) $(GSOAP_LIBS)
gfal_testget_CFLAGS = $(VOMS_CFLAGS)

gfal_teststat_SOURCES = gfal_teststat.c
gfal_teststat_LDADD = -luuid -lc -lldap -ldl libgfal.la $(VOMS_NOTHR_LIBS) $(CGSI_GSOAP_LIBS) $(GSOAP_LIBS)
gfal_teststat_CFLAGS = $(VOMS_CFLAGS)

gfal_testcreatdir_SOURCES = gfal_testcreatdir.c
gfal_testcreatdir_LDADD = -luuid -lc -lldap -ldl libgfal.la $(VOMS_NOTHR_LIBS) $(CGSI_GSOAP_LIBS) $(GSOAP_LIBS)
gfal_testcreatdir_CFLAGS = $(VOMS_CFLAGS)

gfal_testread_SOURCES = gfal_testread.c
gfal_testread_LDADD = -luuid -lc -lldap -ldl libgfal.la $(VOMS_NOTHR_LIBS) $(CGSI_GSOAP_LIBS) $(GSOAP_LIBS)
gfal_testread_CFLAGS = $(VOMS_CFLAGS)

gfal_testunlink_SOURCES = gfal_testunlink.c
gfal_testunlink_LDADD = -luuid -lc -lldap -ldl libgfal.la $(VOMS_NOTHR_LIBS) $(CGSI_GSOAP_LIBS) $(GSOAP_LIBS)
gfal_testunlink_CFLAGS = $(VOMS_CFLAGS)

gfal_testdir_SOURCES = gfal_testdir.c
gfal_testdir_LDADD = -luuid -lc -lldap -ldl libgfal.la $(VOMS_NOTHR_LIBS) $(CGSI_GSOAP_LIBS) $(GSOAP_LIBS)
gfal_testdir_CFLAGS = $(VOMS_CFLAGS)

gfal_testrw_SOURCES = gfal_testrw.c
gfal_testrw_LDADD = -luuid -lc -lldap -ldl libgfal.la $(VOMS_NOTHR_LIBS) $(CGSI_GSOAP_LIBS) $(GSOAP_LIBS)
gfal_testrw_CFLAGS = $(VOMS_CFLAGS)


#-------------------------------------------------------------------------------
# DOCUMENTATION
#-------------------------------------------------------------------------------
man3_MANS = \
	gfal_access.man \
	gfal_chmod.man \
	gfal_closedir.man \
	gfal_close.man \
	gfal_creat.man \
	gfal_deletesurls.man \
	gfal_deletesurls_python.man \
	gfal_get_ids.man \
	gfal_get_ids_python.man \
	gfal_get.man \
	gfal_get_python.man \
	gfal_get_results.man \
	gfal_get_results_python.man \
	gfal_getstatus.man \
	gfal_getstatus_python.man \
	gfal_init.man \
	gfal_init_python.man \
	gfal_internal_free.man \
	gfal_internal_free_python.man \
	gfal_lseek.man \
	gfal_ls.man \
	gfal_ls_python.man \
	gfal.man \
	gfal_mkdir.man \
	gfal_opendir.man \
	gfal_open.man \
	gfal_pin.man \
	gfal_pin_python.man \
	gfal_prestage.man \
	gfal_prestage_python.man \
	gfal_prestagestatus.man \
	gfal_prestagestatus_python.man \
	gfal_python.man \
	gfal_readdir.man \
	gfal_read.man \
	gfal_release.man \
	gfal_release_python.man \
	gfal_rename.man \
	gfal_request_new.man \
	gfal_rmdir.man \
	gfal_set_xfer_done.man \
	gfal_set_xfer_done_python.man \
	gfal_set_xfer_running.man \
	gfal_set_xfer_running_python.man \
	gfal_stat.man \
	gfal_turlsfromsurls.man \
	gfal_turlsfromsurls_python.man \
	gfal_unlink.man \
	gfal_write.man

