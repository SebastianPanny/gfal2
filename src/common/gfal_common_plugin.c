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
 * @file gfal_common_plugin.c
 * @brief the file of the common lib for the plugin management
 * @author Devresse Adrien
 * @date 8/04/2011
 * */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "gfal_types.h"
#include "gfal_common_plugin.h"
#include "gfal_constants.h"
#include "gfal_common_errverbose.h"
#include "gfal_common_filedescriptor.h"


/**
 * convenience function for safe calls to the plugin checkers
 * 
 * */
inline static gboolean gfal_plugin_checker_safe(gfal_plugin_interface* cata_list, const char* path, plugin_mode call_type, GError** terr ){
	if(cata_list->check_plugin_url)
		return cata_list->check_plugin_url(cata_list->handle, path, call_type, terr);
	else{
		g_set_error(terr, 0, EPROTONOSUPPORT, "[%s] unexcepted NULL pointer for a call to the chercker in the plugin %s",__func__, cata_list->getName());
		return FALSE;
	}
}

/**
 * resolve and execute init func in a given module
 */
static int gfal_module_init(gfal_handle handle, void* dlhandle, const char* module_name, GError** err){
	GError* tmp_err=NULL;
	static gfal_plugin_interface (*constructor)(gfal_handle,GError**);
	int* n = &handle->plugin_opt.plugin_number;
	int ret =-1;
	constructor= (gfal_plugin_interface (*)(gfal_handle,GError**)) dlsym(dlhandle, GFAL_PLUGIN_INIT_SYM);
	if(constructor == NULL){
		g_set_error(&tmp_err, 0, EINVAL, "No symbol %s found in the plugin %s, failure", GFAL_PLUGIN_INIT_SYM, module_name);
		*n=0;
	}else{	
		handle->plugin_opt.plugin_list[*n] = constructor(handle, &tmp_err);
		if(tmp_err){
			g_prefix_error(&tmp_err, " Unable to load plugin %s : ", module_name);		
			*n=0;		
		}else{
			*n+=1;
			gfal_print_verbose(GFAL_VERBOSE_NORMAL, " [gfal_module_load] plugin %s loaded with success ", module_name);
			ret=0;
		}
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}


/**
 * 
 * return the proper plugin linked to this file handle
 */
static gfal_plugin_interface* gfal_plugin_getModuleFromHandle(gfal_handle handle, gfal_file_handle fh, GError** err){
	GError* tmp_err=NULL;
	int i;
	gfal_plugin_interface* cata_list=NULL;
	int n = gfal_plugins_instance(handle, &tmp_err);
	if(n > 0){
		cata_list = handle->plugin_opt.plugin_list;
		for(i=0; i < n; ++i){
			if( strncmp(cata_list[i].getName(),fh->module_name, GFAL_MODULE_NAME_SIZE)==0 )
				return &(cata_list[i]);
		}
		g_set_error(&tmp_err, 0, EBADF, "No gfal_module with the handle name : %s", fh->module_name);	
	}else{
		g_set_error(&tmp_err, 0, EINVAL, "No gfal_module loaded");
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return cata_list;
}


char** gfal_plugins_get_list(gfal_handle handle, GError** err){
	GError* tmp_err=NULL;
	char** resu =NULL;
	int n = gfal_plugins_instance(handle, &tmp_err);
	if(n > 0){
		resu =g_new0(char*,n+1);
		int i;
		gfal_plugin_interface* cata_list = handle->plugin_opt.plugin_list;
		for(i=0; i < n; ++i, ++cata_list){
			resu[i] = strndup(cata_list->getName(), GFAL_URL_MAX_LEN);
		}	
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return resu;
}

/**
 *  load the gfal_plugins in the listed library
 */ 
static int gfal_module_load(gfal_handle handle, char* module_name, GError** err){
	void* dlhandle = dlopen(module_name, RTLD_LAZY);
	GError * tmp_err=NULL;
	int ret = -1;
	if (dlhandle==NULL)
		g_set_error(&tmp_err, 0, EINVAL, "[%s] Unable to open the %s plugin specified in the %s en var, failure : %s", __func__, module_name, GFAL_PLUGIN_ENVAR, dlerror());
	else
		ret = gfal_module_init(handle, dlhandle, module_name, &tmp_err);	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}

/**
 * 	search the plugin list for gfal
 *  tab must be free with g_strfreev
 */
static char** gfal_search_plugin_list(GError** err){
	char* gfal_plugin_lst = getenv(GFAL_PLUGIN_ENVAR);
	if( gfal_plugin_lst == NULL ){
		g_set_error(err, 0, EINVAL, "[%s] env var %s not defined, no plugin loaded for gfal !", __func__, GFAL_PLUGIN_ENVAR);
		return NULL;		
	}
	int n = strnlen(gfal_plugin_lst, GFAL_MAX_PLUGIN_LIST);	
	if(n >= GFAL_MAX_PLUGIN_LIST){
		g_set_error(err, 0, ENAMETOOLONG, "[%s] plugin list in %s env var too long", __func__, GFAL_PLUGIN_ENVAR);
		return NULL;
	}
	char** str_tab=  g_strsplit(gfal_plugin_lst, ":", 0);	
	if( *str_tab != NULL )
		return str_tab;
	return NULL;
}

/**
 *  search and load the modules of gfal in checking the GFAL_PLUGIN_ENVAR
 */
static int gfal_modules_resolve(gfal_handle handle, GError** err){
	GError* tmp_err = NULL;
	int ret=-1;
	char** tab_args;
	
	if( (tab_args = gfal_search_plugin_list(&tmp_err)) != NULL){
		char** p= tab_args;
		while(*p  != NULL ){
			if(**p=='\0')
				break;
			if( gfal_module_load(handle, *p, &tmp_err) != 0){
				ret = -1;
				break;
			}
			gfal_print_verbose(GFAL_VERBOSE_VERBOSE, " gfal_plugin loaded succesfully : %s", *p);
			ret =0;
			p++;
		}
		g_strfreev(tab_args);
	}
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}

/**
 * Instance all plugins for use if it's not the case
 *  return the number of plugin available
 */
inline int gfal_plugins_instance(gfal_handle handle, GError** err){
	g_return_val_err_if_fail(handle, -1, err, "[gfal_plugins_instance]  invalid value of handle");
	const int plugin_number = handle->plugin_opt.plugin_number;
	int ret;
	if(plugin_number <= 0){
		GError* tmp_err=NULL;
		ret = gfal_modules_resolve(handle, &tmp_err);
		if(tmp_err){
			g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
			handle->plugin_opt.plugin_number = -1;
		}
		return handle->plugin_opt.plugin_number;	
	}
	return plugin_number;
}
/***
 *  generic plugin operation executor
 *  Check alls plugins, if a plugin is valid execute the given operation on this plugin and return result, 
 *  else return nagative value and set GError to the correct error
 * */
 int gfal_plugins_operation_executor(gfal_handle handle, gboolean (*checker)(gfal_plugin_interface*, GError**),
										int (*executor)(gfal_plugin_interface*, GError**) , GError** err){
	GError* tmp_err=NULL;
	int i;
	int ret = -1;
	const int n_plugins = gfal_plugins_instance(handle, &tmp_err);
	if(n_plugins > 0){
		gfal_plugin_interface* cata_list = handle->plugin_opt.plugin_list;
		for(i=0; i < n_plugins; ++i, ++cata_list){
			const gboolean comp =  checker(cata_list, &tmp_err);
			if(tmp_err)
				break;
				
			if(comp){
				ret = executor(cata_list, &tmp_err);
				break;		
			}	
		}
	}
	if(tmp_err){	
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	}else if(ret){
		g_set_error(err,0,EPROTONOSUPPORT, "[%s] Protocol not supported or path/url invalid", __func__);
	}
	return ret;
	 
 }
 
/**
 *  Execute an access function on the first plugin compatible in the plugin list
 *  return the result of the first valid plugin for a given URL
 *  @return result of the access method or -1 if error and set GError with the correct value
 *  error : EPROTONOSUPPORT means that the URL is not matched by a plugin
 *  */
int gfal_plugins_accessG(gfal_handle handle, const char* path, int mode, GError** err){
	g_return_val_err_if_fail(handle && path, EINVAL, err, "[gfal_plugins_accessG] Invalid arguments");
	GError* tmp_err=NULL;
	
	gboolean access_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_ACCESS, terr);	
	}
	int access_executor(gfal_plugin_interface* cata_list, GError** terr){
		return cata_list->accessG(cata_list->handle, path, mode, terr);
	}
	
	int ret = gfal_plugins_operation_executor(handle, &access_checker, &access_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}

/**
 *  Execute a stat function on the lfc plugin
 * */
int gfal_plugin_statG(gfal_handle handle, const char* path, struct stat* st, GError** err){
	g_return_val_err_if_fail(handle && path, EINVAL, err, "[gfal_plugin_statG] Invalid arguments");
	GError* tmp_err=NULL;
	
	gboolean stat_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_STAT, terr);
	}
	int stat_executor(gfal_plugin_interface* cata_list, GError** terr){
		return cata_list->statG(cata_list->handle, path, st, terr);
	}
	
	int ret = gfal_plugins_operation_executor(handle, &stat_checker, &stat_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__); 
	return ret;	
}

