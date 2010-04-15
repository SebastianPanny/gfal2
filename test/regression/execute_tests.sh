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

# These tests are independent. You can comment out any of them if you want to execute a 
# selected set only.

# Execute a test. The function assumes the following:
#
# 1. All the tests are in subdirectories of "tests"
# 2. There is a single entry point of each tests, it is a script calles "execute_test.sh"
#
# For example, for Savannah bug 52502, the relative path to the script is:
#
#   tests/52502_mkdir_dpm_se/execute_tests.sh
#
# The function requires one argument, the subdirectory name (52502_mkdir_dpm_se). 
source setup.sh

function execute_a_test
{
    local TEST_NAME="$1"
    echo "Executing test $TEST_NAME..."
    pushd $TEST_NAME > /dev/null
    ./execute_test.sh
    popd > /dev/null
}

execute_a_test 52502_mkdir_dpm_se
execute_a_test 62444_gfal_get_prestage_minus_1_on_success

# We take it out from a release for the moment, as it is not an user use case.
# Will be fixed later.
#execute_a_test 62445_gfal_open_twice_fails

