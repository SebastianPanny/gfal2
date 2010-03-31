#! /bin/bash
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
