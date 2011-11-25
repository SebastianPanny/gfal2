%define projectname gfal2
%define version 2.0
%define release 1.17_preview


%define debug_package %{nil}

Name: %{projectname}
License: Apache-2.0
Summary: Grid file access library 2.0
Version: %{version}
Release: %{release}
Group: Grid/lcg
BuildRoot: %{_tmppath}/%{projectname}-%{version}-%{release}
Source: %{projectname}-%{version}-%{release}.src.tar.gz
%description
POSIX abtraction layer for the grid storage systems ( EMI project )


%package core
Summary: core of the Grid File access Library 2.0
Group: grid/lcg
BuildRequires: scons, glib2-devel, openldap-devel, libattr-devel, e2fsprogs-devel
AutoReqProv: yes
Requires: openldap
%description core
Core and main files of GFAL 2.0

%package devel
Summary: development files for GFAL 2.0
Group: grid/lcg
BuildRequires: scons
AutoReqProv: yes
Requires: gfal2-core >= %{version}
%description devel
development files for GFAL 2.0, contain headers, static libraries, pkg-config files.

%package doc
Summary: documentation for GFAL 2.0 core
Group: grid/lcg
BuildRequires: scons
Requires: gfal2-core >= %{version}
%description doc
development files for GFAL 2.0, contain headers, static libraries, pkg-config files.

%package plugin-lfc
Summary: Provide the lfc access for gfal2.0
Group: grid/lcg
BuildRequires: scons, lfc-devel
Requires: gfal2-core >= %{version} , lfc-libs
%description plugin-lfc
Provide the lfc access for gfal2.0, lfc plugin

%package plugin-rfio
Summary: Provide the rfio access for gfal2.0
Group: grid/lcg
BuildRequires: scons, dpm-devel
Requires: gfal2-core >= %{version} , dpm-libs
%description plugin-rfio
Provide the rfio access for gfal2.0, rfio plugin

%package plugin-dcap
Summary: Provide the dcap access for gfal2.0
Group: grid/lcg
BuildRequires: scons, dcap-devel
Requires: gfal2-core >= %{version} , dcap-libs
%description plugin-dcap
Provide the dcap access for gfal2.0, dcap plugin

%package plugin-srm
Summary: Provide the srm access for gfal2.0
Group: grid/lcg
BuildRequires: scons, srm-ifce-devel
Requires: gfal2-core >= %{version} , srm-ifce-libs
%description plugin-srm
Provide the srm access for gfal2.0, dcap plugin

%package plugin-devel
Summary: development files for GFAL 2.0 plugin development
Group: grid/lcg
BuildRequires: scons
Requires: gfal2-core >= %{version} 
%description plugin-devel
Provide the headers files for plugin development

%package all
Summary: Meta package for gfal 2.0 global install
Group: grid/lcg
BuildRequires: scons
Requires: gfal2-core >= %{version}
%description all
Install gfal 2.0 and all the associated plugins





%post
ldconfig

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT";
scons  main_core=yes plugin_srm=yes plugin_devel=yes plugin_dcap=yes plugin_rfio=yes plugin_lfc=yes main_meta=yes production=yes main_devel=yes main_doc=yes -c build

%prep
%setup -q

%build
NUMCPU=`grep processor /proc/cpuinfo | wc -l`; if [[ "$NUMCPU" == "0" ]]; then NUMCPU=1; fi;
scons -j $NUMCPU main_core=yes plugin_srm=yes plugin_devel=yes plugin_dcap=yes plugin_rfio=yes plugin_lfc=yes main_meta=yes production=yes main_devel=yes main_doc=yes build

%postun
ldconfig

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"; 
NUMCPU=`grep processor /proc/cpuinfo | wc -l`; if [[ "$NUMCPU" == "0" ]]; then NUMCPU=1; fi;
scons  -j $NUMCPU main_core=yes plugin_srm=yes plugin_devel=yes plugin_dcap=yes plugin_rfio=yes plugin_lfc=yes main_meta=yes production=yes main_devel=yes main_doc=yes --install-sandbox="$RPM_BUILD_ROOT" install 



%files core
%defattr (-,root,root)
/usr/%{_lib}/libgfal2.so.*
/usr/%{_lib}/libgfal2.so
/usr/bin/gfal2_version

