import os, gfal, sys, errno, unittest 

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

# TEST_DIRECTORY environmant variable must be set: this is a SURL, pointing
# to a test directory to be created. In GFAL regression tests, 
# execute_test.sh calling this Python script sets it.
class GfalOpenTwiceFails_62445_TestCase(unittest.TestCase):

    def testDoubleOpen(self):
        testFile =  os.environ["GFAL_OPEN_TWICE_FAILS_TEST_FILE"]
        fd = gfal.gfal_open(testFile, os.O_RDONLY, 0644)
        self.failIfEqual(-1, fd)
        gfal.gfal_close(fd)
        fd = -2
        fd = gfal.gfal_open(testFile, os.O_RDONLY, 0644)
        self.failIfEqual(-1, fd)
        gfal.gfal_close(fd)

if __name__ == "__main__":
    unittest.main()

