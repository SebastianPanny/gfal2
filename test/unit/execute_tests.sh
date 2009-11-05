#! /bin/bash

BUILD_DIR=../../build

pushd $BUILD_DIR/test/unit &> /dev/null
./gfal-unittest
res=$?
popd &> /dev/null

exit $res