%files devel
%defattr (-,root,root)
/usr/include/gfal2/gfal_api.h
/usr/include/gfal2/common/gfal_constants.h
/usr/include/gfal2/posix/gfal_posix_api.h
/usr/%{_lib}/libgfal2.a
/usr/share/gfal2/example/gfal_testchmod.c
/usr/share/gfal2/example/gfal_testcreatedir.c
/usr/share/gfal2/example/gfal_testdir.c
/usr/share/gfal2/example/gfal_testget.c
/usr/share/gfal2/example/gfal_testread.c
/usr/share/gfal2/example/gfal_testrw.c
/usr/share/gfal2/example/gfal_teststat.c
/usr/%{_lib}/pkgconfig/libgfal2.pc

%files doc
%defattr (-,root,root)
/usr/share/doc/gfal2/annotated.html
/usr/share/doc/gfal2/api_change.html
/usr/share/doc/gfal2/apl.html
/usr/share/doc/gfal2/doxygen.css
/usr/share/doc/gfal2/doxygen.png
/usr/share/doc/gfal2/examples.html
/usr/share/doc/gfal2/faq.html
/usr/share/doc/gfal2/files.html
/usr/share/doc/gfal2/functions.html
/usr/share/doc/gfal2/functions_vars.html
/usr/share/doc/gfal2/gcachemain_8h-source.html
/usr/share/doc/gfal2/gfal__api_8h-source.html
/usr/share/doc/gfal2/gfal__common__all_8c.html
/usr/share/doc/gfal2/gfal__common__config_8c.html
/usr/share/doc/gfal2/gfal__common__config_8h-source.html
/usr/share/doc/gfal2/gfal__common__config_8h.html
/usr/share/doc/gfal2/gfal__common__dir__handle_8c.html
/usr/share/doc/gfal2/gfal__common__dir__handle_8h-source.html
/usr/share/doc/gfal2/gfal__common__dir__handle_8h.html
/usr/share/doc/gfal2/gfal__common__errverbose_8c.html
/usr/share/doc/gfal2/gfal__common__errverbose_8h-source.html
/usr/share/doc/gfal2/gfal__common__errverbose_8h.html
/usr/share/doc/gfal2/gfal__common__file__handle_8c.html
/usr/share/doc/gfal2/gfal__common__file__handle_8h-source.html
/usr/share/doc/gfal2/gfal__common__file__handle_8h.html
/usr/share/doc/gfal2/gfal__common__filedescriptor_8c.html
/usr/share/doc/gfal2/gfal__common__filedescriptor_8h-source.html
/usr/share/doc/gfal2/gfal__common__filedescriptor_8h.html
/usr/share/doc/gfal2/gfal__common__interface_8h-source.html
/usr/share/doc/gfal2/gfal__common__interface_8h.html
/usr/share/doc/gfal2/gfal__common__internal_8h-source.html
/usr/share/doc/gfal2/gfal__common__internal_8h.html
/usr/share/doc/gfal2/gfal__common__lfc_8c.html
/usr/share/doc/gfal2/gfal__common__lfc_8h-source.html
/usr/share/doc/gfal2/gfal__common__lfc_8h.html
/usr/share/doc/gfal2/gfal__common__lfc__open_8c.html
/usr/share/doc/gfal2/gfal__common__lfc__open_8h-source.html
/usr/share/doc/gfal2/gfal__common__mds_8h-source.html
/usr/share/doc/gfal2/gfal__common__mds__ldap__internal_8c.html
/usr/share/doc/gfal2/gfal__common__mds__ldap__internal_8h-source.html
/usr/share/doc/gfal2/gfal__common__mds__ldap__internal__layer_8h-source.html
/usr/share/doc/gfal2/gfal__common__mds__ldap__internal__layer_8h.html
/usr/share/doc/gfal2/gfal__common__parameter_8c.html
/usr/share/doc/gfal2/gfal__common__parameter_8h-source.html
/usr/share/doc/gfal2/gfal__common__parameter_8h.html
/usr/share/doc/gfal2/gfal__common__plugin_8c.html
/usr/share/doc/gfal2/gfal__common__plugin_8h-source.html
/usr/share/doc/gfal2/gfal__common__plugin_8h.html
/usr/share/doc/gfal2/gfal__common__plugin__interface_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm_8h.html
/usr/share/doc/gfal2/gfal__common__srm__access_8c.html
/usr/share/doc/gfal2/gfal__common__srm__access_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__access_8h.html
/usr/share/doc/gfal2/gfal__common__srm__checksum_8c.html
/usr/share/doc/gfal2/gfal__common__srm__checksum_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__checksum_8h.html
/usr/share/doc/gfal2/gfal__common__srm__chmod_8c.html
/usr/share/doc/gfal2/gfal__common__srm__chmod_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__endpoint_8c.html
/usr/share/doc/gfal2/gfal__common__srm__endpoint_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__endpoint_8h.html
/usr/share/doc/gfal2/gfal__common__srm__getxattr_8c.html
/usr/share/doc/gfal2/gfal__common__srm__getxattr_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__getxattr_8h.html
/usr/share/doc/gfal2/gfal__common__srm__internal__layer_8c.html
/usr/share/doc/gfal2/gfal__common__srm__internal__layer_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__internal__layer_8h.html
/usr/share/doc/gfal2/gfal__common__srm__mkdir_8c.html
/usr/share/doc/gfal2/gfal__common__srm__mkdir_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__mkdir_8h.html
/usr/share/doc/gfal2/gfal__common__srm__open_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__opendir_8c.html
/usr/share/doc/gfal2/gfal__common__srm__opendir_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__opendir_8h.html
/usr/share/doc/gfal2/gfal__common__srm__readdir_8c.html
/usr/share/doc/gfal2/gfal__common__srm__readdir_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__readdir_8h.html
/usr/share/doc/gfal2/gfal__common__srm__rmdir_8c.html
/usr/share/doc/gfal2/gfal__common__srm__rmdir_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__rmdir_8h.html
/usr/share/doc/gfal2/gfal__common__srm__stat_8c.html
/usr/share/doc/gfal2/gfal__common__srm__stat_8h-source.html
/usr/share/doc/gfal2/gfal__common__srm__stat_8h.html
/usr/share/doc/gfal2/gfal__constants_8h-source.html
/usr/share/doc/gfal2/gfal__constants_8h.html
/usr/share/doc/gfal2/gfal__dcap__plugin__bindings_8h-source.html
/usr/share/doc/gfal2/gfal__dcap__plugin__layer_8c.html
/usr/share/doc/gfal2/gfal__dcap__plugin__layer_8h-source.html
/usr/share/doc/gfal2/gfal__dcap__plugin__layer_8h.html
/usr/share/doc/gfal2/gfal__dcap__plugin__main_8c.html
/usr/share/doc/gfal2/gfal__dcap__plugin__main_8h-source.html
/usr/share/doc/gfal2/gfal__dcap__plugin__main_8h.html
/usr/share/doc/gfal2/gfal__posix__access_8c.html
/usr/share/doc/gfal2/gfal__posix__api_8h-source.html
/usr/share/doc/gfal2/gfal__posix__chmod_8c.html
/usr/share/doc/gfal2/gfal__posix__close_8c.html
/usr/share/doc/gfal2/gfal__posix__closedir_8c.html
/usr/share/doc/gfal2/gfal__posix__getxattr_8c.html
/usr/share/doc/gfal2/gfal__posix__internal_8h-source.html
/usr/share/doc/gfal2/gfal__posix__local__file_8c.html
/usr/share/doc/gfal2/gfal__posix__local__file_8h-source.html
/usr/share/doc/gfal2/gfal__posix__lseek_8c.html
/usr/share/doc/gfal2/gfal__posix__mkdir_8c.html
/usr/share/doc/gfal2/gfal__posix__ng_8c.html
/usr/share/doc/gfal2/gfal__posix__open_8c.html
/usr/share/doc/gfal2/gfal__posix__opendir_8c.html
/usr/share/doc/gfal2/gfal__posix__parameters_8c.html
/usr/share/doc/gfal2/gfal__posix__read_8c.html
/usr/share/doc/gfal2/gfal__posix__readdir_8c.html
/usr/share/doc/gfal2/gfal__posix__readlink_8c.html
/usr/share/doc/gfal2/gfal__posix__rename_8c.html
/usr/share/doc/gfal2/gfal__posix__rmdir_8c.html
/usr/share/doc/gfal2/gfal__posix__setxattr_8c.html
/usr/share/doc/gfal2/gfal__posix__stat_8c.html
/usr/share/doc/gfal2/gfal__posix__symlink_8c.html
/usr/share/doc/gfal2/gfal__posix__unlink_8c.html
/usr/share/doc/gfal2/gfal__posix__write_8c.html
/usr/share/doc/gfal2/gfal__prototypes_8h-source.html
/usr/share/doc/gfal2/gfal__rfio__plugin__bindings_8h-source.html
/usr/share/doc/gfal2/gfal__rfio__plugin__layer_8c.html
/usr/share/doc/gfal2/gfal__rfio__plugin__layer_8h-source.html
/usr/share/doc/gfal2/gfal__rfio__plugin__main_8c.html
/usr/share/doc/gfal2/gfal__rfio__plugin__main_8h-source.html
/usr/share/doc/gfal2/gfal__testchmod_8c-example.html
/usr/share/doc/gfal2/gfal__testcreatedir_8c-example.html
/usr/share/doc/gfal2/gfal__testdir_8c-example.html
/usr/share/doc/gfal2/gfal__testget_8c-example.html
/usr/share/doc/gfal2/gfal__testread_8c-example.html
/usr/share/doc/gfal2/gfal__testrw_8c-example.html
/usr/share/doc/gfal2/gfal__types_8h-source.html
/usr/share/doc/gfal2/gfal_diagram.png
/usr/share/doc/gfal2/globals.html
/usr/share/doc/gfal2/globals_func.html
/usr/share/doc/gfal2/globals_type.html
/usr/share/doc/gfal2/group__posix__group.html
/usr/share/doc/gfal2/gskiplist_8h-source.html
/usr/share/doc/gfal2/gwatchdog_8h-source.html
/usr/share/doc/gfal2/index.html
/usr/share/doc/gfal2/lfc__ifce__ng_8c.html
/usr/share/doc/gfal2/lfc__ifce__ng_8h-source.html
/usr/share/doc/gfal2/lfc__ifce__ng_8h.html
/usr/share/doc/gfal2/modules.html
/usr/share/doc/gfal2/page_design.html
/usr/share/doc/gfal2/pages.html
/usr/share/doc/gfal2/struct__GSkiplist.html
/usr/share/doc/gfal2/struct__GWatchdog.html
/usr/share/doc/gfal2/struct__gfal__plugin__interface.html
/usr/share/doc/gfal2/struct__gfal__request__state.html
/usr/share/doc/gfal2/struct__gfal__srm__external__call.html
/usr/share/doc/gfal2/struct__plugin__opts.html
/usr/share/doc/gfal2/structgfal__mds__endpoint.html
/usr/share/doc/gfal2/structstructure.html
/usr/share/doc/gfal2/tab_b.gif
/usr/share/doc/gfal2/tab_l.gif
/usr/share/doc/gfal2/tab_r.gif
/usr/share/doc/gfal2/tabs.css

