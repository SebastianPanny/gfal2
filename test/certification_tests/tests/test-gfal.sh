#!/bin/sh
#set -x
#=============================================================================
#
# Test suite for GFAL
# 
#=============================================================================

#=== FUNCTION ================================================================
# Prints the usage
#=============================================================================
function usage(){
    echo "Usage: $0 -v <voname> -l <LFC Host> -d <SE>"
}

#=== FUNCTION ================================================================
# Parse input arguments and set global vairables 
#=============================================================================
#function parse_arguments(){
    while getopts "v:l:d:" arg ; do
	case "$arg" in
	    v) vo="$OPTARG" ;;
	    l) lfc="$OPTARG" ;;
	    d) dest="$OPTARG" ;;
	    -) break ;;
	    ?) usage
	    exit 1 ;;
	esac
    done
    if [ "x${vo}" == "x"  -o "x${lfc}" == "x" -o "x${dest}" == "x" ]; then
	usage
	exit 1
    fi
    export VO=${vo}
    export LFC=${lfc}
    export DEST=${dest}
#}

#=== FUNCTION ================================================================
# test preperation
#=============================================================================
function prepare(){
    uni_pid=$$
    uni_time=`date +%s`
    name="$uni_pid$uni_time"
    
    export LFN="lfn:/grid/${VO}/$name"
    export LOCAL_FILE=/etc/redhat-release
    export NAME=${name}
}
#=== FUNCTION ================================================================
# Checks for a valid proxy.
#=============================================================================
function check_proxy(){
    $(voms-proxy-info >/dev/null 2>&1)
    if [ $? -gt 0 ]; then
	echo "Error: Couldn't find a valid proxy."
	exit 1
    fi
}

#=== FUNCTION ================================================================
# Runs Test Command
#=============================================================================
function run_command() {
    echo "running command $1"
    command=$1
    message=$2
    echo -n "${message} ... "
    output=$(${command} 2>&1)
    if [ $? -gt 0 ]; then
	echo "Failed"
	echo "${command}" >&2
	echo "${output}" >&2
	export RETVAL=1
	return 1
    fi
    echo "OK"
    export OUTPUT=${output}
    return 0
}
    
#=============================================================================
# Start Test
#=============================================================================

prepare
check_proxy

export RETVAL=0
export LFC_HOST=${LFC}

echo "Testing gfal for VO ${VO} ... "

BUILD_DIR=../../../build

#Create a file, open, write, close, re-open, read close
command="$BUILD_DIR/test/certification_tests/tests/gfal-test file:///tmp/${NAME}"
message="Running GFAL test script"
run_command "${command}" "${message}"


#command="gfal_testcreatdir lfn:/grid/${VO}/tmp/${NAME}"
command="$BUILD_DIR/src/gfal_testcreatdir lfn:/grid/${VO}/sa3tmp-`date +%s`"
message="Running GFAL create dir test"
run_command "${command}" "${message}"

command="lcg-cr --vo ${VO} -d ${DEST} -l ${LFN} file://${LOCAL_FILE}"
message="Copying file to Storage Element using lcg-cr"
run_command "${command}" "${message}"

GUID=${OUTPUT}

command="lcg-lr -v --vo ${VO} ${LFN}"
message="Getting SURL using lcg-lr"
run_command "${command}" "${message}"

SURL=$(echo ${OUTPUT} | grep -i -E -o 'sfn.*|srm.*')

#command="lcg-gt -v $SURL rfio"
command="lcg-gt -v $SURL gsiftp"
message="Getting TURL using lcg-gt"
run_command "${command}" "${message}"

TURL=$(echo ${OUTPUT})

#What is the point of this? It gives a local file.
command="$BUILD_DIR/src/gfal_testrw file:///tmp/${NAME}"
message="Running GFAL read write test (local file)"
run_command "${command}" "${message}"

sleep 20

command="$BUILD_DIR/src/gfal_testread ${LFN}"
message="Running GFAL read test"
run_command "${command}" "${message}"

command="$BUILD_DIR/src/gfal_testget ${SURL}"
message="Running GFAL get test"
run_command "${command}" "${message}"

command="$BUILD_DIR/src/gfal_testchmod ${LFN} 666"
message="Running GFAL chmod test"
run_command "${command}" "${message}"

command="$BUILD_DIR/src/gfal_teststat ${LFN}"
message="Running GFAL stat test"
run_command "${command}" "${message}"

#Unlink a replica providing a SURL
command="$BUILD_DIR/src/gfal_testunlink ${SURL}"
message="Running GFAL unlink test"
run_command "${command}" "${message}"

if [ ${RETVAL} -gt 0 ]; then
    echo "-TEST FAILED-"
else
    echo "-TEST PASSED-"
fi

exit ${RETVAL}
