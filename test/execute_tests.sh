#! /bin/bash
# Execute the full test suite (unit and regression tests). 

function execute_test {
    local testdir=$1
    
    echo -e "\nExecuting test suite \"$testdir\"\n"
    pushd $testdir &> /dev/null
    ./execute_tests.sh
    local res=$? 
    popd &> /dev/null
    
    if [ $res != 0 ] ; then
        echo 
        echo -e "\nA test in \"$testdir\" failed.\n"
        exit 1
    fi
    echo -e "\nAll tests in \"$testdir\" passed.\n"   
}

execute_test unit regression
