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

/**
 * \brief display a verbose message 
 * 
 * msg is displayed if current verbose level is superior to verbose mode specified
 * 
 */
void gfal_print_verbose(int verbose_lvl,const char* msg, ...);
/**
 * \brief set the verbose mode for the current program
 * 
 * 
 */
int gfal_set_verbose (int value);
/**
 * \brief return verbose mode level
 */
int gfal_get_verbose();

