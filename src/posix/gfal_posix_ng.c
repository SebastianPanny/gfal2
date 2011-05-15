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
 * @file gfal_posix_ng.c
 * @brief main file of the posix lib ng
 * @author Devresse Adrien
 * @version 2.0
 * @date 09/05/2011
 * */


#include "../common/gfal_constants.h" 
#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"




/**
 * \brief test access to the given file
 * \param file can be in supported protocols lfn, srm, file, guid
 * \return This routine return 0 if the operation was successful, or -1 if error occured and errno is set, call \ref gfal_posix_print_error() to check it. \n
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Search permission is denied on a component of the path prefix or specified access to the file itself is denied.
 *   		- EFAULT: path is a NULL pointer.
 *   		- ENOTDIR: A component of path prefix is not a directory.
 *  	- gfal errors ( associated with a specific gfal_posix_print_error() ):
 *   		- ECOMM: Communication error
 *   		- EPROTONOSUPPORT: Access method not supported.
 *   		- EINVAL: path has an invalid syntax or amode is invalid.
 * 
 */
int gfal_access (const char *path, int amode){
	return gfal_posix_internal_access(path, amode);	
}

/**
 * @brief change the right for a file or a folder
 * @param path : path of the file or the folder concerned : must be a Catalog URL ( lfn: ), a guid,  or a local file URL ( file: )
 * @param mode : right to configure
 * @return return 0 if success else -1 and errno is set, call \ref gfal_posix_print_error() to check it
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Search permission is denied on a component of the path prefix or specified access to the file itself is denied.
 *   		- EFAULT: path is a NULL pointer.
 *   		- ENOTDIR: A component of path prefix is not a directory.
 *  	- gfal errors ( associated with a specific gfal_posix_print_error() ):
 *   		- ECOMM: Communication error
 *   		- EPROTONOSUPPORT: Access method not supported.
 *   		- EINVAL: path has an invalid syntax or amode is invalid. 		
 */
int gfal_chmod(const char* path, mode_t mode){
	return gfal_posix_internal_chmod(path, mode);
}

/**
 * @brief  change the name or location of a file
 * oldpath and newpath need to be on the same catalog
 * this functions work only with catalogs (lfc ) and local files
 * @param oldpath : the old path of the file
 * @param newpath : the new path of the file
 * @return : return 0 if success, else -1 and errno / \ref gfal_posix_error_print()
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Write perimission is denied for newpath or oldpath, or, search permission is denied for one of the directories in the path prefix of oldpath or newpath,
              or oldpath is a directory and does not allow write permission (needed to update the ..  entry)
 *   		- EFAULT: oldpath or newpath is a NULL pointer.
 * 			- EISDIR: newpath is an existing directory but oldpath is not a directory
 * 			- ENOTEMPTY or EEXIST : newpath is a nonempty directory, that is, contains entries other than "." and ".."
 *   		- ENOTDIR: A component of path prefix is not a directory.
 * 			- EXDEV  oldpath  and  newpath  are  not on the same mounted file system. 
 *  	- gfal errors ( associated with a specific gfal_posix_print_error() ):
 *   		- ECOMM: Communication error
 *   		- EPROTONOSUPPORT: oldpath or newpath has a syntax error or the protocol speficied is not supported
 *   		- EINVAL: oldpath or newpath has an invalid syntax .
 * 
*/
int gfal_rename(const char *oldpath, const char *newpath){
	return gfal_posix_internal_rename(oldpath, newpath);
}

/**
 *  @brief  informations about a file 
 * These functions return information about a file.  No permissions are required on the file itself, but — in the case of stat() and lstat() — execute (search) permission is
 *     required on all of the directories in path that lead to the file.
 * @param path : path of a file. Can be a SURL, a Catalog URL or a guid
 * @param buff : pointer to an allocated struct stat
 * @return return 0 if success else -1 and errno is set ( and gfal_posix_print_error() )
 * 
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Write perimission is denied for newpath or oldpath, or, search permission is denied for one of the directories in the path prefix of oldpath or newpath,
              or oldpath is a directory and does not allow write permission (needed to update the ..  entry)
 *   		- EFAULT: oldpath or newpath is a NULL pointer.
 *   		- ENOTDIR: A component of path prefix is not a directory.
 *  	- gfal errors ( associated with a specific gfal_posix_print_error() ):
 *   		- ECOMM: Communication error
 *   		- EPROTONOSUPPORT: oldpath or newpath has a syntax error or the protocol speficied is not supported
 *   		- EINVAL: oldpath or newpath has an invalid syntax .
 * */
int gfal_stat(const char* path, struct stat* buf){
	return gfal_posix_internal_stat(path, buf);
}

/**
 * Display the last string error reported by the gfal error system for the posix API
 * Errors are printed on stderr
 */
void gfal_posix_print_error(){
	gfal_handle handle;
	GError* err=NULL;
	if((handle = gfal_posix_instance()) == NULL){
		g_printerr("[gfal] Initialisation error gfal_posix_instance() failure\n");
	}else if ( (err = handle->err) != NULL){
		g_printerr("[gfal]%s \n", err->message);
	}else if(errno !=0){
		char* sterr = strerror(errno);
		g_printerr("[gfal] errno reported by external lib : %s", sterr);
	}else{
		g_printerr("[gfal] No gfal error reported\n");
	}
}

/**
 * Display and clear the last string error reported by the gfal error system for the posix API and 
 * similar to a gfal_posix_print_error() and a gfal_posix_clear_error()
 */
void gfal_posix_release_error(){
	gfal_posix_print_error();
	gfal_posix_clear_error();
}


/**
 * clear the last error reported by a gfal posix function
 */
void gfal_posix_clear_error(){
	gfal_handle handle;
	char* msg;
	if((handle = gfal_posix_instance()) == NULL){
		g_printerr("[gfal][gfal_posix_clear_error] Initialisation error gfal_posix_instance() failure\n");
	}else{
		g_clear_error( &(handle->err));
		errno =0;
	}	

}

int gfal_posix_code_error(){
	gfal_handle handle;
	GError* err=NULL;
	int ret = -1;
	if((handle = gfal_posix_instance()) == NULL){
		g_printerr("[gfal_posix_code_error] Initialisation error gfal_posix_instance() failure\n");
	}else  {
		ret = ((err = handle->err) != NULL)? err->code :0 ;
	}
	return ret;
}