/**
 *  Execute a readlink function
 * */
ssize_t gfal_plugin_readlinkG(gfal_handle handle, const char* path, char* buff, size_t buffsiz, GError** err){
	g_return_val_err_if_fail(handle && path, EINVAL, err, "[gfal_plugin_readlinkG] Invalid arguments");
	GError* tmp_err=NULL;
	ssize_t resu;
	
	gboolean readlink_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_READLINK, terr);
	}
	int readlink_executor(gfal_plugin_interface* cata_list, GError** terr){
		return ((resu = cata_list->readlinkG(cata_list->handle, path, buff, buffsiz, terr)) !=-1)?0:-1;
	}
	
	gfal_plugins_operation_executor(handle, &readlink_checker, &readlink_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__); 
	return resu;	
}



/**
 *  Execute a lstat function in the lfc
 * */
int gfal_plugin_lstatG(gfal_handle handle, const char* path, struct stat* st, GError** err){
	g_return_val_err_if_fail(handle && path, EINVAL, err, "[gfal_plugin_lstatG] Invalid arguments");
	GError* tmp_err=NULL;
	
	gboolean lstat_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_LSTAT, terr);
	}
	int lstat_executor(gfal_plugin_interface* cata_list, GError** terr){
		return cata_list->lstatG(cata_list->handle, path, st, terr);
	}
	
	int ret = gfal_plugins_operation_executor(handle, &lstat_checker, &lstat_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__); 
	return ret;	
}

