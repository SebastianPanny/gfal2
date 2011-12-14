Name:				gfal2
Version:			2.0.0
Release:			1.18_preview
Summary:			Grid file access library 2.0
Group:				Applications/Internet
License:			ASL 2.0
URL:				https://svnweb.cern.ch/trac/lcgutil/wiki/gfal2
## source URL
# svn export http://svn.cern.ch/guest/lcgutil/gfal/branches/gfal_2_0_main gfal2
#
Source:				%{name}-%{version}.src.tar.gz
BuildRoot:			%{_tmppath}/%{name}-%{version}-%{release}

BuildRequires:		scons
BuildRequires:		glib2-devel%{?_isa}
BuildRequires:		openldap-devel%{?_isa}
BuildRequires:		libattr-devel%{?_isa}
BuildRequires:		e2fsprogs-devel%{?_isa}

%description
GFAL 2.0 offers a POSIX API for the distributed file systems.
Designed for the WLCG file system, It offers an extensible \
plugin systems able to support new protocols. 

%package core
Summary:			Core of the Grid File access Library 2.0
Group:				Applications/Internet
Requires:			openldap%{?_isa}

%description core
Core and main files of GFAL 2.0

%package devel
Summary:			Development files for GFAL 2.0
Group:				Applications/Internet
Requires: 			gfal2-core%{?_isa} >= %{version}
Requires: 			libattr-devel%{?_isa} 

%description devel
development files for GFAL 2.0

%package doc
Summary:			Documentation for GFAL 2.0 core
Group:				Applications/Internet
Requires:			gfal2-core%{?_isa} >= %{version}

%description doc
Doxygen documentation of gfal 2.0 .

%package plugin-lfc
Summary:			Provide the lfc access for gfal2.0
Group:				Applications/Internet
BuildRequires:		lfc-devel%{?_isa}
Requires: 			gfal2-core%{?_isa} >= %{version}
Requires: 			lfc-libs%{?_isa}

%description plugin-lfc
Provide the lfc access (lfn :// ) for gfal2.0, lfc plugin

%package plugin-rfio
Summary:			Provide the rfio access for gfal2.0
Group:				Applications/Internet
BuildRequires:		dpm-devel%{?_isa}
Requires:			gfal2-core%{?_isa} >= %{version}
Requires:			dpm-libs%{?_isa}

%description plugin-rfio
Provide the rfio access (rfio:// ) for gfal2.0, rfio plugin

%package plugin-dcap
Summary:			Provide the dcap access for gfal2.0
Group:				Applications/Internet
BuildRequires:		dcap-devel%{?_isa}
Requires:			gfal2-core%{?_isa} >= %{version}
Requires:			dcap-libs%{?_isa}

%description plugin-dcap
Provide the dcap access (gsidcap://, dcap:// ) for gfal2.0, dcap plugin

%package plugin-srm
Summary:			Provide the srm access for gfal2.0
Group:				Applications/Internet
BuildRequires:		srm-ifce-devel%{?_isa}
Requires:			gfal2-core%{?_isa} >= %{version}

%description plugin-srm
Provide the srm access (srm:// ) for gfal2.0, srm plugin

%package plugin-devel
Summary:			Development files for GFAL 2.0 plugin development
Group:				Applications/Internet
Requires:			gfal2-core%{?_isa} >= %{version} 

%description plugin-devel
Provide the headers files for plugin development

%package all
Summary:			Meta package for gfal 2.0 global install
Group:				Applications/Internet
Requires:			gfal2-core%{?_isa} >= %{version}
Requires:			gfal2-plugin-lfc%{?_isa} >= %{version}
Requires:			gfal2-plugin-dcap%{?_isa} >= %{version}
Requires:			gfal2-plugin-srm%{?_isa} >= %{version}
Requires:			gfal2-plugin-rfio%{?_isa} >= %{version}

%description all
Install gfal 2.0 and all the standard plugins

%post -p /sbin/ldconfig

%clean
rm -rf "$RPM_BUILD_ROOT";
scons  %{?_smp_mflags} main_core=yes plugin_srm=yes plugin_devel=yes \
plugin_dcap=yes plugin_rfio=yes plugin_lfc=yes main_meta=yes \
production=yes main_devel=yes main_doc=yes -c build

%prep
%setup -q

%build
scons %{?_smp_mflags} main_core=yes plugin_srm=yes plugin_devel=yes \
plugin_dcap=yes plugin_rfio=yes plugin_lfc=yes \
main_meta=yes production=yes main_devel=yes main_doc=yes build

%postun
ldconfig

%install
rm -rf "$RPM_BUILD_ROOT"; 
scons  %{?_smp_mflags} main_core=yes plugin_srm=yes plugin_devel=yes \
plugin_dcap=yes plugin_rfio=yes plugin_lfc=yes main_meta=yes \
production=yes main_devel=yes \
main_doc=yes --install-sandbox="$RPM_BUILD_ROOT" install 



%files core
%defattr (-,root,root)
%{_libdir}/libgfal2.so.*
%{_bindir}/gfal2_version

%files devel
%defattr (-,root,root)
%{_includedir}/gfal2/gfal_api.h
%{_includedir}/gfal2/common/gfal_constants.h
%{_includedir}/gfal2/posix/gfal_posix_api.h
%{_datadir}/gfal2/example/gfal_testchmod.c
%{_datadir}/gfal2/example/gfal_testcreatedir.c
%{_datadir}/gfal2/example/gfal_testdir.c
%{_datadir}/gfal2/example/gfal_testget.c
%{_datadir}/gfal2/example/gfal_testread.c
%{_datadir}/gfal2/example/gfal_testrw.c
%{_datadir}/gfal2/example/gfal_teststat.c
%{_libdir}/pkgconfig/libgfal2.pc
%{_libdir}/libgfal2.so

%files doc
%defattr (-,root,root)
%{_datadir}/doc/gfal2/*

%files plugin-lfc
%defattr (-,root,root)
%{_libdir}/libgfal_plugin_lfc.so
%{_sysconfdir}/profile.d/gfal_plugin_lfc.csh
%{_sysconfdir}/profile.d/gfal_plugin_lfc.sh

%files plugin-rfio
%defattr (-,root,root)
%{_libdir}/libgfal_plugin_rfio.so
%{_sysconfdir}/profile.d/gfal_plugin_rfio.csh
%{_sysconfdir}/profile.d/gfal_plugin_rfio.sh

%files plugin-dcap
%defattr (-,root,root)
%{_libdir}/libgfal_plugin_dcap.so
%{_sysconfdir}/profile.d/gfal_plugin_dcap.csh
%{_sysconfdir}/profile.d/gfal_plugin_dcap.sh

%files plugin-srm
%defattr (-,root,root)
%{_libdir}/libgfal_plugin_srm.so
%{_sysconfdir}/profile.d/gfal_plugin_srm.csh
%{_sysconfdir}/profile.d/gfal_plugin_srm.sh

%files all
%defattr (-,root,root)
%{_datadir}/gfal2/LICENSE

%files plugin-devel
%defattr (-,root,root)
%{_includedir}/gfal2/common/gfal_common_plugin_interface.h
%{_includedir}/gfal2/common/gfal_prototypes.h
%{_includedir}/gfal2/common/gfal_types.h
%{_includedir}/gfal2/common/gfal_common_plugin.h

%changelog
* Mon Dec 12 2011 adevress at cern.ch 2.0.0-1.17_preview
 - Initial gfal 2.0 preview release
