#include "CuTest.h"

#define defTest(_name) extern void Test##_name(CuTest*)

defTest(TSURL);
defTest(ArrayOfTSURL);
defTest(TSURLInfo);
defTest(ArrayOfTSURLInfo);
defTest(TDirOption);
defTest(TGetFileRequest);
defTest(ArrayOfTGetFileRequest);
defTest(TPutFileRequest);
defTest(ArrayOfTPutFileRequest);
defTest(srmPrepareToPutRequest);
defTest(srmPrepareToPutResponse);
defTest(srmPrepareToGetRequest);
defTest(srmPrepareToGetResponse);
defTest(TReturnStatus);
defTest(TPutRequestFileStatus);
defTest(ArrayOfTPutRequestFileStatus);
defTest(srmPutDoneRequest);
defTest(srmPutDoneResponse);
defTest(srmAbortFilesRequest);
defTest(srmAbortFilesResponse);
defTest(srmStatusOfGetRequestRequest);
defTest(srmStatusOfGetRequestResponse);
defTest(srmStatusOfPutRequestRequest);
defTest(srmStatusOfPutRequestResponse);
defTest(TSURLReturnStatus);
defTest(ArrayOfTSURLReturnStatus);
defTest(TGetRequestFileStatus);
defTest(ArrayOfTSURLReturnStatus);
defTest(ArrayOfTGetRequestFileStatus);
defTest(TMetaDataPathDetail);
defTest(srmLsRequest);
defTest(srmLsResponse);
defTest(srmCheckPermissionRequest);
defTest(srmCheckPermissionResponse);

void RunAllTests(void) 
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();


    SUITE_ADD_TEST(suite, TestTSURL);
    SUITE_ADD_TEST(suite, TestArrayOfTSURL);
    SUITE_ADD_TEST(suite, TestTSURLInfo);
    SUITE_ADD_TEST(suite, TestArrayOfTSURLInfo);
    SUITE_ADD_TEST(suite, TestTDirOption);
    SUITE_ADD_TEST(suite, TestTGetFileRequest);
    SUITE_ADD_TEST(suite, TestArrayOfTGetFileRequest);
    SUITE_ADD_TEST(suite, TestTPutFileRequest);
    SUITE_ADD_TEST(suite, TestArrayOfTPutFileRequest);
    SUITE_ADD_TEST(suite, TestsrmPrepareToPutRequest);
    SUITE_ADD_TEST(suite, TestsrmPrepareToPutResponse);
    SUITE_ADD_TEST(suite, TestsrmPrepareToGetRequest);
    SUITE_ADD_TEST(suite, TestsrmPrepareToGetResponse);
    SUITE_ADD_TEST(suite, TestTReturnStatus);
    SUITE_ADD_TEST(suite, TestTPutRequestFileStatus);
    SUITE_ADD_TEST(suite, TestArrayOfTPutRequestFileStatus);
    SUITE_ADD_TEST(suite, TestsrmPutDoneRequest);
    SUITE_ADD_TEST(suite, TestsrmPutDoneResponse);
    SUITE_ADD_TEST(suite, TestsrmAbortFilesRequest);
    SUITE_ADD_TEST(suite, TestsrmAbortFilesResponse);
    SUITE_ADD_TEST(suite, TestsrmStatusOfGetRequestRequest);
    SUITE_ADD_TEST(suite, TestsrmStatusOfGetRequestResponse);
    SUITE_ADD_TEST(suite, TestsrmStatusOfPutRequestRequest);
    SUITE_ADD_TEST(suite, TestsrmStatusOfPutRequestResponse);
    SUITE_ADD_TEST(suite, TestTSURLReturnStatus);
    SUITE_ADD_TEST(suite, TestArrayOfTSURLReturnStatus);
    SUITE_ADD_TEST(suite, TestTGetRequestFileStatus);
    SUITE_ADD_TEST(suite, TestArrayOfTSURLReturnStatus);
    SUITE_ADD_TEST(suite, TestArrayOfTGetRequestFileStatus);
    SUITE_ADD_TEST(suite, TestTMetaDataPathDetail);
	SUITE_ADD_TEST(suite, TestsrmLsRequest);
	SUITE_ADD_TEST(suite, TestsrmLsResponse);
	SUITE_ADD_TEST(suite, TestsrmCheckPermissionRequest);
	SUITE_ADD_TEST(suite, TestsrmCheckPermissionResponse);
	
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(void)
{
    RunAllTests();
}