/**
 * Delete all instance of plugins 
 */
int gfal_plugins_delete(gfal_handle handle, GError** err){
	g_return_val_err_if_fail(handle, -1, err, "[gfal_plugins_delete] Invalid value of handle");
	const int plugin_number = handle->plugin_opt.plugin_number;
	if(plugin_number > 0){
			int i;
			for(i=0; i< plugin_number; ++i){
				if(handle->plugin_opt.plugin_list[i].plugin_delete)
					handle->plugin_opt.plugin_list[i].plugin_delete( handle->plugin_opt.plugin_list[i].handle );
			}
		
		handle->plugin_opt.plugin_number =0;
	}
	return 0;
}
/**
 *  Execute the chmod function on the first compatible plugin ( checked with check_url func )
 *  @return 0 if success or -1 and set the GError to the correct errno value with a description msg
 * */
 int gfal_plugin_chmodG(gfal_handle handle, const char* path, mode_t mode, GError** err){
	g_return_val_err_if_fail(handle && path, -1, err, "[gfal_plugin_chmodG] Invalid arguments");	
	GError* tmp_err = NULL;		
	
		
	gboolean chmod_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_CHMOD, terr);	
	}
	int chmod_executor(gfal_plugin_interface* cata_list, GError** terr){
		return cata_list->chmodG(cata_list->handle, path, mode, terr);
	}
	
	int ret = gfal_plugins_operation_executor(handle, &chmod_checker, &chmod_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret; 
 }
 
 /**
 *  Execute the rename function on the first compatible plugin ( checked with check_url func )
 *  @return 0 if success or -1 and set the GError to the correct errno value with a description msg
 * */
