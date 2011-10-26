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
 * @file gfal_dcap_plugin_layer.c
 * @brief file for the external call, abstraction layer for mock purpose
 * @author Devresse Adrien
 * @date 20/07/2011
 * 
 **/


#include <regex.h>
#include <time.h> 
#include <dlfcn.h>
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_plugin.h"
#include "../gfal_types.h"
#include "gfal_dcap_plugin_layer.h"

static char* libdcap_name= "libdcap.so.1";

struct dcap_proto_ops * gfal_dcap_internal_loader_base(GError** err){
	void *dlhandle;
	struct dcap_proto_ops * pops = NULL;
	GError* tmp_err=NULL;

	if( (dlhandle = dlopen(libdcap_name, RTLD_LAZY)) == NULL){
			g_set_error(&tmp_err, 0, EPROTONOSUPPORT, " library %s for the dcap_plugin cannot be loaded properly, failure : %s ", libdcap_name, dlerror());
	}


	if(dlhandle){
		gfal_print_verbose(GFAL_VERBOSE_VERBOSE, "dcap library's name : %s", libdcap_name);	
		
		pops = g_new0(struct dcap_proto_ops, 1);
		pops->geterror = (int* (*) ()) dlsym (dlhandle, "__dc_errno");
		pops->strerror = (const char* (*)(int)) dlsym (dlhandle, "dc_strerror");
		pops->access = (int (*) (const char *, int)) dlsym (dlhandle, "dc_access");
		pops->chmod = (int (*) (const char *, mode_t)) dlsym (dlhandle, "dc_chmod");
		pops->close = (int (*) (int)) dlsym (dlhandle, "dc_close");
		pops->closedir = (int (*) (DIR *)) dlsym (dlhandle, "dc_closedir");
		pops->lseek = (off_t (*) (int, off_t, int)) dlsym (dlhandle, "dc_lseek");
		pops->lseek64 = (off64_t (*) (int, off64_t, int)) dlsym (dlhandle, "dc_lseek64");
		pops->lstat = (int (*) (const char *, struct stat *)) dlsym (dlhandle, "dc_lstat");
		pops->lstat64 = (int (*) (const char *, struct stat64 *)) dlsym (dlhandle, "dc_lstat64");
		pops->mkdir = (int (*) (const char *, mode_t)) dlsym (dlhandle, "dc_mkdir");
		pops->open = (int (*) (const char *, int, ...)) dlsym (dlhandle, "dc_open");
		pops->opendir = (DIR * (*) (const char *)) dlsym (dlhandle, "dc_opendir");
		pops->read = (ssize_t (*) (int, void *, size_t)) dlsym (dlhandle, "dc_read");
		pops->pread = (ssize_t (*)(int fildes, void *buf, size_t nbytes, off_t offset)) dlsym(dlhandle, "dc_pread");
		pops->readdir = (struct dirent * (*) (DIR *)) dlsym (dlhandle, "dc_readdir");
		pops->readdir64 = (struct dirent64 * (*) (DIR *)) dlsym (dlhandle, "dc_readdir64");
		pops->rename = (int (*) (const char *, const char *)) &rename;
		pops->rmdir = (int (*) (const char *)) dlsym (dlhandle, "dc_rmdir");
		pops->stat = (int (*) (const char *, struct stat *)) dlsym (dlhandle, "dc_stat");
		pops->stat64 = (int (*) (const char *, struct stat64 *)) dlsym (dlhandle, "dc_stat64");
		pops->unlink = (int (*) (const char *)) dlsym (dlhandle, "dc_unlink");
		pops->write = (ssize_t (*) (int, const void *, size_t)) dlsym (dlhandle, "dc_write");
		pops->pwrite = (ssize_t (*)(int fildes, const void *buf, size_t nbytes, off_t offset)) dlsym(dlhandle, "dc_pwrite");
		pops->debug_level= (void(*)(unsigned int)) dlsym(dlhandle, "dc_setDebugLevel");
		pops->active_mode = (void(*)(void)) dlsym(dlhandle, "dc_setClientActive");
		
		//
		pops->active_mode(); // switch to active mode to avoid firewalls problems
		if( (gfal_get_verbose() & GFAL_VERBOSE_TRACE ) )
			pops->debug_level(8 |6 | 32);
	}
	if(tmp_err)
			g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return pops;
	
}


struct dcap_proto_ops * (*gfal_dcap_internal_loader)(GError** err)= &gfal_dcap_internal_loader_base;

