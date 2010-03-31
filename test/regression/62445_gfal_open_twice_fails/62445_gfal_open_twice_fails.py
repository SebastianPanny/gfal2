import os, gfal, sys, errno, unittest 

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

