/*
 * Copyright (C) 2008-2010 by CERN
 *
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Objects for dependency injection of SRM services.
 * (see: http://en.wikipedia.org/wiki/Dependency_injection)
 *
 * The goal: make the GFAL logic unit-testable by decoupling from the real SOAP
 * calls. Unit tests then create mock objects and create response objects
 * without real soap call / network operations.
 */

/*
 * @(#)$RCSfile: srm_dependencies.h,v $ $Revision: 1.2 $ $Date: 2009/11/05 13:47:03 $ CERN Remi Mollon
 */
#ifndef SRM_DEPENDENCIES_H__
#define SRM_DEPENDENCIES_H__

#define WITH_NOGLOBAL
#include "stdsoap2.h"
#undef SOAP_FMAC3
#define SOAP_FMAC3 static
#undef SOAP_FMAC5
#define SOAP_FMAC5 static
#include "srmv2H.h"
#ifdef GFAL_SECURE
#include "cgsi_plugin.h"
#endif // #ifdef GFAL_SECURE

/* enforce proper calling convention */
#ifdef __cplusplus
extern "C"
{
#endif

/* Function types of the appropriate SOAP calls */
typedef int (*gfal_soap_call_prepare_to_get_fv)(
    struct soap *, const char *, const char *,
    struct srm2__srmPrepareToGetRequest *,
    struct srm2__srmPrepareToGetResponse_ *);

typedef int (*gfal_soap_call_bring_online_fv)(
    struct soap *, const char *, const char *,
    struct srm2__srmBringOnlineRequest *,
    struct srm2__srmBringOnlineResponse_ *);

typedef int (*gfal_soap_call_prepare_to_put_fv)(
    struct soap *, const char *, const char *,
    struct srm2__srmPrepareToPutRequest *,
    struct srm2__srmPrepareToPutResponse_ *);

/* The collection of SOAP service calls. The pointers may be replaced with test
 * functions (dependency injection) */
typedef struct gfal_soap_callers {
    gfal_soap_call_prepare_to_get_fv call_prepare_to_get;
    gfal_soap_call_bring_online_fv call_bring_online;
    gfal_soap_call_prepare_to_put_fv call_prepare_to_put;
} gfal_soap_callers_t;

/* For SRM v1 */
extern gfal_soap_callers_t gfal_srm_callers_v1;

/* For SRM v2 */
extern gfal_soap_callers_t gfal_srm_callers_v2;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef SRM_DEPENDENCIES_H__ */

