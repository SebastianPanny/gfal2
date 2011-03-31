#!/bin/bash
##
# Simple compile script to compile without ETICS ( issued from ETICS logs ) 
# @author : Devresse Adrien
# @version : v0.03
# @date : 14/03/2011



init_dir=`pwd`

if [[ $ETICS_WORKSPACE == '' ]]; then
	echo -e " ********************************************************** " 1>&2
	echo -e " ERROR : This script need the location of the ETICS workspace " 1>&2
	echo -e " Please set the ETICS_WORKSPACE en var to a correct value \n" 1>&2
	echo -e " Example : export ETICS_WORKSPACE=/home/name/workspace \n " 1>&2
	exit -1
fi

function init {
	# init
	echo "Start Init......"
	cd $init_dir
	mkdir -p src/autogen build
	aclocal -I $ETICS_WORKSPACE/stage/share/build/m4
	libtoolize --force
	autoheader
	automake --foreign --add-missing --copy
	autoconf
	echo " Init Ended"
	cd $init_dir
}


function configure {
	cd $init_dir
	echo " Start configure......"
	cd build
	$init_dir/configure --enable-debug --enable-debug --enable-wall  --prefix=$ETICS_WORKSPACE/org.glite.data.gfal/etics-tmp --with-version=0.0.0 --with-release=0 --with-globus-prefix=$ETICS_WORKSPACE/repository/vdt/globus/4.0.7-VDT-1.10.1/sl5_x86_64_gcc412 --with-globus-thr-flavor=gcc64dbgpthr --with-globus-nothr-flavor=gcc64dbg --with-gsoap-location=$ETICS_WORKSPACE/repository/externals/gsoap/2.7.6b/sl5_x86_64_gcc412 --with-gsoap-version=2.7.6b --with-cgsi-gsoap-location=$ETICS_WORKSPACE/stage --with-swig-prefix=$ETICS_WORKSPACE/repository/externals/swig/1.3.29/sl5_x86_64_gcc412 --with-voms-location=$ETICS_WORKSPACE/stage --with-srm-ifce-location=$ETICS_WORKSPACE/stage --with-dcap-location=$ETICS_WORKSPACE/repository/externals/dcache-dcap/1.8.0/sl5_x86_64_gcc412/dcap --with-lfc-location=$ETICS_WORKSPACE/stage --with-dpm-location=$ETICS_WORKSPACE/stage
	echo "Configure Ended"
	##
}

function compile {
	cd $init_dir
	echo "Start Compilation......"
	make -C build all
	echo "Compilation Ended"
}

function test {
	cd $init_dir
	cd test
	./execute_tests.sh
}
	

function all {
	init
	configure
	compile
	test
}

function clean {
	cd $init_dir
	make -C build clean	
	
	
}

if [[ $1 == '' ]]; then
	all
else
	$1
fi

