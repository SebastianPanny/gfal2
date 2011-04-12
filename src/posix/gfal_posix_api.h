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
 * @file gfal_posix.c
 * @brief main header file for API of the posix lib
 * @author Devresse Adrien
 * @version 2.0
 * @date 11/04/2011
 * */
 
 
#include "gfal_internals.h"
#include "gfal_constants.h"
#include "../common/gfal_common.h"
#include "gfal_types.h"
#include <glib.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * \brief test access to the given file
 * \param file can be in supported protocols lfn, srm, file, tftp
 * \return This routine returns 0 if the operation was successful or -1 if the operation failed. In the latter case, errno is set appropriately \n
 *  - ERRNO list : \n
 *    - ENOENT: The named file/directory does not exist.
 *    - EACCES: Search permission is denied on a component of the path prefix or specified access to the file itself is denied.
 *    - EFAULT: path is a NULL pointer.
 *    - ENOTDIR: A component of path prefix is not a directory.
 *    - EINVAL: path has an invalid syntax or amode is invalid.
 *    - ECOMM: Communication error.
 *    - EPROTONOSUPPORT: Access method not supported.
 * 
 */
int gfal_access (const char *, int);

/**
 * \brief set rights of the file/dir 
 * 
 * changes access mode of the file/directory path according to the bit pattern in mode.
 * \param path specifies the file name (this can only be a TURL in the current implementation).
 * \param mode the bit pattern is built by an OR of the constants defined in <sys/stat.h>.
 * \return This routine returns 0 if the operation was successful or -1 if the operation failed. In the latter case, errno is set appropriately \n
 *  - ERRNO list : \n
 *    - ENOENT: The named file/directory does not exist.
 *    - EACCES: Search permission is denied on a component of the path prefix or specified access to the file itself is denied.
 *    - EFAULT: path is a NULL pointer.
 *    - ENOTDIR: A component of path prefix is not a directory.
 *    - EINVAL: path has an invalid syntax or amode is invalid.
 *    - ECOMM: Communication error.
 *    - EPROTONOSUPPORT: Access method not supported.
*/
int gfal_chmod (const char *, mode_t);

/**
 * \brief close the current file descriptor 
 * 
 *   close the file whose descriptor fd is the one returned by gfal_open.
 *  \return  This routine returns 0 if the operation was successful or -1 if the operation failed. In the latter case, errno is set appropriately.
 *  - ERRNO list : \n
 *    - EBADF:  fd is not a valid file descriptor
 *    - ECOMM: Communication error.
 *    - EPROTONOSUPPORT: Access method not supported.
*/
int gfal_close (int);
/**
 * \brief gfal_closedir - close a directory  
 * 
 * gfal_closedir closes the directory opened by gfal_opendir and associated with the DIR structure pointed by dirp
 * \param file descriptor
 * \return This routine returns 0 if the operation was successful or -1 if the operation failed. In the latter case, errno is set appropriately.  

	- ERRORS :
		-EBADF :File descriptor in DIR structure is invalid. 
		- ECOMM : Communication error. 
		- EPROTONOSUPPORT : Access method not supported. 
 * */
int gfal_closedir (DIR *);
/**
 * \brief creates a new file or truncates an existing one
 * 
 * \param filename specifies the file name: either a logical file name, a guid, an SURL or a TURL
 * \param mode   is used only if the file is created
 *  \return This routine returns a write-only file descriptor if the operation was successful or -1 if the operation failed. In the latter case, errno is set appropriately
 *  - ERRNO list : \n
 *    - ENOENT: The named file/directory does not exist.
 *    - EACCES: Search permission is denied on a component of the path prefix or specified access to the file itself is denied.
 *    - EFAULT: path is a NULL pointer.
 *    - EBUSY: Device or resource busy. Happens if you try to open in write mode a CASTOR file that is in an active migration stream
 *    - ENOTDIR: A component of path prefix is not a directory.
 *    - EINVAL: path has an invalid syntax or amode is invalid.
 *    - ECOMM: Communication error.
 *    - EPROTONOSUPPORT: Access method not supported.
 */
int gfal_creat (const char *, mode_t);
/**
 * \brief same as create for large file
 */
