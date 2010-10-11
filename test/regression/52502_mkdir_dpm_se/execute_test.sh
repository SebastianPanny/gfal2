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

source ../../setup.sh

# ------- EDIT THIS to configure ---------
# Make sure that the last character is not '/'! Because, this caused the bug...
export TEST_DIRECTORY=$GLOBAL_SRM_TEST_DIR_DPM/52502_mkdir_dpm_se
# ------- DO NOT EDIT BELOW THIS ---------

echo $TEST_DIRECTORY

# Python script to test the original problem.
python 52502_mkdir_dpm_se.py -v 

# The same functionality than above, but from C, to see if the problem
# originates from the C library or from Python.
BUILD_DIR=../../../build
pushd $BUILD_DIR/test/regression/52502_mkdir_dpm_se &> /dev/null
./52502_mkdir_dpm_se

popd &> /dev/null

unset TEST_DIRECTORY
