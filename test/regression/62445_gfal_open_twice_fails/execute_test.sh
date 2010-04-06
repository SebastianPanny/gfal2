#! /bin/bash
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://www.eu-egee.org/partners/ for details on the copyright holders.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
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

