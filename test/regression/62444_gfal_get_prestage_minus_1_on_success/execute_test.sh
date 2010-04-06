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
#   62444: [GFAL-Python] gfal_get and gfal_prestage return (-1) on success
#
# https://savannah.cern.ch/bugs/?62444:
#

source ../setup.sh

# ------- EDIT THIS to configure ---------
export TEST_DIRECTORY=$GLOBAL_SRM_TEST_DIR_DPM_1/62444_gfal_get_prestage_minus_1_on_success
# ------- DO NOT EDIT BELOW THIS ---------

# Python script to test the original problem.
python2.5 62444_gfal_get_prestage_minus_1_on_success.py -v

unset TEST_DIRECTORY