int gfal_plugin_renameG(gfal_handle handle, const char* oldpath, const char* newpath, GError** err){
	g_return_val_err_if_fail(oldpath && newpath, -1, err, "[gfal_plugin_renameG] invalid value in args oldpath, handle or newpath");
	GError* tmp_err=NULL;
	
	gboolean rename_checker(gfal_plugin_interface* cata_list, GError** terr){
		return (gfal_plugin_checker_safe(cata_list, oldpath, GFAL_PLUGIN_RENAME, terr) && 
			gfal_plugin_checker_safe(cata_list, newpath, GFAL_PLUGIN_RENAME, terr));	
	}
	int rename_executor(gfal_plugin_interface* cata_list, GError** terr){
		return cata_list->renameG(cata_list->handle, oldpath, newpath, terr);
	}
	
	int ret = gfal_plugins_operation_executor(handle, &rename_checker, &rename_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret; 
	
}

/**
 * Execute the symlink function on the first compatible plugin
 */
int gfal_plugin_symlinkG(gfal_handle handle, const char* oldpath, const char* newpath, GError** err){
	g_return_val_err_if_fail(oldpath && newpath, -1, err, "[gfal_plugin_symlinkG] invalid value in args oldpath, handle or newpath");
	GError* tmp_err=NULL;
	
	gboolean symlink_checker(gfal_plugin_interface* cata_list, GError** terr){
		return (gfal_plugin_checker_safe(cata_list, oldpath, GFAL_PLUGIN_SYMLINK, terr) && 
			gfal_plugin_checker_safe(cata_list, newpath, GFAL_PLUGIN_SYMLINK, terr));	
	}
	int symlink_executor(gfal_plugin_interface* cata_list, GError** terr){
		return cata_list->symlinkG(cata_list->handle, oldpath, newpath, terr);
	}
	
	const int ret = gfal_plugins_operation_executor(handle, &symlink_checker, &symlink_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret; 	
	
}

/**
 * Execute a mkdir function on the first compatible plugin ( checked with check url func )
 *  @param handle handle of the current context
 *  @param path path to create
 *  @param mode right of the file created
 *  @param pflag if TRUE, execute the request recursively if necessary else work as the common mkdir system call
 *  @param GError error report system
 *  @warning no check on the path, please check the path before
 *  @return return 0 if success else return -1
 * 
 * */
int gfal_plugin_mkdirp(gfal_handle handle, const char* path, mode_t mode, gboolean pflag,  GError** err){
	g_return_val_err_if_fail(handle && path , -1, err, "[gfal_plugin_mkdirp] Invalid argumetns in path or/and handle");
	GError* tmp_err=NULL;	

	gboolean mkdirp_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_MKDIR, terr);	
	}
	int mkdirp_executor(gfal_plugin_interface* cata_list, GError** terr){
		return cata_list->mkdirpG(cata_list->handle, path, mode, pflag, terr);
	}

	int ret = gfal_plugins_operation_executor(handle, &mkdirp_checker, &mkdirp_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret; 	
}

/**
 * Execute a rmdir function on the first compatible plugin ( checked with check url func )
 *  @param handle handle of the current context
 *  @param path path to delete
 *  @param GError error report system
 *  @warning no check on the path, please check the path before
 *  @return return 0 if success else return -1
 * 
 * */
int gfal_plugin_rmdirG(gfal_handle handle, const char* path, GError** err){
	g_return_val_err_if_fail(handle && path , -1, err, "[gfal_plugin_rmdirp] Invalid argumetns in path or/and handle");
	GError* tmp_err=NULL;	

	gboolean rmdir_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_RMDIR, terr);	
	}
	int rmdir_executor(gfal_plugin_interface* cata_list, GError** terr){
		return cata_list->rmdirG(cata_list->handle, path, terr);
	}

	int ret = gfal_plugins_operation_executor(handle, &rmdir_checker, &rmdir_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret; 	
}

