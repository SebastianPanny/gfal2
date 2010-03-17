#! /bin/bash

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
