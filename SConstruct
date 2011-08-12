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

##
# try to find etics workspace or check if ETICS_WORKSPACE is defined
def get_etics_dir():
	# check en var
	r = os.environ.get('ETICS_WORKSPACE')
	if(r !=''  and r != None):
		return r	
	# try to find	
	home = os.environ.get('HOME')
	if(os.path.exists(home +'/workspace/.etics')):
		return home+'/workspace/'
	print "NO ETICS WORKSPACE FOUND, configuration set to remote etics build"
	return "";

# global var
etics_build_dir=get_etics_dir()
version= '2.0'
package_version = '1.0_preview'

## generic function to get conf value
def get_depconf(key_value, include_path='/include/', lib_path='/lib/', lib64_path='/lib64/', etics_suffix="/stage/"):
	if ARGUMENTS.get(key_value,'0') !='0':
		tmp_path = ARGUMENTS.get(key_value,'0')
	else:
		tmp_path= etics_build_dir+ etics_suffix 
	return ([ tmp_path+ include_path],[ tmp_path + lib64_path, tmp_path + lib_path ] )

# glib get conf
#### -> define by pkg config
# voms get conf
voms_header_dir, voms_lib_dir = get_depconf('voms_path', include_path='/include/glite/security/voms/')
voms_header_dir_emi, voms_lib_dir_emi = get_depconf('voms_path_emi', include_path='/include/voms/')
# get dpm conf
dpm_header_dir, dpm_lib_dir = get_depconf('dpm_path', include_path='/include/lcgdm/')
# get dcap conf
dcap_header_dir, dcap_lib_dir = get_depconf('dcap_path')
# get srmifce conf
srmifce_header_dir, srmifce_lib_dir = get_depconf('srmifce_path')
# get lfc conf
lfc_header_dir, lfc_lib_dir = get_depconf('lfc_path', include_path='/include/lfc/')

# old gfal header for testing/example 
old_gfal_header= get_depconf('old_gfal_path')[0]

link_libs= ['m','uuid','c','dl','glib-2.0']

build_dir = 'build'

#related var for headers:
build_dir_src = build_dir +'/src'
build_dir_externals = build_dir_src + '/externals'
build_dir_test= build_dir +'/test/src'



	
	
headers= ['.', '#.', '#build/src/'] +  voms_header_dir+ dpm_header_dir+ dcap_header_dir+ srmifce_header_dir +lfc_header_dir+ voms_header_dir_emi
libs=[ '#'+build_dir+'/libs'] +  voms_lib_dir+ dpm_lib_dir+ dcap_lib_dir+ srmifce_lib_dir+ lfc_lib_dir+ voms_lib_dir_emi
cflags=['-DVERSION=\\\"'+version+'\\\"', '-DGFAL_SECURE' , '-D_LARGEFILE64_SOURCE','-DGFAL_ENABLE_RFIO','-DGFAL_ENABLE_DCAP','-pthread' ] # largefile flag needed in 64 bits mod, Version setter, Warning flags and other legacy flags 
# create default env
env = Environment(tools=['default', 'packaging'], CPPPATH= headers, LIBPATH=libs, CFLAGS=cflags, LIBS=link_libs)
env.ParseConfig('pkg-config --cflags --libs glib-2.0')
r = os.getenv('LD_LIBRARY_PATH')	# get ld path
env['ENV']['LD_LIBRARY_PATH'] = (r is not None) and r or "" # set ld path or empty one if not exist

# debug mode
if ARGUMENTS.get('debug','0') =='yes':
	print "DEBUG MODE"
	env.Append(CFLAGS='-g')

# profile mode
if ARGUMENTS.get('profile','0') =='yes':
	print "PROFILE MODE"
	env.Append(CFLAGS='-pg', LINKFLAGS=['-pg'])
	
# prod mode
if ARGUMENTS.get('production','0') =='yes':
	print "prod MODE"
	env.Append(CFLAGS='-O3')
	

#externals builds
env_libgcache = env.Clone()
VariantDir(build_dir_externals, 'src/externals/')
gsimplecache = SConscript(build_dir_externals +'/gsimplecache/SConscript',['headers', 'libs', 'env_libgcache'])

#main build
VariantDir(build_dir_src, 'src')
mainlib, staticlib, versionexe,plugin_lfc_lib, plugin_srm_lib, plugin_rfio_lib, plugin_dcap_lib  = SConscript(build_dir_src +'/SConscript',['env', 'headers', 'libs', 'build_dir_src'])


# global testing build
SConscript('testing/SConscript', ['env', 'headers', 'libs', 'old_gfal_header'])

