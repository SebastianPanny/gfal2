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
 * @(#)$RCSfile: srm_default_dependencies.c,v $ $Revision: 1.1 $ $Date: 2009/10/08 15:32:39 $ CERN Remi Mollon
 */

#define _GNU_SOURCE
#include "srm_dependencies.h"



