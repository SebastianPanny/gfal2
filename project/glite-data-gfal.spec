Summary: Grid File Access Library
Name: GFAL-client
Version: @MODULE.VERSION@
Release: @MODULE.BUILD@
Source0: glite-data-gfal-%{version}_bin.tar.gz
License: Open Source EGEE License
Group: Application/File
BuildRoot: %{_builddir}/%{name}-%{version}
Prefix: /opt/lcg
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
%{prefix}/bin/gfal_test*
%{prefix}/include/gfal*.h
%{prefix}/lib/libgfal*
%doc %{prefix}/man/man3/gfal*.3
