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
gsoap_location= etics_build_dir+ "/repository/externals/gsoap/2.7.6b/sl5_x86_64_gcc412"
glib_location = etics_build_dir+ "/repository/externals/glib2-devel/2.12.3/sl5_x86_64_gcc412"
voms_location = etics_build_dir+ "/stage/lib64"
link_libs= ['m','uuid','c','dl']


#special var :
build_dir = 'build'
header_folder='/stage/include/'
build_dir_src = build_dir +'/src'

# auto-defined var :
gsoap_header_dir= gsoap_location+"/include"
glib_header_dir = [ glib_location + '/lib64/glib-2.0/include/', glib_location+ '/include/glib-2.0/' ]


		
etics_lib_dir= glob.glob(etics_build_dir+'/repository/vdt/globus/*/*/lib/')[0]
etics_header_dir = [etics_build_dir+header_folder] + glob.glob(etics_build_dir+header_folder+'/*/') + glob.glob(etics_build_dir+header_folder+'/*/*/') + glob.glob(etics_build_dir+header_folder+'/*/*/*/')



print "ETICS WORKSPACE : " + etics_build_dir
print "ETICS compiled lib dir : " + etics_lib_dir

	
headers= ['.', '#.', '#build/src/',etics_header_dir, glib_header_dir, gsoap_header_dir]
libs=[ '#'+build_dir+'/libs' , etics_lib_dir, voms_location ]
cflags=" -DVERSION=\\\""+version+"\\\""
print cflags
env = Environment(CPPPATH= headers, LIBPATH=libs, CFLAGS=cflags, LIBS=link_libs)
env['ENV']['LD_LIBRARY_PATH'] = etics_lib_dir



SConscript('testing/SConscript', ['env', 'headers', 'libs'])
VariantDir(build_dir_src, 'src')
SConscript(build_dir_src +'/SConscript',['env', 'headers', 'libs', 'gsoap_location', 'build_dir_src'])


	

	
	

