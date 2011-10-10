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
 * @file gfal_posix_parameters.c
 * @brief new file for the parameters management
 * @author Devresse Adrien
 * @date 02/10/2011
 * */

#include <glib.h>
#include <stdlib.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>


#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_parameter.h>

#include "gfal_posix_internal.h"

/**
 * internal function to set parameter / options
 * 
 */
inline int gfal_posix_internal_set_parameter(const char* module, const char* name, char* value, size_t max_size, GFAL_TYPE req_type){
  GError* tmp_err=NULL;
  gfal_handle handle;
  int res = -1;

  gfal_print_verbose(GFAL_VERBOSE_TRACE, "%s ->",__func__);

  if((handle = gfal_posix_instance()) == NULL){
    errno = EIO;
    return -1;
  }
  
  if(module == NULL){ // internal parameters
    res = gfal_common_parameter(name, (void*) value, max_size, GFAL_PARAM_SET, req_type, &tmp_err);
  }else{
    res = gfal_common_plugin_parameter(handle, module, name, value, max_size, GFAL_PARAM_SET, req_type, &tmp_err);
  }
  
  if(tmp_err){
    gfal_posix_register_internal_error(handle, "[gfal_set_parameter]", tmp_err);
    errno = tmp_err->code;	
  }else
    errno=0;
  
  return res;
}


/**
 * internal function for get parameter / options
 * 
 */
inline int gfal_posix_internal_get_parameter(const char* module, const char* name, char* value, size_t buff_size, GFAL_TYPE req_type){
  GError* tmp_err=NULL;
  gfal_handle handle;
  int res = -1;

  gfal_print_verbose(GFAL_VERBOSE_TRACE, "%s ->",__func__);

  if((handle = gfal_posix_instance()) == NULL){
    errno = EIO;
    return -1;
  }
  
  if(module == NULL){ // internal parameters
    res = gfal_common_parameter(name, value, buff_size, GFAL_PARAM_GET, req_type, &tmp_err);
  }else{
    res = gfal_common_plugin_parameter(handle, module, name, value, buff_size, GFAL_PARAM_GET,req_type, &tmp_err);
  }
  
  if(tmp_err){
    gfal_posix_register_internal_error(handle, "[gfal_get_parameter]", tmp_err);
    errno = tmp_err->code;	
  }else
    errno=0;
  
  return res;
}

