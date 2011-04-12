#!/usr/bin/python
##
# @file SConstruct build file
# Simple SCons file in order to build ONLY the testing  files currently
# @author : Devresse Adrien
# @version : v0.0.3
# @date 24/03/2011

import os
import subprocess
import glob

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
	raise Exception("Error env var ETICS_WORKSPACE not set")

# global var
etics_build_dir=get_etics_dir()
version= '0.1_alpha'
ccheck_location = etics_build_dir+ "/repository/externals/c-check/0.9.8/sl5_x86_64_gcc412"
glib_location = etics_build_dir+ "/repository/externals/glib2-devel/2.12.3/sl5_x86_64_gcc412"
voms_location = etics_build_dir+ "/stage/lib64"
srm_ifce_location= etics_build_dir+ "/stage/lib64"
dcap_location = etics_build_dir+ "/repository/externals/dcache-dcap/1.8.0/sl5_x86_64_gcc412/dcap"
dpm_location = etics_build_dir + "/stage"
link_libs= ['m','uuid','c','dl','glib-2.0']

build_dir = 'build'

#related var for headers:
header_folder='/stage/include/'
ccheck_header= ccheck_location+ "/include"
build_dir_src = build_dir +'/src'
build_dir_test= build_dir +'/test/src'
ccheck_header= ccheck_location+ "/include"
glib_header_dir = [ glib_location + '/lib64/glib-2.0/include/', glib_location+ '/include/glib-2.0/' ]
dpm_header= dpm_location+ "/usr/include/dpm"
dcap_header= dcap_location + "/include"

# related var for static library
ccheck_lib= ccheck_location+ "/lib64/libcheck.a"

		
etics_lib_dir= glob.glob(etics_build_dir+'/repository/vdt/globus/*/*/lib/')[0]
etics_header_dir = [etics_build_dir+header_folder] + glob.glob(etics_build_dir+header_folder+'/*/') + glob.glob(etics_build_dir+header_folder+'/*/*/') + glob.glob(etics_build_dir+header_folder+'/*/*/*/')

print "ETICS WORKSPACE : " + etics_build_dir
print "ETICS compiled lib dir : " + etics_lib_dir

headers= ['.', '#.', '#build/src/',etics_header_dir, glib_header_dir, srm_ifce_location, dpm_header, dcap_header]
libs=[ '#'+build_dir+'/libs' , etics_lib_dir, voms_location ]
cflags=['-DVERSION=\\\"'+version+'\\\"', '-DGFAL_SECURE' , '-D_LARGEFILE64_SOURCE','-DGFAL_ENABLE_RFIO','-DGFAL_ENABLE_DCAP' ] # largefile flag needed in 64 bits mod, Version setter, Warning flags and other legacy flags 
env = Environment(CPPPATH= headers, LIBPATH=libs, CFLAGS=cflags, LIBS=link_libs)
env['ENV']['LD_LIBRARY_PATH'] = os.getenv('LD_LIBRARY_PATH')

# debug mode
if ARGUMENTS.get('debug','0') =='yes':
	print "DEBUG MODE"
	env.Append(CFLAGS='-g')

#main build
VariantDir(build_dir_src, 'src')
SConscript(build_dir_src +'/SConscript',['env', 'headers', 'libs', 'build_dir_src'])

# global testing build
SConscript('testing/SConscript', ['env', 'headers', 'libs'])

#unit tests
env_test = env.Clone()
env_test.Append(CPPPATH=[ccheck_header, "#src/common", "#src/"])
VariantDir(build_dir_test, 'test')
SConscript(build_dir_test +'/SConscript',['env_test', 'headers', 'libs', 'build_dir_src','ccheck_header','ccheck_lib'])
	

	
	

