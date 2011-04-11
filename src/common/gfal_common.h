#pragma once
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
 */

/**
 * @file gfal_common.h
 * @brief the core header file of the common lib part
 * @author Devresse Adrien
 * @version 0.0.1
 * @date 8/04/2011
 * */

#include "gfal_api.h"
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gfal_constants.h"
#include "gfal_common_catalog.h"
#include "gfal_internals.h"
#include "gfal_types.h"





gfal_request gfal_request_new ();
int gfal_init (gfal_request, gfal_internal *, char *, int);
int gfal_deletesurls (gfal_internal, char *, int);
int gfal_removedir (gfal_internal, char *, int);
/* removed func fix it int gfal_ls (gfal_internal, char *, int); */
/* remvoed func fix it int gfal_ls_end (gfal_internal, char *, int); */
int gfal_turlsfromsurls (gfal_internal, char *, int);
int gfal_get (gfal_internal, char *, int);
int gfal_getstatus (gfal_internal, char *, int);
int gfal_bringonline (gfal_internal, char *, int);
int gfal_prestage (gfal_internal, char *, int);
int gfal_prestagestatus (gfal_internal, char *, int);
int gfal_pin (gfal_internal, char *, int);
int gfal_release (gfal_internal, char *, int);
int gfal_set_xfer_done (gfal_internal, char *, int);
int gfal_set_xfer_running (gfal_internal, char *, int);
int gfal_abortrequest (gfal_internal, char *, int);
int gfal_abortfiles (gfal_internal, char *, int);
int gfal_get_results (gfal_internal, gfal_filestatus **);
int gfal_get_ids_setype (gfal_internal, enum se_type *, int *, int **, char **);
int gfal_get_ids (gfal_internal, int *, int **, char **);
int gfal_set_ids (gfal_internal, int, const int *, int, const char *, char *, int);
void gfal_internal_free (gfal_internal);

