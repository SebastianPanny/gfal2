#! /bin/bash

# Global settings for all the tests. 

# ++++++++++++ Edit this ++++++++++++
# Workspace root (default works under e-env). All the sources are checked in and
# built here.
WS=$HOME

# The full name of a directory according to the SRM protocol. The tests files
# will be copied.
export GLOBAL_SRM_TEST_DIR_DPM_1=srm://lxb7608v1.cern.ch:8446/srm/managerv2?SFN=/dpm/cern.ch/home/dteam

# For the meaning of these global variables, see the LFC documentations.
export LFC_HOST=lxb7608v3.cern.ch
export LCG_GFAL_INFOSYS=lxbra2306.cern.ch:2170 

# Your VO name.
export GLOBAL_TEST_VO=dteam
# ++++++++++++ Do not edit after this. ++++++++++++
