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
 * @file gfal_common_srm_open.c
 * @brief file for the open function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 06/07/2011
 * */

#include "gfal_common_srm_rmdir.h"
#include "gfal_constants.h"
#include "gfal_common_errverbose.h"
#include <gfal_srm_ifce_types.h> 


gfal_file_handle gfal_srm_open(gfal_handle handle, const char* path, int flag, mode_t mode, GError** err){
	gfal_file_handle fh = NULL;
	GError * tmp_err=NULL;
	g_set_error(&tmp_err, 0, ENOSYS, "not implmented");
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return fh;
}
