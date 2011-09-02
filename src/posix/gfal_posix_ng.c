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
 * @brief new file for the posix interface
 * @author Devresse Adrien
 * @date 09/05/2011
 * */


#include "../common/gfal_constants.h" 
#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"


static int nobdii = 0;




/**
 * \brief test access to the given file
 * \param path path of the file to access, can be in supported protocols (lfn, srm, file, guid,..)
 * \param amode access mode to check (R_OK, W_OK, X_OK or F_OK)
 * \return This routine return 0 if the operation was successful, or -1 if error occured and errno is set, call @ref gfal_posix_check_error() to check it. \n
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Search permission is denied on a component of the path prefix or specified access to the file itself is denied.
 *   		- EFAULT: path is a NULL pointer.
 *   		- ENOTDIR: A component of path prefix is not a directory.
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
 * @param path : path of the file or the folder, can be in all supported protocols (lfn, srm, file, guid,..)
 * @param mode : right to configure
 * @return return 0 if success else -1 and errno is set, call @ref gfal_posix_check_error() to check it
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Search permission is denied on a component of the path prefix or specified access to the file itself is denied.
 *   		- EFAULT: path is a NULL pointer.
 *   		- ENOTDIR: A component of path prefix is not a directory.
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
 * @param oldpath : the old path of the file, can be in supported protocols but need to be in the same adress space than newpath
 * @param newpath : the new path of the file, can be in supported protocols (lfn, srm, file, guid,..)
 * @return : return 0 if success, else -1 and errno / @ref gfal_posix_check_error()
 *  - ERRNO list : \n
 *			- ENOENT: The named file/directory does not exist.
 *			- EACCES: Write perimission is denied for newpath or oldpath, or, search permission is denied for one of the directories in the path prefix of oldpath or newpath,
              or oldpath is a directory and does not allow write permission (needed to update the ..  entry)
 *			- EFAULT: oldpath or newpath is a NULL pointer
 *			- EISDIR: newpath is an existing directory but oldpath is not a directory
 *			- ENOTEMPTY or EEXIST : newpath is a nonempty directory, that is, contains entries other than "." and ".."
 *			- ENOTDIR: A component of path prefix is not a directory
 *			- EXDEV  oldpath  and  newpath  are  not on the same mounted file system
 *			- ECOMM: Communication error
 *			- EPROTONOSUPPORT: oldpath or newpath has a syntax error or the protocol speficied is not supported
 *			- EINVAL: oldpath or newpath has an invalid syntax
 *  
*/
int gfal_rename(const char *oldpath, const char *newpath){
	return gfal_posix_internal_rename(oldpath, newpath);
}

/**
 *  @brief  informations about a file 
 * These functions return information about a file.  No permissions are required on the file itself, but — in the case of stat() and lstat() — execute (search) permission is
 *     required on all of the directories in path that lead to the file.
 * @param path : path of the file, can be in supported protocols (lfn, srm, file, guid,..)
 * @param buff : pointer to an allocated struct stat
 * @return return 0 if success else -1 and errno is set, call @ref gfal_posix_check_error() to check it
 * 
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Write perimission is denied for newpath or oldpath, or, search permission is denied for one of the directories in the path prefix of oldpath or newpath,
              or oldpath is a directory and does not allow write permission (needed to update the ..  entry)
 *   		- EFAULT: oldpath or newpath is a NULL pointer.
 *   		- ENOTDIR: A component of path prefix is not a directory.
 *   		- ECOMM: Communication error
 *   		- EPROTONOSUPPORT: oldpath or newpath has a syntax error or the protocol speficied is not supported
 *   		- EINVAL: oldpath or newpath has an invalid syntax .
 * */
int gfal_stat(const char* path, struct stat* buff){
	return gfal_posix_internal_stat(path, buff);
}

