#!/usr/bin/python
##
# @file SConstruct build file
# @author : Devresse Adrien
# @version : v1
# @date 11/08/2011
# modification of the script ofr scons 1.3 from epel

import os
import subprocess
import glob
import time

from SConsAddons import packaging_lib
from SConsAddons import changelog_utils
from SConsAddons import ArgumentUtils
from SConsAddons import EnvironmentBuilder
from SConsAddons import ReleaseDebugConfig

## global vars
# scopes
debug_mode = False;
plugin_dcap = ArgumentUtils.Parameter_bool("plugin_dcap", False) 
plugin_rfio= ArgumentUtils.Parameter_bool("plugin_rfio", False)
plugin_srm= ArgumentUtils.Parameter_bool("plugin_srm", False)
plugin_lfc= ArgumentUtils.Parameter_bool("plugin_lfc", False)
main_core= ArgumentUtils.Parameter_bool("main_core", False)
main_devel= ArgumentUtils.Parameter_bool("main_devel", False)
main_doc= ArgumentUtils.Parameter_bool("main_doc", False)
main_meta= ArgumentUtils.Parameter_bool("main_meta", False)
main_tests= ArgumentUtils.Parameter_bool("main_tests", False)
plugin_devel = ArgumentUtils.Parameter_bool("plugin_devel", False)

## config var
isifce= ArgumentUtils.Parameter_bool("isifce", False)

##


# global var
etics_build_dir= "/usr/" # disable
version_major= '2'
version_minor= '0'
version_patch= '0'
version = ".".join([version_major, version_minor, version_patch])
package_version = '1.17_preview'
license_type = "Apache Software License"

scons_mods_files = Glob("#SConsAddons/*.py")
changelog = changelog_utils.get_rpm_changelog_from_file()

build_deps = scons_mods_files + [File("#CHANGELOG")]

## generic function to get conf value
def get_depconf(key_value, include_path='/include/', lib_path='/lib/', lib64_path='/lib64/', etics_suffix="/stage/"):
	if ARGUMENTS.get(key_value,'0') !='0':
		tmp_path = ARGUMENTS.get(key_value,'0')
	else:
		tmp_path= etics_build_dir
	return ([ tmp_path+ include_path],[ tmp_path + lib64_path, tmp_path + lib_path ] )




# glib get conf
#### -> define by pkg config
# get dpm conf
dpm_header_dir, dpm_lib_dir = get_depconf('dpm_path', include_path='/include/lcgdm/')
dpm_header_dir_emi, dpm_lib_dir_emi = get_depconf('dpm_path_emi', include_path='/include/dpm/')
# get dcap conf
dcap_header_dir, dcap_lib_dir = get_depconf('dcap_path')
# get srmifce conf
srmifce_header_dir, srmifce_lib_dir = get_depconf('srmifce_path')
# get lfc conf
lfc_header_dir, lfc_lib_dir = get_depconf('lfc_path', include_path='/include/lfc/')
# get cgreen conf
cgreen_header_dir, cgreen_lib_dir = get_depconf('cgreen_path', include_path='/local/include', lib_path='/local/lib/', lib64_path='/local/lib64/')
# get is interface path
isifce_header_dir, isifce_lib_dir = get_depconf('isifce_path', include_path='include', lib_path='/lib/', lib64_path='/lib64/')

# old gfal header for testing/example 
old_gfal_header= get_depconf('old_gfal_path')[0]

link_libs= ['m','uuid','c','dl']

build_dir = 'build'

#related var for headers:
build_dir_src = build_dir +'/src'
build_dir_externals = build_dir_src + '/externals'
build_dir_test= build_dir +'/test/src'


	
	
headers= ['.', '#.', '#build/src/'] +   dpm_header_dir+ dcap_header_dir+ srmifce_header_dir +lfc_header_dir  + dpm_header_dir_emi + cgreen_header_dir + isifce_header_dir	
libs=[ '#'+build_dir+'/libs'] +   dpm_lib_dir+ dcap_lib_dir+ srmifce_lib_dir+ lfc_lib_dir+ dpm_lib_dir_emi + cgreen_lib_dir + isifce_lib_dir
cflags=['-DVERSION='+version, '-Wall','-D_LARGEFILE64_SOURCE','-pthread' ] # 
# create default env
env = Environment(tools=['default', 'packaging'], CPPPATH= headers, LIBPATH=libs, CFLAGS=cflags, LIBS=link_libs)


env["VERSION_MAJOR"]=version_major
env["VERSION_MINOR"]=version_minor
env["VERSION_PATCH"]=version_patch


env.ParseConfig('pkg-config --cflags --libs glib-2.0')
env.ParseConfig('pkg-config --libs gthread-2.0')
## add generic builder for symlink
builder = Builder(action = "cd ${TARGET.dir} && ln -s ${SOURCE.file} ${TARGET.file}", chdir = False)
env.Append(BUILDERS = {"Symlink" : builder})

