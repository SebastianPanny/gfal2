# Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
#
# Tool to help reproducing Savannah bug #62444.
#
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

import os, gfal, unittest

# IMPORTANT!
# Make sure that the endpoint returns SRM_REQUEST_QUEUED, otherwise the test fails!

class GetPrestageMinus1OnSuccess_62444_TestCase(unittest.TestCase):

    def setUp(self):
        self.testDir_ = os.environ["TEST_DIRECTORY"]
        self.testFile_ = self.testDir_ + "/62444.test.dat"
        self.request_ = {}
        self.request_['defaultsetype'] = 'srmv2'
        self.request_['surls'] = [self.testFile_]
        self.tearDown()
        gfal.gfal_mkdir(self.testDir_, 0755)
        fd = gfal.gfal_creat(self.testFile_, 0755)
        self.failIfEqual(-1, fd)
        gfal.gfal_close(fd)
        res = gfal.gfal_stat(self.testFile_)
        self.failUnlessEqual(0, res[0])
        self.failUnlessEqual(0, gfal.gfal_get_errno())

    def tearDown(self):
        errCode,gfalObject,errMessage = gfal.gfal_init(self.request_)
        gfal.gfal_deletesurls(gfalObject)
        gfal.gfal_rmdir(self.testDir_)

    def testGet(self):
        errCode,gfalObject,errMessage = gfal.gfal_init(self.request_)
        errCode,gfalObject,errMessage = gfal.gfal_get(gfalObject)
        self.failUnlessEqual(0, errCode)
        errCode,gfalObject,fileStatuses = gfal.gfal_get_results(gfalObject)
        self.failIf("SRM_REQUEST_QUEUED" not in fileStatuses[0]['explanation'])

    def testPrestage(self):
        errCode,gfalObject,errMessage = gfal.gfal_init(self.request_)
        errCode,gfalObject,errMessage = gfal.gfal_prestage(gfalObject)
        self.failUnlessEqual(0, errCode)
        errCode,gfalObject,fileStatuses = gfal.gfal_get_results(gfalObject)
        self.failIf("SRM_REQUEST_QUEUED" not in fileStatuses[0]['explanation'])

if __name__ == "__main__":
    unittest.main()

