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
 * @file gfal_common_errverbose.c
 * @brief the file of the common lib for error management and verbose display
 * @author Devresse Adrien
 * @version 0.0.1
 * @date 8/04/2011
 * */

#include "gfal_common_errverbose.h"

/*
 * Verbose level
 *   API mode (no messages on stderr) by default
 *   CLI has to set it to '0' to get normal error messages
 */
static int gfal_verbose = -1;

extern int gfal_get_verbose(){
	return gfal_verbose;
}

extern int gfal_set_verbose (int value)
{
    if (value < 0)
        return (-1);
    gfal_verbose = value;
    return (0);
}


 extern void gfal_print_verbose(int verbose_lvl, const char* msg, ...){
	 if(verbose_lvl <= gfal_get_verbose()){
			va_list args;
			va_start(args, msg);
			printf(msg, args); 
			va_end(args);		 
	 }

 }
 
 
 extern void gfal_release_GError(GError** err){
	 fprintf(stderr,"[gfal]%s", (*err)->message);
	 free(*err);
	 *err=NULL;	 
 }