r = os.getenv('LD_LIBRARY_PATH')	# get ld path
env['ENV']['LD_LIBRARY_PATH'] = (r is not None) and r or "" # set ld path or empty one if not exist
env.PrependENVPath('LD_LIBRARY_PATH', map(lambda x :Dir(x).get_abspath(), libs)) # setup internal env LD PATH

print " ld path : " + env['ENV']['LD_LIBRARY_PATH'] 

env = ReleaseDebugConfig.ConfigureBuildTarget(env) # add rel/debug/profile
	

#externals builds
env_libgcache = env.Clone()
env_gskiplist = env.Clone()
VariantDir(build_dir_externals, 'src/externals/')
gsimplecache = SConscript(build_dir_externals +'/gsimplecache/SConscript',['headers', 'libs', 'env_libgcache'])
static_gskiplist, OS_gskiplist = SConscript(build_dir_externals +'/gskiplist/SConscript',['headers', 'libs', 'env_gskiplist'])

#main build
VariantDir(build_dir_src, 'src')
mainlib, staticlib, versionexe,plugin_lfc_lib, plugin_srm_lib, plugin_rfio_lib, plugin_dcap_lib, all_headers  = SConscript(build_dir_src +'/SConscript',['env', 'headers', 'libs', 'build_dir_src','debug_mode', 'isifce', 'build_deps'])

# global testing build
SConscript('testing/SConscript', ['env', 'headers', 'libs', 'old_gfal_header','debug_mode', 'build_deps'])

#unit tests
env_test = env.Clone()
env_test.Append(CPPPATH=[ "#src/common", "#src/", "#src/posix"]+ cgreen_header_dir)
env_test.Append(LIBPATH= cgreen_lib_dir)
VariantDir(build_dir_test, 'test')
tests = SConscript(build_dir_test +'/SConscript',['env_test', 'headers', 'libs', 'build_dir_src','debug_mode', 'all_headers','static_gskiplist', 'build_deps'])

#VariantDir("rpmbuildir/", 'rpm/')
#SConscript("rpmbuildir/SConscript", ["env","mainlib", "staticlib", "versionexe", "version", "package_version", "plugin_lfc_lib"] )



#packaging staff
libdir = (os.uname()[4] == 'x86_64') and "lib64" or "lib"

def define_rpm_install(opt):
	return 'scons '+ opt+ ' --install-sandbox="$RPM_BUILD_ROOT" install '
	
def define_rpm_build(opt):
	return 'scons '+ opt+ ' build'


def arguments_to_str():
	ret = ' ';
	for arg, value in ARGUMENTS.iteritems():
		ret += arg+ '=' +value+ ' '
	return ret

pack_list = []
install_list = []
comp_list = []

if(main_core):
	comp_list += [ mainlib, versionexe ]
	lib_main = env.Install('/usr/'+libdir+'/', mainlib)
	version_main = env.Install('/usr/bin/', versionexe)
	install_list += [lib_main, version_main] 
	x_rpm_install = define_rpm_install(arguments_to_str());
	pack_list += env.Package( 
			 NAME     = 'gfal2-core',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = license_type,
			 CHANGELOG		= changelog,
			 SUMMARY        = 'Grid file access library 2.0',
			 DESCRIPTION    = 'POSIX abtraction layer for grid storage system',
			 X_RPM_GROUP    = 'System Environment/Libraries',
			 X_RPM_INSTALL= x_rpm_install,
			 X_RPM_BUILD = define_rpm_build(arguments_to_str()),
			 X_RPM_POSTINSTALL = "ldconfig",
			 X_RPM_POSTUNINSTALL = "ldconfig",
			 X_RPM_BUILDREQUIRES = 'scons, glib2-devel, openldap-devel, libattr-devel',			 
			 X_RPM_REQUIRES = 'openldap',
			 source= [lib_main, version_main] 
			 )



if(main_devel):
	header_main = env.Install('/usr/include/gfal2/', "src/gfal_api.h" )
	header_main2= env.Install('/usr/include/gfal2/common/', "src/common/gfal_constants.h")
	header_main3= env.Install('/usr/include/gfal2/posix/', "src/posix/gfal_posix_api.h" )
	example_main = env.Install('/usr/share/gfal2/example/', Glob("testing/example/*.c"))
	pkg_config = env.Install('/usr/lib64/pkgconfig/', Glob('dist/lib64/pkgconfig/libgfal2.pc'))
	install_list += [header_main, header_main2, header_main3, example_main, pkg_config] 
	x_rpm_install = define_rpm_install(arguments_to_str());
	pack_list += env.Package( 
			 NAME     = 'gfal2-devel',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = license_type,			 
			 CHANGELOG		= changelog,			 
			 SUMMARY        = 'development files for grid file access library 2.0',
			 DESCRIPTION    = 'development files for grid file access library 2.0',
			 X_RPM_GROUP    = 'Development/Libraries',
			 X_RPM_BUILD = define_rpm_build(arguments_to_str()),			 
			 X_RPM_INSTALL= x_rpm_install,
			 X_RPM_REQUIRES = 'glib2-devel, gfal2-core',
			 source= [header_main, header_main2, header_main3, example_main, pkg_config] 
			 )
			 
			 
