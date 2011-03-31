#!/usr/bin/python
##
# @file SConstruct build file
# Simple SCons file in order to build ONLY the testing example files currently
# @author : Devresse Adrien
# @version : v0.0.1
# @date 21/03/2011

import os
import subprocess

# global var
etics_build_dir= os.environ.get('ETICS_WORKSPACE')
etics_lib_dir= etics_build_dir+'/repository/vdt/globus/4.0.7-VDT-1.10.1/sl5_x86_64_gcc412/lib/'
build_dir = 'build'

if( etics_build_dir == ''):
	print " Error env var ETICS_WORKSPACE not set", sys.stderr
	os.exit(-1)
	
print "ETICS WORKSPACE : " + etics_build_dir
print "ETICS compiled lib dir : " + etics_lib_dir

	
headers= ['.', '#src/', Dir(etics_build_dir+'/org.glite.data.srm-ifce/src/')]

libs=[ '#'+build_dir+'/src/.libs/' , etics_build_dir+ '/repository/vdt/globus/4.0.7-VDT-1.10.1/sl5_x86_64_gcc412/lib/' ]
env = Environment(CPPPATH= headers, LIBPATH=libs)
env['ENV']['LD_LIBRARY_PATH'] = etics_lib_dir


SConscript('testing/SConscript', ['env', 'headers'])
