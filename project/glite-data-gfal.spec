Summary: Grid File Access Library
Name: GFAL-client
Version: 1.3.0
Release: 1
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Application/File
BuildRoot: %{_builddir}/%{name}-%{version}-root
Prefix: /opt/lcg
%define __spec_install_post %{nil}
%description
The Grid File Access Library offers a POSIX interface to Replica Catalogs,
Storage Resource Managers and File Access services using protocols like rfio,
dcap...
%prep
%setup -q
%build
make

%install
rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install
make DESTDIR=$RPM_BUILD_ROOT install.man

%clean
rm -rf $RPM_BUILD_ROOT
%files
%defattr(-,root,root)
/opt/lcg/bin/gfal_testdir
/opt/lcg/bin/gfal_testread
/opt/lcg/bin/gfal_testrw
/opt/lcg/bin/gfal_teststat
/opt/lcg/include/gfal_api.h
/opt/lcg/include/gfal_constants.h
/opt/lcg/lib/libgfal.a
/opt/lcg/lib/libgfal.so
/opt/lcg/man/man3/gfal.3
/opt/lcg/man/man3/gfal_access.3
/opt/lcg/man/man3/gfal_chmod.3
/opt/lcg/man/man3/gfal_close.3
/opt/lcg/man/man3/gfal_closedir.3
/opt/lcg/man/man3/gfal_creat.3
/opt/lcg/man/man3/gfal_lseek.3
/opt/lcg/man/man3/gfal_mkdir.3
/opt/lcg/man/man3/gfal_open.3
/opt/lcg/man/man3/gfal_opendir.3
/opt/lcg/man/man3/gfal_read.3
/opt/lcg/man/man3/gfal_readdir.3
/opt/lcg/man/man3/gfal_rename.3
/opt/lcg/man/man3/gfal_rmdir.3
/opt/lcg/man/man3/gfal_stat.3
/opt/lcg/man/man3/gfal_unlink.3
/opt/lcg/man/man3/gfal_write.3
