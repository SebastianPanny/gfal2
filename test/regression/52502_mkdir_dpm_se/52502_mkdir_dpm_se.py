import os, gfal, sys, errno, unittest 

# TEST_DIRECTORY environmant variable must be set: this is a SURL, pointing
# to a test directory to be created. In GFAL regression tests, 
# execute_test.sh calling this Python script sets it.

class MkdirDpmSe_52502_TestCase(unittest.TestCase):

    def setUp(self):
        # Setting up the SRM request
        self.dirToCreate_ = os.environ["TEST_DIRECTORY"]
        gfalDict = {}
        gfalDict['defaultsetype'] = 'srmv2'
        gfalDict['surls'] = [self.dirToCreate_]
        errCode,self.request_,errMessage = gfal.gfal_init(gfalDict)
        # Delete the directory, if it is there
        gfal.gfal_rmdir(self.dirToCreate_) 
        # The thest directory must NOT exist
        res,self.request_,errmsg = gfal.gfal_ls(self.request_)
        self.failUnlessEqual(0, res)
        numberOfResults,self.request_,listOfResults = gfal.gfal_get_results(self.request_) 
        self.failIfEqual(0, listOfResults[0]["status"], listOfResults[0]["explanation"])



    def tearDown(self):
        gfal.gfal_internal_free(self.request_)

    def testTheBug(self):
        # Create the test directory 
        res = gfal.gfal_mkdir(self.dirToCreate_, 0755) 
        errNo = gfal.gfal_get_errno() 
        self.failUnlessEqual(0, res)
        # The directory must exist, otherwise the test failed.
        res,self.request_,errmsg = gfal.gfal_ls(self.request_)
        self.failUnlessEqual(0, res)
        numberOfResults,self.request_,listOfResults = gfal.gfal_get_results(self.request_) 
        self.failUnlessEqual(0, listOfResults[0]["status"], listOfResults[0]["explanation"])

if __name__ == "__main__":
    unittest.main()

