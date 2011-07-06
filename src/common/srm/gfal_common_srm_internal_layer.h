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
 * @file gfal_common_srm_internal_layer.h
 * @brief header file for the srm external function mapping for mocking purpose
 * @author Devresse Adrien
 * @version 2.0
 * @date 09/06/2011
 * */

#include <gfal_srm_ifce.h> 
#include <gfal_srm_ifce_types.h> 
#include <glib.h>
#include "../gfal_types.h"

/**
 * structure for mock abylity in the srm part
 *
 */
struct _gfal_srm_external_call{
	
	void (*srm_context_init)(struct srm_context *context,char *srm_endpoint,char *errbuf,int errbufsz,int verbose);
	
	int (*srm_ls)(struct srm_context *context,
		struct srm_ls_input *input,struct srm_ls_output *output);
		
	int (*srm_rmdir)(struct srm_context *context,
		struct srm_rmdir_input *input,struct srm_rmdir_output *output);
		
	int (*srm_mkdir)(struct srm_context *context,
		struct srm_mkdir_input *input);
		
	int (*srm_getpermission) (struct srm_context *context,
		struct srm_getpermission_input *input,struct srm_getpermission_output *output);

	int (*srm_check_permission)(struct srm_context *context,
		struct srm_checkpermission_input *input,struct srmv2_filestatus **statuses);	

	int (*srm_prepare_to_get)(struct srm_context *context,
		struct srm_preparetoget_input *input,struct srm_preparetoget_output *output);
		
	void (*srm_srmv2_pinfilestatus_delete)(struct srmv2_pinfilestatus*  srmv2_pinstatuses, int n);
	
	void (*srm_srmv2_mdfilestatus_delete)(struct srmv2_mdfilestatus* mdfilestatus, int n);
	
	void (*srm_srmv2_filestatus_delete)(struct srmv2_filestatus*  srmv2_statuses, int n);
	
	void (*srm_srm2__TReturnStatus_delete)(struct srm2__TReturnStatus* status);
	
	int (*srm_prepare_to_put)(struct srm_context *context,
		struct srm_preparetoput_input *input,struct srm_preparetoput_output *output);
	
};

extern struct _gfal_srm_external_call gfal_srm_external_call;

int gfal_check_fullendpoint_in_surl(const char * surl, GError ** err);


int gfal_srm_getTURLS_catalog(catalog_handle ch, const char* surl, char* buff_turl, int size_turl, GError** err);

int gfal_srm_putTURLS_catalog(catalog_handle ch, const char* surl, char* buff_turl, int size_turl, GError** err);
