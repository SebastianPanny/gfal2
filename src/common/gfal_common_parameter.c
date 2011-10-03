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
 * @file gfal_common_parameter.c
 * @brief file for internal settings set/get
 * @author Devresse Adrien
 * @version 2.0
 * @date 02/10/2011
 * */

#include <glib.h>
#include <stdlib.h>
#include "gfal_common_errverbose.h"


static gboolean no_bdii = FALSE;

int gfal_common_bdii(char* value, size_t max_size, int flag_mode, GError ** err){
  GError* tmp_err = NULL;
  int res = -1;
  if(value !=NULL){
      if( flag_mode == GFAL_PARAM_SET){
	if(strncmp(value, GFAL_PARAM_TRUE, max_size)== 0){
	  no_bdii = TRUE;
	  res = 0;
	}else if(strncmp(value, GFAL_PARAM_FALSE, max_size)== 0){
	  no_bdii = FALSE;
	  res = 0;
	}else{
	g_set_error(&tmp_err, 0, EINVAL, "bad value for this parameter %s, but be %s or %s",GFAL_NO_BDII_OPTION, GFAL_PARAM_TRUE, GFAL_PARAM_FALSE );  
	}
      }else{
	  g_strlcpy((char*) value, (no_bdii)? GFAL_PARAM_TRUE:GFAL_PARAM_FALSE, max_size);
	  res=0;
      }
    
  } else{
     g_set_error(&tmp_err, 0, EINVAL, "value  of the parameter set to NULL");   
  }
  return res;
}

int gfal_common_parameter(const char* name, char * value, size_t max_size, int flag_mode, GError** err){
  GError* tmp_err=NULL;
  int res = -1;
  
  if(name != NULL){

      if(strcmp(name, GFAL_NO_BDII_OPTION) ==0){
	res = gfal_common_bdii(value, max_size, flag_mode, &tmp_err);
      }else{
	  g_set_error(&tmp_err, 0, ENOENT, "Unknow parameter %s ", value);    
      }
  }else{
    g_set_error(&tmp_err, 0, EINVAL, "Name of the parameter set to NULL");
  }
  
  if(tmp_err)
    g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
  return res;
  
}