int gfal_creat64 (const char *, mode_t);
/**
 *  \brief gfal_lseek - position a file 
 * 
 * gfal_lseek positions/repositions to offset the file associated with the descriptor fd generated by a previous gfal_open. whence indicates how to interpret the offset value:

    - SEEK_SET
        The offset is set from beginning of file. 
    - SEEK_CUR
        The offset is added to current position. 
    - SEEK_END
        The offset is added to current file size. 
     
    @return This routine returns the actual offset from the beginning of the file if the operation was successful or -1 if the operation failed. In the latter case, errno is set appropriately.  
	- ERRORS :
		- EBADF
			fd is not a valid file descriptor. 
		- EINVAL
			incorrect value for whence. 
		- EFBIG 
			Resulting offset is too big to fit in off_t. 
		- ECOMM
			Communication error. 
		- EPROTONOSUPPORT
			Access method not supported. 

	gfal_lseek64 performs the same function on large files using off64_t rather than off_t. 
 */ 
off_t gfal_lseek (int, off_t, int);

int gfal_mkdir (const char *, mode_t);
/**
 * \brief opens a file according to the value of flags
 *  \param filename specifies the file name: either a logical file name, a guid, an SURL or a TURL
 *  \param flags  value is built by OR’ing the bits defined in <fcntl.h> but one and only one of the first three flags below must be used
           - O_RDONLY    open for reading only
		   - O_WRONLY    open for writing only
           - O_RDWR      open for reading and writing
		   - O_CREAT     If the file exists already and O_EXCL is also set, gfal_open will fail.
		   - O_LARGEFILE allows files whose sizes cannot be represented in 31 bits to be opened
	\param mode usef only if file is created
	\return This routine returns the file descriptor if the operation was successful or -1 if the operation failed. In the latter case, errno is set appropriately
 *  - ERRNO list : \n
 *    - EACCES: Search permission is denied on a component of the path prefix or specified access to the file itself is denied.
 *    - EFAULT: path is a NULL pointer.
 *    - ENOTDIR: A component of path prefix is not a directory.
 *    - EINVAL: path has an invalid syntax or amode is invalid.
 *    - ECOMM: Communication error.
 *    - EPROTONOSUPPORT: Access method not supported.	
*/		   
int gfal_open (const char *, int, mode_t);

int gfal_open64 (const char *, int, mode_t);

DIR *gfal_opendir (const char *);

ssize_t gfal_read (int, void *, size_t);

struct dirent *gfal_readdir (DIR *);

int gfal_rename (const char *, const char *);

int gfal_rmdir (const char *);

ssize_t gfal_setfilchg (int, const void *, size_t);

int gfal_unlink (const char *);

ssize_t gfal_write (int, const void *, size_t);



#if defined(linux) || defined(sun)
struct dirent *gfal_readdir (DIR *);
#endif

#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)


off64_t gfal_lseek64 (int, off64_t, int);


int gfal_lstat (const char *, struct stat *);


int gfal_lstat64 (const char *, struct stat64 *);


int gfal_stat (const char *, struct stat *);


int gfal_stat64 (const char *, struct stat64 *);


#if defined(linux) || defined(sun)
struct dirent64 *gfal_readdir64 (DIR *);
#endif
#endif


const char *gfal_version ();
/**
 * \brief set the verbose global level
 * \param values : { \ref GFAL_VERBOSE_QUIET, \ref GFAL_VERBOSE_NORMAL , \ref GFAL_VERBOSE_VERBOSE , \ref GFAL_VERBOSE_DEBUG  }
 */
int gfal_set_verbose (int);
/**
 * \brief set the vo used by gfal
 */
int gfal_set_vo (const char *vo);
/**
 *  \brief define if bdii System is ignored or not
 *  \param : 0 for False else true
 * */
void gfal_set_nobdii (int);
/**
 * \brief change the default timeout 
 */
void gfal_set_timeout_connect (int);
/**
 * \brief get the current timeout
 * default value : 60
 */
int gfal_get_timeout_connect ();
/**
 * set the value for the send/receive timeout 
 */
void gfal_set_timeout_sendreceive (int);
/**
 *  get the value for the send/receive timeout 
 *  default value : 0
 */
int gfal_get_timeout_sendreceive ();
/**
 * set the timeout for the bdii queries
 */
void gfal_set_timeout_bdii (int);
/**
 * get the current the timeout for the bdii queries
 *  defautl value : 60
 */
int gfal_get_timeout_bdii ();
/**
 * 	set the timeout for the SRM queries
 * */
void gfal_set_timeout_srm (int);
/**
 * 	get the timeout for the SRM queries
 *  default value : 3600
 * */
int gfal_get_timeout_srm ();


const char *gfal_get_checksum_algname (enum gfal_cksm_type);

enum gfal_cksm_type gfal_parse_checksum_algname (const char *);

int gfal_is_nobdii ();

