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
  * 
  @file lfc_ifce_ng.c
  @brief main internal file of the lfc catalog module
  @author Adrien Devresse
  @version 0.0.1
  @date 02/05/2011
 */


#include "lfc_ifce_ng.h"


char* gfal_get_lfchost_envar(GError** err){

	char *lfc_host=NULL, *lfc_port=NULL;
	char lfc_endpoint[GFAL_MAX_LFCHOST_LEN];
	if( (lfc_host = getenv ("LFC_HOST")) !=NULL){
		if (strnlen (lfc_host,GFAL_MAX_LFCHOST_LEN) + 6 >= GFAL_MAX_LFCHOST_LEN)  {
				g_set_error(err, 0, ENAMETOOLONG, "[gfal_get_lfchost_envar] Host name from LFC_HOST env var too long");
				return NULL;
		}

		lfc_port = getenv ("LFC_PORT");
		if (lfc_port && strlen (lfc_port) > 5 && atoi(lfc_port) != 0) {
			g_set_error(err, 0, EINVAL , "[gfal_get_lfchost_envar]  Invalid LFC port number in the LFC_PORT env var");
			return NULL;
		}

		if (lfc_port)
			snprintf (lfc_endpoint, GFAL_MAX_LFCHOST_LEN, "%s:%s", lfc_host, lfc_port);
		else
			snprintf (lfc_endpoint, GFAL_MAX_LFCHOST_LEN, "%s", lfc_host);	
	}

	return strdup(lfc_endpoint);
}

static int gfal_define_lfc_env_var(char* lfc_host, GError** err){
	char* lfc_endpoint=NULL, *lfc_port = NULL;
	if (strncmp (lfc_host, "lfc://", 6) == 0)				// correct the url begining with lfc://
		strncpy (lfc_endpoint, lfc_host + 6, GFAL_HOSTNAME_MAXLEN);
	else /* just a plain hostname */
		strncpy (lfc_endpoint, lfc_host, GFAL_HOSTNAME_MAXLEN);

	if ((lfc_port = strchr (lfc_endpoint, ':')) != NULL){	// register LFC_PORT var and clear port number
		if (strnlen (lfc_port+1,6) > 5  && atoi(lfc_port) ==0) {
			g_set_error(err,0, EINVAL, "[gfal_define_lfc_env_var] %s: Invalid LFC port number", lfc_port+1);
			return (-1);
		}
		setenv("LFC_PORT", lfc_port+1,1);
		*lfc_port = '\0';	
		setenv("LFC_HOST", lfc_host, 1);
	}
	return 0;
}

/**
 * setup the lfc_host correctly for the lfc calls 
 * @param err GError report system if 
 * @return 0 if success else negative value and set err properly
 */
int gfal_setup_lfchost(gfal_handle handle, GError ** err){
	char* lfc_host = NULL;
	GError* tmp_err = NULL;
	
	if ( (lfc_host = gfal_get_lfchost_envar(&tmp_err)) == NULL ) { // if env var not specified got one from bdii, and setup the env var
		if(tmp_err){
			g_propagate_prefixed_error(err, tmp_err, "[gfal_get_lfchost]");
			return -1;
		}
		
		if( (lfc_host = gfal_get_lfchost_bdii(handle, &tmp_err)) ==NULL ){ // call the bdii 
			g_propagate_prefixed_error(err, tmp_err, "[gfal_get_lfchost]");
			return -2;	
		}else{				
			gfal_define_lfc_env_var(lfc_host, &tmp_err);		// define the env var if sucess
			if(tmp_err){
				g_propagate_prefixed_error(err, tmp_err,"[gfal_get_lfchost]");
				return -4;
			}	
		}
	} 

	if (strnlen (lfc_host,GFAL_MAX_LFCHOST_LEN) + 6 >= GFAL_MAX_LFCHOST_LEN) { 
		g_set_error(err, 0, ENAMETOOLONG, "[gfal_get_lfchost] lfc host name :  %s, Host name too long", lfc_host);
		return (-3);		
	}	
	free(lfc_host);
	return 0;
}


