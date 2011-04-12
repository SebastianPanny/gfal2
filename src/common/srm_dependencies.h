/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