%files plugin-lfc
%defattr (-,root,root)
/usr/%{_lib}/libgfal_plugin_lfc.so
/etc/profile.d/gfal_plugin_lfc.csh
/etc/profile.d/gfal_plugin_lfc.sh

%files plugin-rfio
%defattr (-,root,root)
/usr/%{_lib}/libgfal_plugin_rfio.so
/etc/profile.d/gfal_plugin_rfio.csh
/etc/profile.d/gfal_plugin_rfio.sh

%files plugin-dcap
%defattr (-,root,root)
/usr/%{_lib}/libgfal_plugin_dcap.so
/etc/profile.d/gfal_plugin_dcap.csh
/etc/profile.d/gfal_plugin_dcap.sh

%files plugin-srm
%defattr (-,root,root)
/usr/%{_lib}/libgfal_plugin_srm.so
/etc/profile.d/gfal_plugin_srm.csh
/etc/profile.d/gfal_plugin_srm.sh

%files all
%defattr (-,root,root)
/usr/share/gfal2/LICENSE

%files plugin-devel
%defattr (-,root,root)
/usr/include/gfal2/common/gfal_common_plugin_interface.h
/usr/include/gfal2/common/gfal_prototypes.h
/usr/include/gfal2/common/gfal_types.h
/usr/include/gfal2/common/gfal_common_plugin.h

 
%files

%changelog
* Mon Nov 14 2011 adevress at cern.ch 
 - Initial gfal 2.0 preview release
