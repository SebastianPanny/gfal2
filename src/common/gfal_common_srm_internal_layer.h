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

/**
 * structure for mock abylity in the srm part
 *
 */
struct _gfal_srm_external_call{
	int (*srm_ls)(struct srm_context *context,
		struct srm_ls_input *input,struct srm_ls_output *output);
		
	int (*srm_rmdir)(struct srm_context *context,
		struct srm_rmdir_input *input,struct srm_rmdir_output *output);
		
	int (*srm_getpermission) (struct srm_context *context,
		struct srm_getpermission_input *input,struct srm_getpermission_output *output);

	int (*srm_check_permission)(struct srm_context *context,
		struct srm_checkpermission_input *input,struct srmv2_filestatus **statuses);	
	
};

extern struct _gfal_srm_external_call gfal_srm_external_call;
