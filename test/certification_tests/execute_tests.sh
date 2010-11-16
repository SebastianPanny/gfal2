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

source ../setup.sh

export LOGSLOCATION="$BUILD_ROOT/testlogs"
export GFAL_BASIC="yes"
export GFAL_PYTHON_API="yes"
export SE_HOST=$SE_HOST_1
export VO=$LCG_GFAL_VO

./GFAL-certtest 
