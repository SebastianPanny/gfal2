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
 * @file gfal_common_all.c
 * @brief core file for the utility function of gfal common part
 * @author Devresse Adrien
 * @version 0.0.1
 * @date 02/05/2011
 * */


#include "gfal_common.h"
#include "gfal_common_errverbose.h"
#include <dlfcn.h>
#include <regex.h>


/* the version should be set by a "define" at the makefile level */
static const char *gfalversion = VERSION;

 /**
 * initiate a gfal's context with default parameters for use
 * @return a gfal_handle, need to be free after usage. return NULL if errors
 */
gfal_handle gfal_initG (GError** err)
{
	gfal_handle handle = calloc(1,sizeof(struct gfal_handle_));// clear allocation of the struct and set defautl options
	if(handle == NULL){
		errno= ENOMEM;
		g_set_error(err,0,ENOMEM, "[gfal_initG] bad allocation, no more memory free");
		return NULL;
	}
	handle->err= NULL;
	handle->srm_proto_type = PROTO_SRMv2;
	handle->initiated = 1;
	handle->catalog_opt.catalog_number=-1;
	handle->srmv2_opt = calloc(1,sizeof(struct _gfal_srmv2_opt));	// define the srmv2 option struct and clear it
	return handle;
}

/**
 * @brief free a gfal's handle, safe if null
 * 
 */
void gfal_handle_freeG (gfal_handle handle){
	if(handle == NULL)
		return;
	g_clear_error(&(handle->err));
	free(handle->srmv2_opt);
	gfal_delete_request_state(handle->last_request_state);
	free(handle);
	handle = NULL;
}


/**
 * @brief convert glist of surl char* to char**
 *  @return return NULL if error or pointer to char**
 */
char** gfal_GList_to_tab(GList* surls){
	int surl_size = g_list_length(surls);
	int i;
	char **resu = surl_size?((char**)calloc(surl_size+1, sizeof(char*))):NULL;
	for(i=0;i<surl_size; ++i){
		resu[i]= surls->data;
		surls = g_list_next(surls);
	}
	return resu;
}

/**
 * @brief convert glist of int to a table of int
 *  @return return NULL if error or pointer to int*
 */
int* gfal_GList_to_tab_int(GList* int_list){
	int int_size = g_list_length(int_list);
	int i;
	int *resu = int_size?((int*)calloc(int_size+1, sizeof(int))):NULL;
	for(i=0;i<int_size; ++i){
		resu[i]= GPOINTER_TO_INT(int_list->data);
		int_list = g_list_next(int_list);
	}
	return resu;
}


/**
 *  @brief open dynamycally a list of symbols
 *  resolve all symbols from sym_list to flist with the associated dlhandle
 *  set GError properly if error
 *  
 * */
int resolve_dlsym_listG(void* handle, void*** flist, const char** sym_list, int num, GError** err){
	if(num >0){
		void* sym = dlsym(handle, *sym_list);
		if(!sym){
			g_set_error(err,0, EBADR, "[resolve_dlsym_listG] Unable to resolve symbol %s, dlsym Error : %s ",*sym_list, dlerror());
			return -1;
		}
		**flist= sym;
		return resolve_dlsym_listG(handle, flist+1, sym_list+1, num-1, err);
	}else 
		return num;
	
}

/**
 * return a string of the current gfal version
 * 
 * */
const char * gfal_version (){
    return gfalversion;
}

/**
 *  return a pointer to the internal last error of the handle
 * */
GError** gfal_get_last_gerror(gfal_handle handle){
	return &(handle->err);
}

/***
 * check the validity of a result for a "access" standard call
 * @return TRUE if status is a valid and standard access methode error, else return FALSE
 * */
 gboolean is_valid_access_result(int status){
	 switch(status){
		 case 0:
		 case ENOENT:
		 case EACCES:
		 case ELOOP:
		 case ENAMETOOLONG:
		 case ENOTDIR:
		 case EROFS:{
			return TRUE;
		 }
		 default:{
			return FALSE;		 
		}
	}
	 
 }
 
 
 /**
 * parse a guid to check the validity
 */
gboolean gfal_guid_checker(const char* guid, GError** err){
	g_return_val_err_if_fail(guid != NULL,FALSE,err,"[gfal_guid_checker] check URL failed : guid is empty");
	regex_t rex;
	int ret = regcomp(&rex, "^guid:([0-9]|[a-z]|[A-Z]){8}-([0-9]|[a-z]|[A-Z]){4}-([0-9]|[a-z]|[A-Z]){4}-([0-9]|[a-z]|[A-Z]){4}-([0-9]|[a-z]|[A-Z]){12}",REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret==0,FALSE,err,"[gfal_guid_checker] fail to compile regex, report this bug");
	ret=  regexec(&rex,guid,0,NULL,0);
	if(ret) 
		g_set_error(err,0,EINVAL,"[gfal_guid_checker] Incorrect guid, impossible to parse guid %s :", guid);
	return (!ret)?TRUE:FALSE;
} 