/**
 * @brief gfal_lstat is identical to \ref gfal_stat except for symbolic links. 
 * In this case, the link itself is statted and not
       followed.
*/
int gfal_lstat(const char* path, struct stat* buff){
	return gfal_posix_internal_lstat(path, buff);
}
/**
 * @brief  create a new directory
 * creates a new directory with permission bits taken from mode.
 *  The default behavior of this command is recursive, like "mkdir -p".
 * @param path : url of the directory, can be in supported protocols (lfn, srm, file, guid,..)
 * @param mode : right of the directory ( depend of the implementation )
 * @return return 0 if success else -1 and errno is set call @ref gfal_posix_check_error() to check it
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Write perimission is denied for newpath or oldpath, or, search permission is denied for one of the directories in the path prefix of oldpath or newpath,
 *            or oldpath is a directory and does not allow write permission (needed to update the ..  entry)
 *   		- EFAULT: oldpath or newpath is a NULL pointer
 *   		- ENOTDIR: A component of path prefix is not a directory  
 * 			- EEXIST: file already exist  
 *   		- ECOMM: Communication error
 *   		- EPROTONOSUPPORT: oldpath or newpath has a syntax error or the protocol speficied is not supported
 *   		- EINVAL: oldpath or newpath has an invalid syntax .
 */
int gfal_mkdirp( const char* path, mode_t mode){
	return  gfal_posix_internal_mkdir( path, mode);
	
}
/**
 * Wrapper to mkdir for comptibility, same behavior than \ref gfal_mkdirp ( but subject to change in order to follow POSIX mkdir in the futur )
 */
int gfal_mkdir( const char* path, mode_t mode){
	return  gfal_mkdirp( path, mode);
	
}

/**
 * @brief  removes a directory if it is empty
 * remove an existing directory, return error if the dir is not empty
 *  @param path specifies the directory name, can be in supported protocols (lfn, srm, file, guid,..)
 *  @return return 0 is success else -1 and errno is set call @ref gfal_posix_check_error() to check it
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Write perimission is denied for path, or, search permission is denied for one of the directories in the path prefix of oldpath or newpath,
              or oldpath is a directory and does not allow write permission (needed to update the ..  entry)
 *   		- EFAULT: path is a NULL pointer.
 * 			- ENOTEMPTY : path is a nonempty directory, that is, contains entries other than "." and ".."
 *   		- ENOTDIR: A component of path prefix is not a directory 
 *   		- ECOMM: Communication error
 *   		- EPROTONOSUPPORT: path has a syntax error or the protocol speficied is not supported
 *   		- EINVAL: path has an invalid syntax .
 * 
 * */
int gfal_rmdir(const char* path){
	return gfal_posix_internal_rmdir(path);
}

/**
 * @brief  open a directory
 * 
 * opens a directory to be used in subsequent gfal_readdir operations
 * the url supported are : local files, surls, catalog url ( lfc,...)
 * @param name of the directory to open, can be in supported protocols (lfn, srm, file, guid,..)
 * @return file descriptor DIR* if success else NULL if error and errno is set call @ref gfal_posix_check_error() to check it
 * 
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- ENOENT: The named file/directory does not exist.
 *    		- EACCES: Write perimission is denied for path, or, search permission is denied for one of the directories in the path prefix of oldpath or newpath,
              or oldpath is a directory and does not allow write permission (needed to update the ..  entry)
 *   		- EFAULT: path is a NULL pointer.
 * 			- ENOTEMPTY : newpath is a nonempty directory, that is, contains entries other than "." and ".."
 *   		- ENOTDIR:  path or a component in path is not a directory 
 * 			- EMFILE: too many file open by the process
 *   		- ECOMM: Communication error
 *   		- EPROTONOSUPPORT: path has a syntax error or the protocol speficied is not supported
 *   		- EINVAL: path has an invalid syntax .
 * */
DIR* gfal_opendir(const char* name){
	return gfal_posix_internal_opendir(name);
}

/**
 * @brief  read a directory
 * 
 * The readdir() function returns a pointer to a dirent structure representing the next directory entry in the directory stream pointed to by dirp.  It returns NULL on
 *      reaching the end of the directory stream or if an error occurred.
 *            struct dirent {
 *             ino_t          d_ino;       // inode number 
 *             off_t          d_off;       // offset to the next dirent 
 *             unsigned short d_reclen;    // length of this record 
 *             unsigned char  d_type;      // type of file; not supported
 *                                         //  by all file system types 
 *             char           d_name[256]; // filename 
 *         };
 *
 * 
 * @param d file handle ( return by opendir ) to read
 * @return pointer to struct dirent with file information or NULL if end of list or error, errno is set call @ref gfal_posix_check_error() to check it
 * @warning struct dirents are allocated statically, do not use free() on them
 * 
 * 
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- EBADF : bad file descriptor
 * 			- ECOMM : Communication error
 * */
