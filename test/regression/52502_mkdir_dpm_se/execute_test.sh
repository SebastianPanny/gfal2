#! /bin/bash
#
# Tests for Savannah bug 
#
#   52502: gfal_mkdir and DPM SE
#
# https://savannah.cern.ch/bugs/?52502
#

source ../setup.sh

# ------- EDIT THIS to configure ---------
# Make sure that the last character is not '/'! Because, this caused the bug...
export TEST_DIRECTORY=$GLOBAL_SRM_TEST_DIR_DPM_1/52502_mkdir_dpm_se
echo $TEST_DIRECTORY
# ------- DO NOT EDIT BELOW THIS ---------

# Python script to test the original problem.
python2.5 52502_mkdir_dpm_se.py -v 

# The same functionality than above, but from C, to see if the problem
# originates from the C library or from Python.
BUILD_DIR=../../../build
pushd $BUILD_DIR/test/regression/52502_mkdir_dpm_se &> /dev/null
./52502_mkdir_dpm_se
popd &> /dev/null

unset TEST_DIRECTORY
