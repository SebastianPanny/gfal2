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
 * @file gfal_rfio_plugin_main.c
 * @brief header file for bindings for rfio funcs
 * @author Devresse Adrien
 * @version 0.1
 * @date 30/06/2011
 * 
 **/


#include <regex.h>
#include <time.h> 
#include <glib.h>
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_catalog.h"
#include "../gfal_common_filedescriptor.h"
#include "../gfal_types.h"
#include "gfal_rfio_plugin_bindings.h"
#include "gfal_rfio_plugin_main.h"
#include "gfal_rfio_plugin_layer.h"

static void rfio_report_error(gfal_plugin_rfio_handle h,  const char * func_name, GError** err){
	char buff_error[2048];
	const int status = h->rf->geterror();
	g_set_error(err, 0, status, "[%s] Error reported by the external library rfio : %s", func_name, strerror_r(status, buff_error, 2048));
}

gfal_file_handle gfal_rfio_openG(catalog_handle handle , const char* path, int flag, mode_t mode, GError** err){
	gfal_plugin_rfio_handle h = (gfal_plugin_rfio_handle) handle;
	int fd= h->rf->open(path, flag, mode);
	if(fd == 0)
		rfio_report_error(h, __func__, err);
	return gfal_file_handle_new(gfal_rfio_getName(), GINT_TO_POINTER(fd));
}

int gfal_rfio_readG(catalog_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, GError** err){
	gfal_plugin_rfio_handle h = (gfal_plugin_rfio_handle) handle;
	int ret = h->rf->read(GPOINTER_TO_INT(fd->fdesc), buff, s_buff);
	if(ret <0)
		rfio_report_error(h, __func__, err);
	else
		errno=0;
	return ret;
}

int gfal_rfio_writeG(catalog_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, GError** err){
	gfal_plugin_rfio_handle h = (gfal_plugin_rfio_handle) handle;
	int ret = h->rf->write(GPOINTER_TO_INT(fd->fdesc), buff, s_buff);
	if(ret <0)
		rfio_report_error(h, __func__, err);
	return ret;
}




int gfal_rfio_closeG(catalog_handle handle, gfal_file_handle fd, GError ** err){
	gfal_plugin_rfio_handle h = (gfal_plugin_rfio_handle) handle;
	int ret= h->rf->close(GPOINTER_TO_INT(fd->fdesc));
	if(ret != 0)
		rfio_report_error(h, __func__, err);
	return ret;	
}

const char* gfal_rfio_getName(){
	return "rfio";
}