struct dirent* gfal_readdir(DIR* d){
	return gfal_posix_internal_readdir(d);	
}

/**
 * @brief  read a directory
 *  close the file descriptor of an opendir call
 * 
 * @param d file handle ( return by opendir ) to close
 * @return 0 if success else negativevalue and errno is set (  ( gfal_posix_error_print() )
 * 
 *   - ERRNO list : \n 
 *		- EBADF: bad file descriptor
 *		- ECOMM: Communication error  
 * 
 * 
 * */
int gfal_closedir(DIR* d){
	return gfal_posix_internal_closedir(d);
}

/**
 *  @brief open a file
 * 	opens a file according to the value of flags.
 *  @param path : url of the filename to open. can be in supported protocols (lfn, srm, file, guid,..)
 *  @param flag : same flag supported value is built by OR’ing the bits defined in <fcntl.h> but one and only one of the first three flags below must be used
 *            O_RDONLY    open for reading only
 *            O_WRONLY    open for writing only
 *            O_RDWR      open for reading and writing
 *            O_CREAT     If the file exists already and O_EXCL is also set, gfal_open will fail
 *            O_LARGEFILE allows files whose sizes cannot be represented in 31 bits to be opened
 *  @param mode is used only if the file is created.
 *  @return return the file descriptor or -1 if errno is set call @ref gfal_posix_check_error() to check it
 * */
int gfal_open(const char * path, int flag, mode_t mode){
	return gfal_posix_internal_open(path, flag, mode);
}



/**
 *  @brief  create a new file or truncate an existing one
 * 	opens a file according to the value of flags.
 *  @param filename : url of the filename to create, can be in supported protocols (lfn, srm, file, guid,..)
 *  @param mode is used only if the file is created.
 *  @return return the file descriptor or -1 if errno is set call @ref gfal_posix_check_error() to check it
 * */
int gfal_creat (const char *filename, mode_t mode){
    return (gfal_open (filename, O_WRONLY|O_CREAT|O_TRUNC, mode));
}

/**
 *  @brief read a file
 * 	gfal_read reads up to size bytes from the file descriptor fd into the buffer pointed by buff
 *  @param fd file descriptor
 *  @param buff buffer of the data to read
 *  @param s_buff size of the data read in bytes
 *  @return number of byte read or -1 if error, errno is set call @ref gfal_posix_check_error() to check it 
 */
int gfal_read(int fd, void* buff, size_t s_buff){
	return gfal_posix_internal_read(fd, buff, s_buff);
}
/**
 *  @brief write a file
 * gfal_write writes size bytes from the buffer pointed by buff to the file descriptor fd.
 *  @param fd file descriptor
 *  @param buff buffer of the data to write
 *  @param s_buff size of the data write in bytes
 *  @return number of byte write or -1 if error, errno is set call @ref gfal_posix_check_error() to check it   
 */
int gfal_write(int fd, const void *buff, size_t s_buff){
	return gfal_posix_internal_write(fd, (void*) buff, s_buff);
}

/**
 *  @brief  close a file
 * 	closes the file whose descriptor fd is the one returned by gfal_open.
 *  @param fd : descriptor or the file given by @ref gfal_open
 *  @return This routine returns 0 if the operation was successful or -1 if the operation failed. In the latter case, errno is set appropriately
 *  - ERRNO list : \n
 *    	- usual errors:
 *    		- EBADF:  fd is not a valid file descriptor
 *   		- ECOMM: Communication error
 *   		- EPROTONOSUPPORT: path has a syntax error or the protocol speficied is not supported
 * */
int gfal_close(int fd){
	return gfal_posix_internal_close(fd);
}