#unit tests
env_test = env.Clone()
env_test.Append(CPPPATH=[ "#src/common", "#src/", "#src/posix"])
VariantDir(build_dir_test, 'test')
SConscript(build_dir_test +'/SConscript',['env_test', 'headers', 'libs', 'build_dir_src'])


#VariantDir("rpmbuildir/", 'rpm/')
#SConscript("rpmbuildir/SConscript", ["env","mainlib", "staticlib", "versionexe", "version", "package_version", "plugin_lfc_lib"] )



#packaging staff
libdir = (os.uname()[4] == 'x86_64') and "lib64" or "lib"

def define_rpm_install(opt):
	return 'scons -j 8 '+ opt+ ' --install-sandbox="$RPM_BUILD_ROOT" "$RPM_BUILD_ROOT" '

def define_rpm_install_with_file(str1, dest_file, content, mode):
	str1 += " && mkdir -p `dirname " + dest_file + "` "
	str1 +=	' && echo -e   "'+content+'" > '+ dest_file + " "
	str1 +=	" && chmod "+str(mode)+'  '+ dest_file + " "
	return str1

def arguments_to_str():
	ret = ' ';
	for arg, value in ARGUMENTS.iteritems():
		ret += arg+ '=' +value+ ' '
	return ret



def package_main():
	env_pack_main= env.Clone()
	lib_main = env_pack_main.Install('/usr/'+libdir+'/', mainlib)
	version_main = env_pack_main.Install('/usr/bin/', versionexe)
	x_rpm_install = define_rpm_install(arguments_to_str());
	p_main = env_pack_main.Package( 
			 NAME     = 'gfal2-core',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = 'Apache2.0',
			 SUMMARY        = 'grid file access library 2.0',
			 DESCRIPTION    = 'Provide a POSIX like API to manage file and directory with the multiples protocols of the grid',
			 X_RPM_GROUP    = 'CERN/grid',
			 X_RPM_INSTALL= x_rpm_install,
			 X_RPM_REQUIRES = 'glib2',
			 source= [lib_main, version_main] 
			 )
	return p_main

def package_main_devel():
	env_pack_main_devel= env.Clone()
	header_main = env_pack_main_devel.Install('/usr/include/gfal2/', Glob("dist/include/gfal2/*.h") + Glob("dist/include/gfal2/common/*.h") + Glob("dist/include/gfal2/posix/*.h") )
	static_main = env_pack_main_devel.Install('/usr/'+libdir+'/', staticlib)
	example_main = env_pack_main_devel.Install('/usr/share/gfal2/example/', Glob("testing/example/*.c"))
	x_rpm_install = define_rpm_install(arguments_to_str());
	p_main_devel = env_pack_main_devel.Package( 
			 NAME     = 'gfal2-core-devel',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = 'Apache2.0',
			 SUMMARY        = 'development files for grid file access library 2.0',
			 DESCRIPTION    = 'development files for grid file access library 2.0',
			 X_RPM_GROUP    = 'CERN/grid',
			 X_RPM_INSTALL= x_rpm_install,
			 X_RPM_REQUIRES = 'glib2, gfal2-core',
			 source= [header_main,static_main, example_main] 
			 )
	return p_main_devel
	
def package_lfc():
	env_pack_main= env.Clone()
	lib_plugin_lfc = env_pack_main.Install('/usr/'+libdir+'/', plugin_lfc_lib)
	lib_plugin_lfc_conf= env_pack_main.Install('/etc/profile.d/', Glob("dist/scripts/gfal_plugin_lfc/*sh"))
	x_rpm_install = define_rpm_install(arguments_to_str());
	p_plugin_lfc = env_pack_main.Package( 
			 NAME     = 'gfal2-plugin-lfc',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = 'Apache2.0',
			 SUMMARY        = 'plugin lfc for gfal 2.0',
			 DESCRIPTION    = 'Provide the lfc access for gfal2.0',
			 X_RPM_GROUP    = 'CERN/grid',
			 X_RPM_REQUIRES = 'lfc-libs, glib2, gfal2-core ',
			 X_RPM_INSTALL= x_rpm_install,
			 source= [lib_plugin_lfc, lib_plugin_lfc_conf],
			 )	
	return p_plugin_lfc
	

def package_srm():
	env_pack_main= env.Clone()
	lib_plugin_srm = env_pack_main.Install('/usr/'+libdir+'/', plugin_srm_lib)
	lib_plugin_srm_conf= env_pack_main.Install('/etc/profile.d/', Glob("dist/scripts/gfal_plugin_srm/*sh"))
	x_rpm_install = define_rpm_install(arguments_to_str());
	p_plugin_srm = env_pack_main.Package( 
			 NAME     = 'gfal2-plugin-srm',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = 'Apache2.0',
			 SUMMARY        = 'plugin srm for gfal 2.0',
			 DESCRIPTION    = 'Provide the srm access for gfal2.0',
			 X_RPM_GROUP    = 'CERN/grid',
			 X_RPM_REQUIRES = 'glib2, gfal2-core, gfal-srm-ifce ',
			 X_RPM_AUTOREQPROV = "no",
			 X_RPM_INSTALL= x_rpm_install,
			 source= [lib_plugin_srm, lib_plugin_srm_conf],
			 )	
	return p_plugin_srm
	
