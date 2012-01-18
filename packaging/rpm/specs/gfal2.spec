Name:				gfal2
Version:			2.0.0
Release:			5beta1%{?dist}
Summary:			Grid file access library 2.0
Group:				Applications/Internet
License:			ASL 2.0
URL:				https://svnweb.cern.ch/trac/lcgutil/wiki/gfal2
# svn export http://svn.cern.ch/guest/lcgutil/gfal/branches/gfal_2_0_main gfal2
Source0:			http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/tar/%{name}/%{name}-%{version}.tar.gz 
BuildRoot:			%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

#main lib dependencies
BuildRequires:		cmake
BuildRequires:		glib2-devel
BuildRequires:		openldap-devel
BuildRequires:		libattr-devel
## libuuid is in a different rpm for el5
%if 0%{?el5}
BuildRequires:		e2fsprogs-devel
%else
BuildRequires:		libuuid-devel	
%endif
#lfc plugin dependencies
BuildRequires:		lfc-devel
#rfio plugin dependencies
BuildRequires:		dpm-devel
#srm plugin dependencies
BuildRequires:		srm-ifce-devel
#dcap plugin dependencies
BuildRequires:		dcap-devel

Requires:			%{name}-core = %{version}

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
Requires:			%{name}%{?_isa} = %{version}
Requires:			libattr-devel%{?_isa} 
Requires:			pkgconfig%{?_isa}

%description devel
development files for GFAL 2.0

%package doc
Summary:			Documentation for GFAL 2.0 core
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description doc
Doxygen documentation of gfal 2.0 .

%package plugin-lfc
Summary:			Provide the lfc access for gfal2.0
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description plugin-lfc
Provide the lfc access (lfn :// ) for gfal2.0, lfc plugin

%package plugin-rfio
Summary:			Provide the rfio access for gfal2.0
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}
Requires:			dpm-libs%{?_isa}

%description plugin-rfio
Provide the rfio access (rfio:// ) for gfal2.0, rfio plugin

%package plugin-dcap
Summary:			Provide the dcap access for gfal2.0
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description plugin-dcap
Provide the dcap access (gsidcap://, dcap:// ) for gfal2.0, dcap plugin

%package plugin-srm
Summary:			Provide the srm access for gfal2.0
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description plugin-srm
Provide the srm access (srm:// ) for gfal2.0, srm plugin

%package plugin-devel
Summary:			Development files for GFAL 2.0 plugin development
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description plugin-devel
Provide the headers files for plugin development

%package all
Summary:			Meta package for gfal 2.0 global install
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}
Requires:			%{name}-plugin-lfc%{?_isa} = %{version}
Requires:			%{name}-plugin-dcap%{?_isa} = %{version}
Requires:			%{name}-plugin-srm%{?_isa} = %{version}
Requires:			%{name}-plugin-rfio%{?_isa} = %{version}

%description all
Install gfal 2.0 and all standard plugins

%post -p /sbin/ldconfig

%clean
rm -rf "$RPM_BUILD_ROOT";
make clean

%prep
%setup -q

%build
%cmake -DDOC_INSTALL_DIR=%{_docdir}/%{name}-%{version} .
make %{?_smp_mflags}

%post core -p /sbin/ldconfig

%postun core -p /sbin/ldconfig

%post plugin-lfc -p /sbin/ldconfig

%postun plugin-lfc -p /sbin/ldconfig

%post plugin-rfio -p /sbin/ldconfig

%postun plugin-rfio -p /sbin/ldconfig

%post plugin-srm -p /sbin/ldconfig

%postun plugin-srm -p /sbin/ldconfig

%post plugin-dcap -p /sbin/ldconfig

%postun plugin-dcap -p /sbin/ldconfig

%install
rm -rf "$RPM_BUILD_ROOT"; 
make %{?_smp_mflags} DESTDIR=$RPM_BUILD_ROOT install

%files
%defattr (-,root,root)
%{_bindir}/gfal2_version
%{_docdir}/%{name}-%{version}/DESCRIPTION
%{_docdir}/%{name}-%{version}/VERSION

%files core
%defattr (-,root,root)
%{_libdir}/libgfal2.so.*
%{_docdir}/%{name}-%{version}/LICENSE

%files devel
%defattr (-,root,root)
%{_includedir}/gfal2/gfal_api.h
%{_includedir}/gfal2/common/gfal_constants.h
%{_includedir}/gfal2/posix/gfal_posix_api.h
%{_docdir}/%{name}-%{version}/examples/*.c
%{_libdir}/pkgconfig/gfal2.pc
%{_libdir}/libgfal2.so
%{_docdir}/%{name}-%{version}/RELEASE-NOTES

%files doc
%defattr (-,root,root)
%{_docdir}/%{name}-%{version}/html/*

%files plugin-lfc
%defattr (-,root,root)
%{_libdir}/libgfal_plugin_lfc.so*
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_lfc.csh
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_lfc.sh

%files plugin-rfio
%defattr (-,root,root)
%{_libdir}/libgfal_plugin_rfio.so*
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_rfio.csh
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_rfio.sh

%files plugin-dcap
%defattr (-,root,root)
%{_libdir}/libgfal_plugin_dcap.so*
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_dcap.csh
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_dcap.sh

%files plugin-srm
%defattr (-,root,root)
%{_libdir}/libgfal_plugin_srm.so*
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_srm.csh
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_srm.sh

%files all
%defattr (-,root,root)
%{_docdir}/%{name}-%{version}/README

%files plugin-devel
%defattr (-,root,root)
%{_includedir}/gfal2/common/gfal_common_plugin_interface.h
%{_includedir}/gfal2/common/gfal_prototypes.h
%{_includedir}/gfal2/common/gfal_types.h
%{_includedir}/gfal2/common/gfal_common_plugin.h

%changelog
* Mon Dec 12 2011 adevress at cern.ch 2.0.0-1.17_preview
 - Initial gfal 2.0 preview release