if(plugin_devel):
	header_main = env.Install('/usr/include/gfal2/common/', "src/common/gfal_common_plugin_interface.h" )
	header_main2= env.Install('/usr/include/gfal2/common/', "src/common/gfal_prototypes.h")
	header_main3= env.Install('/usr/include/gfal2/common/', "src/common/gfal_types.h" )
	header_main4= env.Install('/usr/include/gfal2/common/', "src/common/gfal_common_plugin.h" )
	install_list += [header_main, header_main2, header_main3, header_main4] 
	x_rpm_install = define_rpm_install(arguments_to_str());
	pack_list += env.Package( 
			 NAME     = 'gfal2-plugin-devel',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = license_type,
			 SUMMARY        = 'development files for the gfal 2.0 plugins ',
			 DESCRIPTION    = 'development files for the plugins of the grid file access library 2.0',
			 X_RPM_GROUP    = 'Development/Libraries',
			 CHANGELOG		= changelog,			
			 X_RPM_INSTALL= x_rpm_install,
			 X_RPM_POSTINSTALL = "ldconfig",			
			 X_RPM_REQUIRES = 'glib2-devel, gfal2-core-devel',
			 source= [header_main, header_main2, header_main3, header_main4] 
			 )
	
if(plugin_lfc):
	comp_list += [ plugin_lfc_lib]
	lib_plugin_lfc = env.Install('/usr/'+libdir+'/', plugin_lfc_lib)
	lib_plugin_lfc_conf= env.Install('/etc/profile.d/', Glob("dist/scripts/gfal_plugin_lfc/*sh"))
	x_rpm_install = define_rpm_install(arguments_to_str());
	install_list += [lib_plugin_lfc, lib_plugin_lfc_conf]
	pack_list += env.Package( 
			 NAME     = 'gfal2-plugin-lfc',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = license_type,
			 CHANGELOG		= changelog,			 
			 SUMMARY        = 'plugin lfc for gfal 2.0',
			 DESCRIPTION    = 'Provide the lfc access for gfal2.0',
			 X_RPM_GROUP    = 'System Environment/Libraries',
			 X_RPM_REQUIRES = 'lfc-libs, glib2, gfal2-core ',
			 X_RPM_POSTINSTALL = "ldconfig",
			 X_RPM_BUILD = define_rpm_build(arguments_to_str()),			 
			 X_RPM_INSTALL= x_rpm_install,
			 source= [lib_plugin_lfc, lib_plugin_lfc_conf],
			 )	
	


if(plugin_srm):
	comp_list += [ plugin_srm_lib]
	lib_plugin_srm = env.Install('/usr/'+libdir+'/', plugin_srm_lib)
	lib_plugin_srm_conf= env.Install('/etc/profile.d/', Glob("dist/scripts/gfal_plugin_srm/*sh"))
	x_rpm_install = define_rpm_install(arguments_to_str());
	install_list += [lib_plugin_srm, lib_plugin_srm_conf]
	pack_list += env.Package( 
			 NAME     = 'gfal2-plugin-srm',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = license_type,
			 CHANGELOG		= changelog,			 
			 SUMMARY        = 'plugin srm for gfal 2.0',
			 DESCRIPTION    = 'Provide the srm access for gfal2.0',
			 X_RPM_GROUP    = 'System Environment/Libraries',
			 X_RPM_REQUIRES = 'glib2, gfal2-core, srm-ifce ',
			 X_RPM_POSTINSTALL = "ldconfig",
			 X_RPM_BUILD = define_rpm_build(arguments_to_str()),			 
			 X_RPM_INSTALL= x_rpm_install,
			 source= [lib_plugin_srm, lib_plugin_srm_conf],
			 )	

	
