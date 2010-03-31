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
GFAL_OPEN_TWICE_FAILS_TEST_DIR=$GLOBAL_SRM_TEST_DIR_DPM_1/62445_gfal_open_twice_fails/
# Your VO name
TEST_VO=$GLOBAL_TEST_VO
# The test file that will be copied to the server
LOCAL_TEST_FILE=file:///etc/group
# ------- DO NOT EDIT BELOW THIS ---------

export GFAL_OPEN_TWICE_FAILS_TEST_FILE=$GFAL_OPEN_TWICE_FAILS_TEST_DIR/test.dat

#lcg-cp --verbose --nobdii -D srmv2 --vo $TEST_VO $LOCAL_TEST_FILE $GFAL_OPEN_TWICE_FAILS_TEST_FILE

# Python script to test the original problem.
#python2.5 62445_gfal_open_twice_fails.py -v 

export PYTHONPATH=$PYTHONPATH/../../python2.4/site-packages
echo $PYTHONPATH
python2.4 62445_gfal_open_twice_fails.py -v 

# The same functionality than above, but from C, to see if the problem
# originates from the C library or from Python.
BUILD_DIR=../../../build
pushd $BUILD_DIR/test/regression/62445_gfal_open_twice_fails &> /dev/null
#./62445_gfal_open_twice_fails
popd &> /dev/null

#lcg-del -l --verbose --nobdii -D srmv2 --vo $TEST_VO $GFAL_OPEN_TWICE_FAILS_TEST_FILE
#lcg-del -l -d --verbose --nobdii -D srmv2 --vo $TEST_VO $GFAL_OPEN_TWICE_FAILS_TEST_DIR

unset GFAL_OPEN_TWICE_FAILS_TEST_FILE

