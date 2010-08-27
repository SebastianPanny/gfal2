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

function usage 
{
    echo
    echo "Usage:"
    echo
    echo "Prerequistes:"
    echo 
    echo " - BUILD_ROOT must be set to to the root of the build/checkout of the lcgutil project. Example:"
    echo
    echo " export BUILD_ROOT=/home/$USER/workspace"
    echo
}  

function checkVariable
{
    local VAR_NAME=$1
    local VAR_DEFAULT=$2
    local VAR_VALUE=$3

    # color codes, color howto: http://webhome.csc.uvic.ca/~sae/seng265/fall04/tips/s265s047-tips/bash-using-colors.html

    if [ ! -n "$VAR_VALUE" ] ;then
        echo
        echo -e "\e[1;31mERROR: $VAR_NAME is not set.\e[0m For example:"
        echo
        echo "export $VAR_NAME=$VAR_DEFAULT"
        echo
        usage
        exit 1
    fi
}

checkVariable BUILD_ROOT "/home/user/workspace" $BUILD_ROOT
source $BUILD_ROOT/org.glite.data/bin/test-setup-gfal.sh
export GLOBAL_SRM_TEST_DIR_DPM=srm://$SE_ENDPOINT_DPM:8446/srm/managerv2?SFN=/dpm/cern.ch/home/dteam