/**
 * Execute a opendir function on the first compatible plugin ( checked with check url func )
 * @param handle handle of the current context
 * @param path path to open
 * @param GError error report system
 * @return gfal_file_handle pointer given to the handle or NULL if error 
 */
 gfal_file_handle gfal_plugin_opendirG(gfal_handle handle, const char* name, GError** err){
	g_return_val_err_if_fail(handle && name, NULL, err,  "[gfal_plugin_opendir] invalid value");
	GError* tmp_err=NULL;	
	gfal_file_handle resu=NULL;
	
	gboolean opendir_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, name, GFAL_PLUGIN_OPENDIR, terr);	
	}
	int opendir_executor(gfal_plugin_interface* cata_list, GError** terr){
		resu= cata_list->opendirG(cata_list->handle, name, terr);
		return (resu)?0:-1;
	}
	
	gfal_plugins_operation_executor(handle, &opendir_checker, &opendir_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return resu;  
}

/**
 * 
 * close the given dir handle in the proper plugin
 */ 
int gfal_plugin_closedirG(gfal_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail(handle && fh, -1,err, "[gfal_plugin_closedirG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;
	gfal_plugin_interface* if_cata = gfal_plugin_getModuleFromHandle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->closedirG(if_cata->handle, fh, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret;  	
}

/**
 * 
 *  open the file specified by path on the proper plugin with the specified flag and mode
 * */
gfal_file_handle gfal_plugin_openG(gfal_handle handle, const char * path, int flag, mode_t mode, GError ** err){
	GError* tmp_err=NULL;
	int ret =-1;
	gfal_file_handle resu =NULL;
	gfal_print_verbose(GFAL_VERBOSE_TRACE, " %s ->",__func__);

	
	gboolean openG_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_OPEN, terr);
	}	
	int openG_executor(gfal_plugin_interface* cata_list, GError** terr){
		resu = cata_list->openG(cata_list->handle, path, flag, mode, terr);
		return (resu)?0:-1;
	}	
	
	ret = gfal_plugins_operation_executor(handle, &openG_checker, &openG_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);	
	return resu;
}

/**
 *  close the given file handle in the proper plugin
 * */
int gfal_plugin_closeG(gfal_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail(handle && fh, -1,err, "[gfal_plugin_closeG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;
	gfal_plugin_interface* if_cata = gfal_plugin_getModuleFromHandle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->closeG(if_cata->handle, fh, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret;  	
}

/**
 *  execute a readdir for the given file handle on the appropriate plugin
 * 
 * */
struct dirent* gfal_plugin_readdirG(gfal_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail(handle && fh, NULL,err, "[gfal_plugin_readdirG] Invalid args ");	
	GError* tmp_err=NULL;
	struct dirent* ret = NULL;
	gfal_plugin_interface* if_cata = gfal_plugin_getModuleFromHandle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->readdirG(if_cata->handle, fh, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret; 
}



/**
 * @brief implementation in the plugin of the get extended attribute function
 * */
ssize_t gfal_plugin_getxattrG(gfal_handle handle, const char* path, const char*name, void* buff, size_t s_buff, GError** err){
	GError* tmp_err=NULL;
	ssize_t resu = -1;
	
	gboolean getxattr_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_GETXATTR, terr);
	}	
	int getxattr_executor(gfal_plugin_interface* cata_list, GError** terr){
		resu= cata_list->getxattrG(cata_list->handle, path, name, buff, s_buff, terr);
		return (int)resu;
	}
	
	gfal_plugins_operation_executor(handle, &getxattr_checker, &getxattr_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);	
	return resu;	
	
}

ssize_t gfal_plugin_listxattrG(gfal_handle handle, const char* path, char* list, size_t s_list, GError** err){
	GError* tmp_err=NULL;
	ssize_t resu = -1;
	
	gboolean listxattr_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_LISTXATTR, terr);
	}	
	int listxattr_executor(gfal_plugin_interface* cata_list, GError** terr){
		resu= cata_list->listxattrG(cata_list->handle, path, list, s_list, terr);
		return (int)resu;
	}
	
	gfal_plugins_operation_executor(handle, &listxattr_checker, &listxattr_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);	
	return resu;		 
}



