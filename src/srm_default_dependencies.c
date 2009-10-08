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
 * @(#)$RCSfile: srm_default_dependencies.c,v $ $Revision: 1.1 $ $Date: 2009/10/08 15:32:39 $ CERN Remi Mollon
 */

#define _GNU_SOURCE
#include "srm_dependencies.h"



