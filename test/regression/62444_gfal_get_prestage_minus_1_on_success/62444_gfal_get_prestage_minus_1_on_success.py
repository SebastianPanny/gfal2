# Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
#
# Tool to help reproducing Savannah bug #62444.
#
# Copyright (C) 2010 by CERN
# All rights reserved

import os, gfal, unittest

# IMPORTANT!
# Make sure that the endpoint returns SRM_REQUEST_QUEUED, otherwise the test fails!

class GetPrestageMinus1OnSuccess_62444_TestCase(unittest.TestCase):

    def setUp(self):
        self.testDir_ = os.environ["TEST_DIRECTORY"]
        gfal.gfal_mkdir(self.testDir_, 0755)
        self.testFile_ = self.testDir_ + "/62444.test.dat"
        fd = gfal.gfal_creat(self.testFile_, 0755)
        self.failIfEqual(-1, fd)
        gfal.gfal_close(fd)
        res = gfal.gfal_stat(self.testFile_)
        self.failUnlessEqual(0, res[0])
        self.failUnlessEqual(0, gfal.gfal_get_errno())
        self.request_ = {}
        self.request_['defaultsetype'] = 'srmv2'
        self.request_['surls'] = [self.testFile_]

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

