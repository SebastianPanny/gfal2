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
#   62445: [GFAL-Python] gfal_open fails when called twice
#
# https://savannah.cern.ch/bugs/?62445
#

source ../../setup.sh

# ------- EDIT THIS to configure ---------
GFAL_OPEN_TWICE_FAILS_TEST_DIR=$GLOBAL_SRM_TEST_DIR_DPM/62445_gfal_open_twice_fails/
# The test file that will be copied to the server
LOCAL_TEST_FILE=file:///etc/group
# ------- DO NOT EDIT BELOW THIS ---------

export CSEC_TRACEFILE=$PWD/csec.trace
export CSEC_TRACE=1
export CSEC_NOTHREAD=1

export GFAL_OPEN_TWICE_FAILS_TEST_FILE=$GFAL_OPEN_TWICE_FAILS_TEST_DIR/test.dat

lcg-cp --verbose --nobdii -D srmv2 --vo $LCG_GFAL_VO $LOCAL_TEST_FILE $GFAL_OPEN_TWICE_FAILS_TEST_FILE
python 62445_gfal_open_twice_fails.py -v 

# The same functionality than above, but from C, to see if the problem
# originates from the C library or from Python.
BUILD_DIR=../../../build
pushd $BUILD_DIR/test/regression/62445_gfal_open_twice_fails &> /dev/null
./62445_gfal_open_twice_fails
popd &> /dev/null
lcg-del -l --verbose --nobdii -D srmv2 --vo $LCG_GFAL_VO $GFAL_OPEN_TWICE_FAILS_TEST_FILE
lcg-del -l -d --verbose --nobdii -D srmv2 --vo $LCG_GFAL_VO $GFAL_OPEN_TWICE_FAILS_TEST_DIR

unset GFAL_OPEN_TWICE_FAILS_TEST_FILE

