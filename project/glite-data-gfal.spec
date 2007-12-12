Summary: Grid File Access Library
Name: GFAL-client
Version: @MODULE.VERSION@
Release: @MODULE.BUILD@
Source0: glite-data-gfal-%{version}_bin.tar.gz
License: Open Source EGEE License
Group: Application/File
BuildRoot: %{_builddir}/%{name}-%{version}
Prefix: /opt/lcg
AutoReqProv: no
Requires: CGSI_gSOAP_2.7 >= 1.2.0-1, vdt_globus_essentials, openldap, libdl.so.2, libuuid.so.1, glite-security-voms-api-c >= 1.7.24
Provides: libgfal.so, libgfal_pthr.so
%define __spec_install_post %{nil}
%define debug_package %{nil}
%description
The Grid File Access Library offers a POSIX interface to Replica Catalogs,
Storage Resource Managers and File Access services using protocols like rfio,
dcap...

%prep
%setup -c

%build

%install

%clean

%files
%defattr(-,root,root)
%{prefix}/bin/gfal_*
%{prefix}/include/gfal*.h
%{prefix}/%{_lib}/libgfal*
%{prefix}/%{_lib}/python/*
%doc %{prefix}/man/man3/gfal*.3
%doc %{prefix}/share/doc/GFAL/*
