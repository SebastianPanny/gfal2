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

import os, gfal, sys, errno, unittest 

# TEST_DIRECTORY environmant variable must be set: this is a SURL, pointing
# to a test directory to be created. In GFAL regression tests, 
# execute_test.sh calling this Python script sets it.

# Test strategy: create directories with different slash configurations (more slashes between
# direcotry elements, more trailing slashes, etc.), and check if directory was really created.
# This check is based on "normalized" directory names (no trailing slashes, etc.).
class MkdirDpmSe_52502_TestCase(unittest.TestCase):

    def setUp(self):
        # Setting up the SRM request
        self.dirToCreate_ = os.environ["TEST_DIRECTORY"]
        # Check the condition of "last caracter cannot be a '/'"
        self.failIfEqual("/", self.dirToCreate_[-1])
        self.dirOneMoreLevel_ = self.dirToCreate_ + "/oneMoreLevel"
        # Delete the directory, if it is there
        self.deleteDirs_()
        # The test directory must NOT exist
        res = gfal.gfal_stat(self.dirToCreate_)
        self.failUnlessEqual(-1, res[0])
        self.failUnlessEqual(errno.ENOENT, gfal.gfal_get_errno())

    def testOneDirectoryLevelNoTrailingSlash(self):
        self.createAndTest_(self.dirToCreate_)

    def testOneDirectoryLevelWithTrailingSlashes(self):
        self.createAndTest_(self.dirToCreate_, self.dirToCreate_ + "/")
        self.createAndTest_(self.dirToCreate_, self.dirToCreate_ + "//")
        self.createAndTest_(self.dirToCreate_, self.dirToCreate_ + "///")

    def testTwoDirectoryLevelOneSlash(self):
        self.createAndTest_(self.dirOneMoreLevel_)

    def testTwoDirectoryLevelsWithSlashes(self):
        self.createAndTest_(self.dirOneMoreLevel_, self.dirToCreate_ + "/oneMoreLevel")
        self.createAndTest_(self.dirOneMoreLevel_, self.dirToCreate_ + "/oneMoreLevel/")
        self.createAndTest_(self.dirOneMoreLevel_, self.dirToCreate_ + "//oneMoreLevel")
        self.createAndTest_(self.dirOneMoreLevel_, self.dirToCreate_ + "///oneMoreLevel")
        self.createAndTest_(self.dirOneMoreLevel_, self.dirToCreate_ + "//oneMoreLevel/")

    # DO the job.
    # 
    #   realdir: the real directory name (only one slash between directories, no trailing slash
    #   testdir: directory names with additional slashes
    def createAndTest_(self, realdir, testdir = None):
        if testdir is None:
            testdir = realdir 
        # Create the test directory
        res = gfal.gfal_mkdir(testdir, 0755)  
        self.failUnlessEqual(0, gfal.gfal_get_errno())
        # The directory must exist, otherwise the test failed.
        res = gfal.gfal_stat(realdir)
        self.failUnlessEqual(0, res[0])
        self.failUnlessEqual(0, gfal.gfal_get_errno())
        self.deleteDirs_()

    def deleteDirs_(self):
        res = gfal.gfal_rmdir(self.dirOneMoreLevel_)
        res = gfal.gfal_rmdir(self.dirToCreate_)  

if __name__ == "__main__":
    unittest.main()

