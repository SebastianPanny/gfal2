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
 * @file gfal_common_errverbose.h
 * @brief the header file of the common lib for error management and verbose display
 * @author Devresse Adrien
 * @version 0.0.1
 * @date 8/04/2011
 * */


#include "gfal_constants.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

extern const char* no_err;

/** @def macro for fast debug purpose
 * 
 */
#define g_return_val_err_if_fail(exp,val,err,msg) if(!(exp)){ g_set_error(err,0,EINVAL,msg); return val; }

void gfal_print_verbose(int verbose_lvl,const char* msg, ...);
/**
 * \brief set the verbose mode for the current program
 * 
 * 
 */
int gfal_set_verbose (int value);

int gfal_get_verbose();


void gfal_release_GError(GError** err);

gboolean gfal_check_GError(GError** err);

char* gfal_str_GError(GError** err);


#if (GLIB_CHECK_VERSION(2,16,0) != TRUE)			// add a advanced functions of glib for old version of glib

#define ERROR_OVERWRITTEN_WARNING "GError set over the top of a previous GError or uninitialized memory.\n"
 
void     g_propagate_prefixed_error   (GError       **dest,
								   GError        *src,
								   const gchar   *format,
								   ...) G_GNUC_PRINTF (3, 4);
 
#endif


#if (GLIB_CHECK_VERSION(2,28,0) != TRUE)


void g_list_free_full(GList *list, GDestroyNotify free_func);

#endif
