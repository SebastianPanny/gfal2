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

# global var
etics_build_dir=''
build_dir = 'build'
header_folder='/stage/include/'
version= '0.1_alpha'

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
	

	

etics_build_dir = get_etics_dir()	
etics_lib_dir= glob.glob(etics_build_dir+'/repository/vdt/globus/*/*/lib/')[0]
etics_header_dir = [etics_build_dir+header_folder] + glob.glob(etics_build_dir+header_folder+'/*/') + glob.glob(etics_build_dir+header_folder+'/*/*/') + glob.glob(etics_build_dir+header_folder+'/*/*/*/')
glib_header_dir = [ etics_build_dir + '/repository/externals/glib2-devel/2.12.3/sl5_x86_64_gcc412/lib64/glib-2.0/include/', etics_build_dir +'/repository/externals/glib2-devel/2.12.3/sl5_x86_64_gcc412/include/glib-2.0/' ]


print "ETICS WORKSPACE : " + etics_build_dir
print "ETICS compiled lib dir : " + etics_lib_dir

	
headers= ['.', '#src/', etics_header_dir, glib_header_dir]
libs=[ '#'+build_dir+'/src/.libs/' , etics_lib_dir ]
cflags=" -DVERSION=\\\""+version+"\\\""
print cflags
env = Environment(CPPPATH= headers, LIBPATH=libs, CFLAGS=cflags)
env['ENV']['LD_LIBRARY_PATH'] = etics_lib_dir



SConscript('testing/SConscript', ['env', 'headers', 'libs'])
SConscript('src/SConscript',['env', 'headers', 'libs'])


	

	
	