/**
 * do a read operation on the plugin, read s_buff chars on the fd device
 * @return return number of bytes readed else -1 if errors and GError is set
 * 
 * */
int gfal_plugin_readG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
	g_return_val_err_if_fail(handle && fh && buff && s_buff> 0, -1,err, "[gfal_plugin_readG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;
	gfal_plugin_interface* if_cata = gfal_plugin_getModuleFromHandle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->readG(if_cata->handle, fh, buff, s_buff,  &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret; 	
}

/**
 * do a lseek operation on the plugin
 * @return return number of bytes readed else -1 if errors and GError is set
 * 
 * */
int gfal_plugin_lseekG(gfal_handle handle, gfal_file_handle fh, off_t offset, int whence, GError** err){
	g_return_val_err_if_fail(handle && fh , -1,err, "[gfal_plugin_lseekG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;
	gfal_plugin_interface* if_cata = gfal_plugin_getModuleFromHandle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->lseekG(if_cata->handle, fh, offset, whence,  &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret;	
	
}

/**
 * do a write operation on the plugin, write s_buff chars on the fd device
 * @return return number of bytes readed else -1 if errors and GError is set
 * 
 * */
int gfal_plugin_writeG(gfal_handle handle, gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
	g_return_val_err_if_fail(handle && fh && buff && s_buff> 0, -1,err, "[gfal_plugin_writeG] Invalid args ");	
	GError* tmp_err=NULL;
	int ret = -1;
	gfal_plugin_interface* if_cata = gfal_plugin_getModuleFromHandle(handle, fh, &tmp_err);
	if(!tmp_err)
		ret = if_cata->writeG(if_cata->handle, fh,buff, s_buff, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret; 	
}






/**
 * return the plugin type configured at compilation time
 */
static char* get_default_cat(){
	return GFAL_DEFAULT_PLUGIN_TYPE;
}

/***
 *  return the name of the current selected default plugin in a string form
 * */
extern char* gfal_get_cat_type(GError** err) {
    char *cat_env;
    char *cat_type;

    if((cat_env = getenv ("LCG_PLUGIN_TYPE")) == NULL) {
		gfal_print_verbose(GFAL_VERBOSE_VERBOSE, "[get_cat_type] LCG_PLUGIN_TYPE env var is not defined, use default var instead");
        cat_env = get_default_cat(); 
	}
    if((cat_type = strndup(cat_env, 50)) == NULL) {
		g_set_error(err,0,EINVAL,"[%s] invalid env var LCG_PLUGIN_TYPE, please set it correctly or delete it",__func__);
        return NULL;
    }
    return cat_type;
}





/**
 *  apply unlink on the appropriate plugin
 * */
int gfal_plugin_unlinkG(gfal_handle handle, const char* path, GError** err){
	GError* tmp_err=NULL;
	int resu = -1;
	
	gboolean unlink_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_UNLINK, terr);
	}	
	int unlink_executor(gfal_plugin_interface* cata_list, GError** terr){
		resu= cata_list->unlinkG(cata_list->handle, path, terr);
		return resu;
	}
	
	gfal_plugins_operation_executor(handle, &unlink_checker, &unlink_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);	
	return resu;	
	
}

/**
 * setxattr for the plugins
 * */
int gfal_plugin_setxattrG(gfal_handle handle, const char* path, const char* name, const void* value, size_t size, int flags, GError** err){
	GError* tmp_err=NULL;
	int resu = -1;
	
	gboolean setxattrG_checker(gfal_plugin_interface* cata_list, GError** terr){
		return gfal_plugin_checker_safe(cata_list, path, GFAL_PLUGIN_SETXATTR, terr);		
	}
	int setxattrG_executor(gfal_plugin_interface* cata_list, GError** terr){
		if(cata_list->setxattrG)
			return cata_list->setxattrG(cata_list->handle, path, name, value, size, flags, terr);
		else{
			g_set_error(terr,0, EPROTONOSUPPORT, "unexcepted NULL pointer for the setxattrG call of the %s plugin", cata_list->getName());
			return -1;
		}
	}

	resu = gfal_plugins_operation_executor(handle, &setxattrG_checker, &setxattrG_executor, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);	
	return resu;		
}

