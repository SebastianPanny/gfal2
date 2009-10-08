/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (C) 2009 by CERN
 * All rights reserved
 *
 * Test if the logic recognizes unknown protocols of wrongly formatted
 * protocol lists in the transfetParameters property of PrepareToGet.
 */
#include "srm_dependencies.h"
#include "srm2_2_ifce.h"
#include <assert.h>

#include "gfal_unit_testsuite.h"

#define __CALL_TEST(test) \
    printf("\nTest group: %s\n", #test); \
    res = (test); \
    if (!res) \
        return res;

/* The new PrepareToGet caller. This one produces the different error
 * scenarios. */
typedef enum _protocol_list_scenario_type {
    E_UNSUPPORTED_PROTOCOL,
    E_EMPTY_LIST,
    E_UNUSED
} _protocol_list_scenario_type_t;

/* The variable controls the actual scenario. The test function sets it,
 * then calls the function to be tested. Deep inside, the function will call
 * the (replaced) SOAP caller. The mock SOAP caller prepares different SRM
 * responses and return it, without the physical gSOAP/network operation.
 * The response content is controlled by this variable (the scenario). */
static _protocol_list_scenario_type_t _protocol_list_scenario = E_UNUSED;

/* A fixture with a protocol list containing an unsupported protocol name */
static char* _unsupported_protocol[] = {"unsupported_protocol", NULL};

/* Signature for srmv2_turlsfromsurls_get, srmv2_gete, srmv2_prestagee,
 * srmv2_bringonline. As the signatures are common, we can reuse the same
 * test function code. */
typedef int (*srmv2_fv_type)(int, const char **, const char *, int, const char *,
    char **, char **, struct srmv2_pinfilestatus **, char *, int, int);


/* There is only 1 file in the requests, and only 1 corresponding file status in
 * the responses. */
#define NUMBER_OF_FILES 1

/* Set up the common part of the SRM responses. The common part: all the tested
 * SRM responses have the same fields, and their values are the same in case of
 * all tests/scenarios. */
static void _prepare_response_common(
    struct soap* soap,
    struct srm2__TReturnStatus ** status,
    int *remainingTotalRequestTime,
    int* __sizestatusArray)
{
    assert(status);
    assert(__sizestatusArray);
    *status = soap_malloc(soap, sizeof(struct srm2__TReturnStatus));
    (*status)->explanation = 0;
    remainingTotalRequestTime = 0;
    *(__sizestatusArray) = NUMBER_OF_FILES;
}

/* Create the SRM response for the srmPrepareToGet call, according to the
 * actual test scenario. */
static void _prepare_response_prepare_to_get(gfal_soap_prepare_to_get_parameters_t* p)
{
    static struct srm2__TGetRequestFileStatus * mock_statuses[NUMBER_OF_FILES];
    assert(p);

    p->rep->srmPrepareToGetResponse = soap_malloc(
        p->soap, sizeof(struct srm2__srmPrepareToGetResponse));

    p->rep->srmPrepareToGetResponse->arrayOfFileStatuses = soap_malloc(
        p->soap, sizeof(struct srm2__ArrayOfTGetRequestFileStatus));

    p->rep->srmPrepareToGetResponse->arrayOfFileStatuses->statusArray = mock_statuses;

    _prepare_response_common(
        p->soap,
        &(p->rep->srmPrepareToGetResponse->returnStatus),
        p->rep->srmPrepareToGetResponse->remainingTotalRequestTime,
        &(p->rep->srmPrepareToGetResponse->arrayOfFileStatuses->__sizestatusArray)
    );
}

/* Create the SRM response for the srmBringOnline call, according to the
 * actual test scenario. */
static void _prepare_response_bring_online(gfal_soap_bring_online_parameters_t* p)
{
    static struct srm2__TBringOnlineRequestFileStatus * mock_statuses[NUMBER_OF_FILES];
    assert(p);

    p->rep->srmBringOnlineResponse = soap_malloc(
        p->soap, sizeof(struct srm2__srmBringOnlineResponse));

    p->rep->srmBringOnlineResponse->arrayOfFileStatuses = soap_malloc(
        p->soap, sizeof(struct srm2__ArrayOfTBringOnlineRequestFileStatus));

    p->rep->srmBringOnlineResponse->arrayOfFileStatuses->statusArray = mock_statuses;

    _prepare_response_common(
        p->soap,
        &(p->rep->srmBringOnlineResponse->returnStatus),
        p->rep->srmBringOnlineResponse->remainingTotalRequestTime,
        &(p->rep->srmBringOnlineResponse->arrayOfFileStatuses->__sizestatusArray)
    );
}

/* Create the SRM response for the srmPrepareToPut call, according to the
 * actual test scenario. */
static void _prepare_response_prepare_to_put(gfal_soap_prepare_to_put_parameters_t* p)
{
    static struct srm2__TPutRequestFileStatus * mock_statuses[NUMBER_OF_FILES];
    assert(p);

    p->rep->srmPrepareToPutResponse = soap_malloc(
        p->soap, sizeof(struct srm2__srmPrepareToPutResponse));

    p->rep->srmPrepareToPutResponse->arrayOfFileStatuses = soap_malloc(
        p->soap, sizeof(struct srm2__ArrayOfTPutRequestFileStatus));

    p->rep->srmPrepareToPutResponse->arrayOfFileStatuses->statusArray = mock_statuses;

    _prepare_response_common(
        p->soap,
        &(p->rep->srmPrepareToPutResponse->returnStatus),
        p->rep->srmPrepareToPutResponse->remainingTotalRequestTime,
        &(p->rep->srmPrepareToPutResponse->arrayOfFileStatuses->__sizestatusArray)
    );
}

/* The dependency injection comes here. This function replaces the production
 * version SOAP calls so that no network operation, server, etc be required.
 * The function directly produces and returns the SRM response objects.
 *
 * This is the "SOAP call" for the srmPrepareToGet operation. */
static int _caller_play_scenarios_prepare_to_get(gfal_soap_prepare_to_get_parameters_t* p)
{
    _prepare_response_prepare_to_get(p);

    switch(_protocol_list_scenario) {
    case E_EMPTY_LIST:
        GFAL_TEST_EQUAL(0, p->req->transferParameters->arrayOfTransferProtocols);
        /* Pretend that the call itself is successful, and do nothing */
         p->rep->srmPrepareToGetResponse->returnStatus->statusCode = SRM_USCORESUCCESS;
        return SOAP_OK;

    case E_UNSUPPORTED_PROTOCOL:
        /* Check if the function filled in the appropriate fields of the request */
        GFAL_TEST_EQUAL(1,
            p->req->transferParameters->arrayOfTransferProtocols->__sizestringArray);

        GFAL_TEST_ASSERT(
            strcmp(p->req->transferParameters->arrayOfTransferProtocols->stringArray[0],
                   _unsupported_protocol[0]) == 0
        );
        /* In case of unsupported protocol, the return status is SRM_NOT_SUPPORTED */
        p->rep->srmPrepareToGetResponse->returnStatus->statusCode = SRM_USCORENOT_USCORESUPPORTED;
        return SOAP_OK;
    default:
        assert(0);
    }

    return 1;
}

/* The dependency injection comes here. This function replaces the production
 * version SOAP calls so that no network operation, server, etc be required.
 * The function directly produces and returns the SRM response objects.
 *
 * This is the "SOAP call" for the srmBringOnline operation. */
static int _caller_play_scenarios_bring_online(gfal_soap_bring_online_parameters_t* p)
{
    _prepare_response_bring_online(p);

    switch(_protocol_list_scenario) {
    case E_EMPTY_LIST:
        GFAL_TEST_EQUAL(0, p->req->transferParameters->arrayOfTransferProtocols);
        /* Pretend that the call itself is successful, and do nothing */
        p->rep->srmBringOnlineResponse->returnStatus->statusCode = SRM_USCORESUCCESS;
        return SOAP_OK;

    case E_UNSUPPORTED_PROTOCOL:
        /* Check if the function filled in the appropriate fields of the request */
        GFAL_TEST_EQUAL(1,
            p->req->transferParameters->arrayOfTransferProtocols->__sizestringArray);

        GFAL_TEST_ASSERT(
            strcmp(p->req->transferParameters->arrayOfTransferProtocols->stringArray[0],
                   _unsupported_protocol[0]) == 0
        );
        /* In case of unsupported protocol, the return status is SRM_NOT_SUPPORTED */
        p->rep->srmBringOnlineResponse->returnStatus->statusCode = SRM_USCORENOT_USCORESUPPORTED;
        return SOAP_OK;
    default:
        assert(0);
    }

    return 1;
}

/* The dependency injection comes here. This function replaces the production
 * version SOAP calls so that no network operation, server, etc be required.
 * The function directly produces and returns the SRM response objects.
 *
 * This is the "SOAP call" for the srmPrepareToPut operation. */
static int _caller_play_scenarios_prepare_to_put(gfal_soap_prepare_to_put_parameters_t* p)
{
    _prepare_response_prepare_to_put(p);

    switch(_protocol_list_scenario) {
    case E_EMPTY_LIST:
        GFAL_TEST_EQUAL(0, p->req->transferParameters->arrayOfTransferProtocols);
        /* Pretend that the call itself is successful, and do nothing */
        p->rep->srmPrepareToPutResponse->returnStatus->statusCode = SRM_USCORESUCCESS;
        return SOAP_OK;

    case E_UNSUPPORTED_PROTOCOL:
        /* Check if the function filled in the appropriate fields of the request */
        GFAL_TEST_EQUAL(1,
            p->req->transferParameters->arrayOfTransferProtocols->__sizestringArray);

        GFAL_TEST_ASSERT(
            strcmp(p->req->transferParameters->arrayOfTransferProtocols->stringArray[0],
                   _unsupported_protocol[0]) == 0
        );
        /* In case of unsupported protocol, the return status is SRM_NOT_SUPPORTED */
        p->rep->srmPrepareToPutResponse->returnStatus->statusCode = SRM_USCORENOT_USCORESUPPORTED;
        return SOAP_OK;
    default:
        assert(0);
    }

    return 1;
}

/* General fixtures for the SRM requests*/
static const char *mock_surls[] = {"fake_surl"};
static int mock_nbfiles = 0;
static const char *mock_srm_endpoint = "fake_endpoint";
static int mock_desiredpintime = 10;
static const char *mock_spacetokendesc = NULL;
static char ** mock_reqtoken = NULL;
static struct srmv2_pinfilestatus * mock_pinfilestatus[3];
static int errbufsz = 1024;
static char errbuf[1024];
static int mock_timeout = 100;

/* Inject the dependencies. Replace the SOAP callers to the test ones. */
static void _setup_srm_callers()
{
    gfal_srm_callers_v2.call_prepare_to_get = _caller_play_scenarios_prepare_to_get;
    gfal_srm_callers_v2.call_bring_online = _caller_play_scenarios_bring_online;
    gfal_srm_callers_v2.call_prepare_to_put = _caller_play_scenarios_prepare_to_put;
}

/* Test the following functions (they have same signatures, so they can be
 * tested in a generic way):
 *
 * srmv2_turlsfromsurls_get, srmv2_gete, srmv2_prestagee, srmv2_bringonline */
static char* _test_protocol_list(srmv2_fv_type fv)
{
    int res;
    errno = 0;
   /* SCENARIO 1: empty protocol list. Call must be OK. */
    _protocol_list_scenario = E_EMPTY_LIST;

    res = fv(mock_nbfiles, mock_surls, mock_srm_endpoint,
        mock_desiredpintime, mock_spacetokendesc,
        NULL, /* The empty protocol list */
        mock_reqtoken, mock_pinfilestatus, errbuf, errbufsz, mock_timeout);

    GFAL_TEST_EQUAL(NUMBER_OF_FILES, res);
    GFAL_TEST_EQUAL(0, errno);
    errno = 0;
    errbuf[0] = 0;

    /* SCENARIO 2: Wrong protocol list. Call must return error. */
    _protocol_list_scenario = E_UNSUPPORTED_PROTOCOL;

    res = fv(mock_nbfiles, mock_surls, mock_srm_endpoint,
        mock_desiredpintime, mock_spacetokendesc,
        _unsupported_protocol,
        mock_reqtoken, mock_pinfilestatus, errbuf, errbufsz, mock_timeout);

    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(EOPNOTSUPP, errno);
}

/* Test the srmv2_turlsfromsurls_put function. */
static char* _test_srmv2_turlsfromsurls_put()
{
    GFAL_LONG64 mock_filesizes[NUMBER_OF_FILES] = {10};
    int res;
    errno = 0;

   /* SCENARIO 1: empty protocol list. Call must be OK. */
    _protocol_list_scenario = E_EMPTY_LIST;

    res = srmv2_turlsfromsurls_put(mock_nbfiles, mock_surls, mock_srm_endpoint,
        mock_filesizes, mock_desiredpintime, mock_spacetokendesc,
        NULL, /* The empty protocol list */
        mock_reqtoken, mock_pinfilestatus, errbuf, errbufsz, mock_timeout);

    GFAL_TEST_EQUAL(NUMBER_OF_FILES, res);
    GFAL_TEST_EQUAL(0, errno);
    errno = 0;
    errbuf[0] = 0;

    /* SCENARIO 2: Wrong protocol list. Call must return error. */
    _protocol_list_scenario = E_UNSUPPORTED_PROTOCOL;

    res = srmv2_turlsfromsurls_put(mock_nbfiles, mock_surls, mock_srm_endpoint,
        mock_filesizes, mock_desiredpintime, mock_spacetokendesc,
        _unsupported_protocol,
        mock_reqtoken, mock_pinfilestatus, errbuf, errbufsz, mock_timeout);

    GFAL_TEST_EQUAL(-1, res);
    GFAL_TEST_EQUAL(EOPNOTSUPP, errno);
}


char * gfal_test__protocol_list_handling()
{
    char* res = NULL;
    _setup_srm_callers();

    __CALL_TEST(_test_protocol_list(srmv2_turlsfromsurls_get));
    __CALL_TEST(_test_protocol_list(srmv2_gete));
    __CALL_TEST(_test_protocol_list(srmv2_prestagee));
    __CALL_TEST(_test_protocol_list(srmv2_bringonline));
    __CALL_TEST(_test_srmv2_turlsfromsurls_put());
    return NULL;
}

#undef __CALL_TEST
