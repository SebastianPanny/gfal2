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

# Global settings for all the tests. 

# ++++++++++++ Edit this ++++++++++++
# Workspace root (default works under e-env). All the sources are checked in and
# built here.
WS=$HOME

# The full name of a directory according to the SRM protocol. The tests files
# will be copied.
export GLOBAL_SRM_TEST_DIR_DPM_1=srm://lxbra1910.cern.ch:8446/srm/managerv2?SFN=/dpm/cern.ch/home/dteam

# For the meaning of these global variables, see the LFC documentations.
export LFC_HOST=lxb7608v3.cern.ch
export LCG_GFAL_INFOSYS=lxbra2306.cern.ch:2170 

# Your VO name.
export GLOBAL_TEST_VO=dteam
# ++++++++++++ Do not edit after this. ++++++++++++
