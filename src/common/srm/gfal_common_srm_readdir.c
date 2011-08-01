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
 * @file gfal_common_srm_readdir.c
 * @brief file for the readdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 14/06/2011
 * */
#define GFAL_FILENAME_MAX FILENAME_MAX
#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include "gfal_common_srm_readdir.h"
#include "../gfal_common_errverbose.h"
#include "gfal_common_srm_opendir.h" 
#include "gfal_common_srm_internal_layer.h"
 



struct dirent* gfal_srm_readdir_convert_result(catalog_handle ch, struct srmv2_mdfilestatus * statuses,  struct dirent* output, GError ** err){
	struct dirent* resu = NULL;
	resu = output;
	char* p = strrchr(statuses->surl,'/')+1;
	if(p!=NULL)
		g_strlcpy(resu->d_name, p, GFAL_URL_MAX_LEN);
	else
		g_strlcpy(resu->d_name, statuses->surl, GFAL_URL_MAX_LEN);
	return resu;
}

int gfal_srm_readdir_internal(catalog_handle ch, gfal_srm_opendir_handle oh, int nb_files, GError** err){
	g_return_val_err_if_fail(ch && oh, -1, err, "[gfal_srmv2_opendir_internal] invaldi args");
	GError* tmp_err=NULL;
	int i=-1;
	int resu =-1;
	struct srm_context context;
	struct srm_ls_input input;
	struct srm_ls_output output;
	struct srmv2_mdfilestatus *srmv2_mdstatuses=NULL;
	char errbuf[GFAL_ERRMSG_LEN];
	int ret =-1;
	int offset = oh->dir_offset;
	char* tab_surl[] = { (char*) oh->surl, NULL};
	int tab_resu[nb_files];
	
	gfal_srm_external_call.srm_context_init(&context, oh->endpoint, errbuf, GFAL_ERRMSG_LEN, gfal_get_verbose());	// init context
	
	input.nbfiles = 1;
	input.surls = tab_surl;
	input.numlevels = 1;
	input.offset = &offset;
	input.count = nb_files;

	ret = gfal_srm_external_call.srm_ls(&context,&input,&output);					// execute ls

	if(ret >=0){
		srmv2_mdstatuses = output.statuses;
		if(srmv2_mdstatuses[0].status != 0){
			g_set_error(err, 0, srmv2_mdstatuses->status, "[%s] Error reported from srm_ifce : %d %s", __func__, 
						srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
			resu = -1;	

		}else {
			oh->resu_offset = oh->dir_offset;
			oh->srm_ls_resu = &srmv2_mdstatuses[0];
			//cache system
			resu = 0;
		}	
	}else{
		g_set_error(err,0, ECOMM, "[%s] Bad answer from srm_ifce, maybe voms-proxy is not initiated properly", __func__);
		resu=-1;
	}

	gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
		
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return resu;	
}

struct dirent* gfal_srm_readdir_pipeline(catalog_handle ch, gfal_srm_opendir_handle oh, GError** err){
	struct dirent* ret = NULL;
	GError* tmp_err=NULL;
	const int max_resu_per_req= 5000;
	
	if(oh->srm_ls_resu == NULL){
		gfal_srm_readdir_internal(ch, oh, max_resu_per_req, &tmp_err);		
	}else if(oh->dir_offset >= (oh->resu_offset+ oh->srm_ls_resu->nbsubpaths) ){
		return NULL; // limited mode in order to not overload the srm server ( slow )
		/*
		gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(oh->srm_ls_resu,1);	
		gfal_srm_readdir_internal(ch, oh, max_resu_per_req, &tmp_err);*/
	}
	if(!tmp_err){
		if(oh->srm_ls_resu->nbsubpaths == 0) // end of the list !!
			return NULL;
		const off_t myoffset = oh->dir_offset - oh->resu_offset;
		ret = gfal_srm_readdir_convert_result(ch, &oh->srm_ls_resu->subpaths[myoffset], &oh->current_readdir, &tmp_err);
		oh->dir_offset += 1;
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}


struct dirent* gfal_srm_readdirG(catalog_handle ch, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail( ch && fh, NULL, err, "[gfal_srm_readdirG] Invalid args");
	GError* tmp_err=NULL;
	struct dirent* ret = NULL;
	if(fh != NULL){
		gfal_srm_opendir_handle oh = (gfal_srm_opendir_handle) fh->fdesc;
		ret = gfal_srm_readdir_pipeline(ch, oh, &tmp_err);		
	}else{
		g_set_error(&tmp_err, 0, EBADF, "bad dir descriptor");
		ret = NULL;
	}

	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}
