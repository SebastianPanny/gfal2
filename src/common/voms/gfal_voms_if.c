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
 * @file gfal_voms_if.c
 * @brief file containing the interface to the voms system
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/04/2011
 * */
 
#include "gfal_voms_if.h"
#include "gfal_voms_if_layer.h"

typedef struct{
	GList* fqan;
	char* userdn;
	char* vo;
	
} gfal_voms_info_;

static __thread gfal_voms_info_* gfal_voms_info=NULL; 

static int gfal_voms_init(struct vomsdata **vd, GError **err){
    char errmsg[GFAL_ERRMSG_LEN];	
    int error;
    if( (*vd = gfal_voms_external.VOMS_Init ("", "")) ==NULL){
		g_set_error(err,0, EINVAL, " [parse_vomsdataG][VOMS_Init] VOMS_Init failed, \
maybe voms-proxy is not initiated correctly");
		gfal_voms_external.VOMS_Destroy(*vd);
		return -1;				
	}
	if( !gfal_voms_external.VOMS_SetVerificationType (VERIFY_NONE, *vd, &error)){
        gfal_voms_external.VOMS_ErrorMessage (*vd, error, errmsg, GFAL_ERRMSG_LEN);	
 		g_set_error(err, 0, EINVAL, "[parse_vomsdataG][VOMS_SetVerificationType] error : %s, \
maybe is not initiated correctly", errmsg);      
 		gfal_voms_external.VOMS_Destroy(*vd); 	
		return -2;
	}	
	if( !gfal_voms_external.VOMS_RetrieveFromProxy (RECURSE_CHAIN, *vd, &error)){
        gfal_voms_external.VOMS_ErrorMessage (*vd, error, errmsg, GFAL_ERRMSG_LEN);	
 		g_set_error(err,0, EINVAL, "[parse_vomsdataG][VOMS_RetrieveFromProxy] error : %s, \
maybe voms-proxy is not initiated correctly", errmsg);      
 		gfal_voms_external.VOMS_Destroy(*vd); 	
		return -3;			
	}
	if( !(*vd)->data || !(*vd)->data[0]) {
            g_set_error(err,0, EINVAL, "[gfal_parse_vomsdata] Unable to get VOMS info from the proxy (Memory problem?)");
            return (-4);
    }	
    return 0;	
}

/**
 * get the data from the proxy and store it
 */ 
int gfal_parse_vomsdataG (GError ** err){

	if(gfal_voms_info!=NULL) // singleton already exist
		return 0;
	
	struct vomsdata *vd;
	int i=0;
	if(gfal_voms_init(&vd, err))
		return -1;
		
	gfal_voms_info = malloc(sizeof(gfal_voms_info_));
	gfal_voms_info->fqan= NULL;
	gfal_voms_info->userdn =  strdup (vd->data[0]->user);
	gfal_voms_info->vo = strdup (vd->data[0]->voname);
    for (i = 0; vd->data[0]->fqan[i] != NULL; ++i) {
			char * str;
            if ((str = strndup (vd->data[0]->fqan[i],2048)) == NULL){
 				g_set_error(err,0, EINVAL, "[parse_vomsdataG] fqan value retrived from VOMS invalid");   
 				gfal_voms_info = NULL ;  
				gfal_voms_external.VOMS_Destroy(vd); 					
                return (-1);
			}
			if(*str != '/'){
 				g_set_error(err,0, EINVAL, "[parse_vomsdataG] fqan first char retrived from VOMS invalid");    
 				gfal_voms_info = NULL;  
 				free(str);
				gfal_voms_external.VOMS_Destroy(vd); 					
                return (-1);				
			}
			gfal_voms_info->fqan = g_list_append(gfal_voms_info->fqan,(gpointer) str);
	}
    gfal_voms_external.VOMS_Destroy (vd);	
    return (0);
}
/**
 * @brief dn of the current registered user
 * @return string of the dn of the current user registered by the voms proxy or null if error
 * need to be free after use
 */ 
char *gfal_get_userdnG (GError ** err){
	GError* tmp_err=NULL;
	if( gfal_parse_vomsdataG(&tmp_err) != 0){// obtain from the singleton
		g_propagate_prefixed_error (err, tmp_err, "[get_userdn]");
		return NULL;
	}			
    return strndup(gfal_voms_info->userdn,100);
}


/**
 * @brief vo of the current user
 * @return string of the VO of the current user or null ir error
 * need to be free after use
 */
char * gfal_get_voG (GError** err){
	char* gfal_vo=NULL;
	GError* tmp_err=NULL;	
	if((gfal_vo = getenv ("LCG_GFAL_VO")) != NULL){	// check var env
		gfal_print_verbose(GFAL_VERBOSE_VERBOSE,"[gfal_get_voG] LCG_GFAL_VO env var defined, this VO will be used");
		return gfal_vo;
	}
	if(gfal_parse_vomsdataG (&tmp_err) != 0){
		g_propagate_prefixed_error(err,tmp_err,"[get_voG]");
		return NULL;
	}
    return strndup(gfal_voms_info->vo,100);
}
/**
 * @brief get the user attributes
 * @return return a List of strings of the compact user Attribute or NULL if error
 * do not free
 */ 
GList* gfal_get_fqanG (GError** err){
	GError* tmp_err=NULL;	
	if(gfal_parse_vomsdataG (&tmp_err) != 0){	// obtain from the singleton
		g_propagate_prefixed_error(err,tmp_err,"[get_fqanG]");
		return NULL;
	} 
    return gfal_voms_info->fqan;
}


/**
 *  Free the memory of the voms parameters
 * */
void gfal_voms_destroy(){
	if(gfal_voms_info){
		g_list_free_full(gfal_voms_info->fqan, free);
		free(gfal_voms_info->userdn);
		free(gfal_voms_info->vo);
		free(gfal_voms_info);
		gfal_voms_info=NULL;
		
	}
}


 
 