/**
 * @brief make a new name for a file
 *  symlink() creates a symbolic link named newpath which contains the string oldpath.
 * @param newpath : path of the link, can be in supported protocols but need to be in the same adress space than newpath
 * @param oldpath : path of the linked file, can be in supported protocols (lfn, srm, file, guid,..)
 * @return 0 if success else -1.  if failure, errno is set, you can call @ref gfal_posix_check_error() for a more complete description. 
*/
int gfal_symlink(const char* oldpath, const char * newpath){
	return gfal_posix_internal_symlink(oldpath, newpath);
}

/**
 * @brief set position in a file 
 *      gfal_lseek  positions/repositions  to  offset  the file associated with the descriptor fd generated by a previous gfal_open.  whence indicates how to interpret the offset
 *     value:
 *
 *            SEEK_SET     The offset is set from beginning of file.
 *
 *            SEEK_CUR     The offset is added to current position.
 *
 *            SEEK_END     The offset is added to current file size.
 * @param fd : file descriptor to lseek
 * @param offset: offset in byte
 * @param whence:  flag
 * @return  This routine returns the actual offset from the beginning of the file if the operation was successful or -1 if the operation failed. In the  latter  case,  errno  is  set
       appropriately, you can call @ref gfal_posix_check_error() for a more complete description. 
*/
off_t gfal_lseek (int fd, off_t offset, int whence){
	return gfal_posix_internal_lseek(fd, offset, whence);
}

/**
 * @brief  retrieve an extended attribute value
 *      gfal_getxattr  retrieves an extended value from an url managed by gfal

 * @param path : path of the file/dir, can be in supported protocols (lfn, srm, file, guid,..)
 * @param name: name of the attribute to get
 * @param value:  pointer to buffer to get the value
 * @param size : size of the buffer
 * @return  return the size of the data returned, or -1 if error. In the  latter  case,  errno  is  set
       appropriately, you can call @ref gfal_posix_check_error() for a more complete description. 
*/
ssize_t gfal_getxattr (const char *path, const char *name,
                        void *value, size_t size){
	return gfal_posix_internal_getxattr(path,name, value, size);
}


ssize_t gfal_readlink(const char* path, char* buff, size_t buffsiz){
	return gfal_posix_internal_readlink(path, buff, buffsiz);
}

int gfal_unlink(const char* path){
	return gfal_posix_internal_unlink(path);
}

/**
 * @brief  list all extended attributes 
 *      gfal_listxattr  list all extended atributes associated with a file

 * @param path : path of the file/dir, can be in supported protocols (lfn, srm, file, guid,..)
 * @param list: lsit of the attribute ina stirng format, separated by '\\0'. the size is equal of of the return
 * @param size : size of the buffer
 * @return  return the size of the data returned, or -1 if error. In the  latter  case,  errno  is  set
       appropriately, you can call @ref gfal_posix_check_error() for a more complete description. 
*/
ssize_t gfal_listxattr (const char *path, char *list, size_t size){
	return gfal_posix_internal_listxattr(path, list, size);
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
	}else if ( (err = *gfal_posix_get_last_error()) != NULL){
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
	g_clear_error( gfal_posix_get_last_error());
	errno =0;	
}

/**
 *  return the current error code registered or if not exist
 * 
 * */
int gfal_posix_code_error(){
	GError* err=NULL;
	int ret = ((err = *gfal_posix_get_last_error()) != NULL)? err->code :0 ;
	return ret;
}

/**
 *  check the current error, if no error report return 0 else return 1 and print the error on stderr
 *  @warning this does not clear the error
 * */
int gfal_posix_check_error(){
	GError* err=NULL;
	if((err = *gfal_posix_get_last_error()) != NULL) {
		g_printerr("[gfal]%s\n", err->message);
		return 1;
	}
	return 0;
}

/**
 * get the current error string
 * 
 */
char* gfal_posix_strerror_r(char* buff_err, size_t s_err){
	return (char*)gfal_str_GError_r(gfal_posix_get_last_error(), buff_err, s_err);
 }
 



 
 void gfal_set_nobdii (int value){
    nobdii = value;
}

int gfal_is_nobdii (){
    return (nobdii);
}

