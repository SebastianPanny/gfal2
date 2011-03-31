/*
 *  Doc File for the POSIX API in the 2.X revisions
 * 
 * 	author : Devresse Adrien
 * 
*/


/**
	\defgroup posix_group all POSIX style function
*/

/**
	\addtogroup posix_group
	@{
*/

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

off64_t gfal_lseek64 (int, off64_t, int);

int gfal_lstat (const char *, struct stat *);

int gfal_lstat64 (const char *, struct stat64 *);

int gfal_stat (const char *, struct stat *);

int gfal_stat64 (const char *, struct stat64 *);


/**
	@} 
	End of the POSIX groupe
*/