if(plugin_rfio):
	comp_list += [ plugin_rfio_lib]
	lib_plugin_rfio = env.Install('/usr/'+libdir+'/', plugin_rfio_lib)
	lib_plugin_rfio_conf= env.Install('/etc/profile.d/', Glob("dist/scripts/gfal_plugin_rfio/*sh"))
	x_rpm_install = define_rpm_install(arguments_to_str());
	install_list += [lib_plugin_rfio, lib_plugin_rfio_conf]
	pack_list += env.Package( 
			 NAME     = 'gfal2-plugin-rfio',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = license_type,
			 CHANGELOG		= changelog,			 
			 SUMMARY        = 'plugin rfio for gfal 2.0',
			 DESCRIPTION    = 'Provide the rfio access for gfal2.0',
			 X_RPM_GROUP    = 'System Environment/Libraries',
			 X_RPM_REQUIRES = 'dpm-libs, glib2, gfal2-core ',
			 X_RPM_POSTINSTALL = "ldconfig",	
			 X_RPM_BUILD = define_rpm_build(arguments_to_str()),			 		 
			 X_RPM_INSTALL= x_rpm_install,
			 source= [lib_plugin_rfio, lib_plugin_rfio_conf],
			 )	
	

if(plugin_dcap):
	comp_list += [ plugin_dcap_lib]
	lib_plugin_dcap = env.Install('/usr/'+libdir+'/', plugin_dcap_lib)
	lib_plugin_dcap_conf= env.Install('/etc/profile.d/', Glob("dist/scripts/gfal_plugin_dcap/*sh"))
	x_rpm_install = define_rpm_install(arguments_to_str());
	install_list +=  [lib_plugin_dcap, lib_plugin_dcap_conf]
	pack_list += env.Package( 
			 NAME     = 'gfal2-plugin-dcap',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = license_type,
			 CHANGELOG		= changelog,			 
			 SUMMARY        = 'plugin dcap for gfal 2.0',
			 DESCRIPTION    = 'Provide the dcap access for gfal2.0',
			 X_RPM_GROUP    = 'System Environment/Libraries',
			 X_RPM_REQUIRES = 'dcap-libs, glib2, gfal2-core ',
			 X_RPM_POSTINSTALL = "ldconfig",	
			 X_RPM_BUILD = define_rpm_build(arguments_to_str()),			 		 
			 X_RPM_INSTALL= x_rpm_install,
			 source= [lib_plugin_dcap, lib_plugin_dcap_conf],
			 )	
	
	
if(main_doc):
	docs_main = env.Install('/usr/share/doc/gfal2/', Glob('doc/build/html/*') )
	env.Depends(docs_main, Glob('doc/build/html/*') )	
	install_list +=  [docs_main]
	pack_list += env.Package( 
				 NAME     = 'gfal2-doc',
				 VERSION        = version,
				 PACKAGEVERSION = package_version,
				 PACKAGETYPE    = 'rpm',
				 LICENSE        = license_type,
				CHANGELOG		= changelog,				 
				 SUMMARY        = 'doc gfal2.0',
				 DESCRIPTION    = 'Documentation package for gfal2.X',
				 X_RPM_GROUP    = 'CERN/grid',
				 X_RPM_REQUIRES = 'gfal2-core ',
				 X_RPM_INSTALL= define_rpm_install(arguments_to_str()),		 
				 source= [docs_main] 
				 )	

if(main_meta):
	license1 = env.Install('/usr/share/gfal2/', 'LICENSE' )	
	install_list +=  [license1] 
	pack_list +=  env.Package( 
				 NAME     = 'gfal2-all',
				 VERSION        = version,
				 PACKAGEVERSION = package_version,
				 PACKAGETYPE    = 'rpm',
				 LICENSE        = license_type,
				 CHANGELOG		= changelog,				 
				 SUMMARY        = 'gfal2.0 meta package',
				 DESCRIPTION    = 'meta package for gfal2.X',
				 X_RPM_GROUP    = 'CERN/grid',
				 X_RPM_REQUIRES = 'gfal2-core, gfal2-doc, gfal2-plugin-lfc, gfal2-plugin-dcap, gfal2-plugin-rfio, gfal2-plugin-srm',
				 X_RPM_INSTALL= define_rpm_install(arguments_to_str()),
				 source= [license1] 
				 )	


if(main_tests):
	main_test = env.Install('/usr/share/gfal2/tests/mocked/', [ tests,Glob("dist/usr/share/gfal2/tests/mocked/*")]  )
	install_list += [main_test] 
	x_rpm_install = define_rpm_install(arguments_to_str());
	pack_list += env.Package( 
			 NAME     = 'gfal2-tests-devel',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 CHANGELOG		= changelog,			 
			 LICENSE        = license_type,
			 SUMMARY        = 'binaries tests for GFAL 2.0',
			 DESCRIPTION    = 'binaries tests for GFAL 2.0',
			 X_RPM_GROUP    = 'Development/Tools',
			 X_RPM_BUILD = define_rpm_build(arguments_to_str()),			 
			 X_RPM_INSTALL= x_rpm_install,
			 X_RPM_REQUIRES = 'gfal2-all',
			 source= [main_test] 
			 )	

env.Alias("install", install_list);

env.Alias("build", comp_list);

env.Alias("package_generator", pack_list);

	
	
	
	 	
	

	