def package_rfio():
	env_pack_main= env.Clone()
	lib_plugin_rfio = env_pack_main.Install('/usr/'+libdir+'/', plugin_rfio_lib)
	lib_plugin_rfio_conf= env_pack_main.Install('/etc/profile.d/', Glob("dist/scripts/gfal_plugin_rfio/*sh"))
	x_rpm_install = define_rpm_install(arguments_to_str());
	p_plugin_rfio = env_pack_main.Package( 
			 NAME     = 'gfal2-plugin-rfio',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = 'Apache2.0',
			 SUMMARY        = 'plugin rfio for gfal 2.0',
			 DESCRIPTION    = 'Provide the rfio access for gfal2.0',
			 X_RPM_GROUP    = 'CERN/grid',
			 X_RPM_REQUIRES = 'dpm-libs, glib2, gfal2-core ',
			 X_RPM_INSTALL= x_rpm_install,
			 source= [lib_plugin_rfio, lib_plugin_rfio_conf],
			 )	
	return p_plugin_rfio
	

def package_dcap():
	env_pack_main= env.Clone()
	lib_plugin_dcap = env_pack_main.Install('/usr/'+libdir+'/', plugin_dcap_lib)
	lib_plugin_dcap_conf= env_pack_main.Install('/etc/profile.d/', Glob("dist/scripts/gfal_plugin_dcap/*sh"))
	x_rpm_install = define_rpm_install(arguments_to_str());
	p_plugin_dcap = env_pack_main.Package( 
			 NAME     = 'gfal2-plugin-dcap',
			 VERSION        = version,
			 PACKAGEVERSION = package_version,
			 PACKAGETYPE    = 'rpm',
			 LICENSE        = 'Apache2.0',
			 SUMMARY        = 'plugin dcap for gfal 2.0',
			 DESCRIPTION    = 'Provide the dcap access for gfal2.0',
			 X_RPM_GROUP    = 'CERN/grid',
			 X_RPM_REQUIRES = 'dcap-libs, glib2, gfal2-core ',
			 X_RPM_INSTALL= x_rpm_install,
			 source= [lib_plugin_dcap, lib_plugin_dcap_conf],
			 )	
	return p_plugin_dcap	
	
	
def package_doc():
	env_doc = env.Clone()
	docs_main = env_doc.Install('/usr/share/doc/gfal2/', Glob('doc/build/html/*') )
	env_doc.Depends(docs_main, Glob('doc/build/html/*') )	
	p_doc = env_doc.Package( 
				 NAME     = 'gfal2-doc',
				 VERSION        = version,
				 PACKAGEVERSION = package_version,
				 PACKAGETYPE    = 'rpm',
				 LICENSE        = 'Apache2.0',
				 SUMMARY        = 'doc gfal2.0',
				 DESCRIPTION    = 'Documentation package for gfal2.X',
				 X_RPM_GROUP    = 'CERN/grid',
				 X_RPM_REQUIRES = 'gfal2-core ',
				 X_RPM_INSTALL= define_rpm_install(arguments_to_str()),		 
				 source= [docs_main] 
				 )	
	return p_doc

def package_meta():	
	env_all = env.Clone()
	license1 = env_all.Install('/usr/share/gfal2/', 'LICENSE' )	
	p_all = env_all.Package( 
				 NAME     = 'gfal2-all',
				 VERSION        = version,
				 PACKAGEVERSION = package_version,
				 PACKAGETYPE    = 'rpm',
				 LICENSE        = 'Apache2.0',
				 SUMMARY        = 'gfal2.0 meta package',
				 DESCRIPTION    = 'meta package for gfal2.X',
				 X_RPM_GROUP    = 'CERN/grid',
				 X_RPM_REQUIRES = 'gfal2-core, gfal2-doc, gfal2-plugin-lfc, gfal2-plugin-dcap, gfal2-plugin-rfio, gfal2-plugin-srm',
				 X_RPM_INSTALL= define_rpm_install(arguments_to_str()),
				 source= [license1] 
				 )	
	return p_all
	
	

if ARGUMENTS.get('package','0') !='0':
	str_func = "package_" + ARGUMENTS.get('package','0')
	print "package : "+ str_func
	p_res = globals()[str_func]()
	Default(p_res)	


	
	
	
	 	
	

	

