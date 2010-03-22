#! /bin/bash

BUILD_DIR=../../build

pushd $BUILD_DIR/src &> /dev/null
./gfal_unittest
res=$?
popd &> /dev/null

exit $res
