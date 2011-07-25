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
 * @file gfal_posix_api
 * @brief main header file for API of the posix lib
 * @author Devresse Adrien
 * @version 2.0
 * @date 11/04/2011
 * */
 
 
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>





int gfal_chmod(const char* path, mode_t mode);
int gfal_rename (const char *oldpath, const char * newpath);

// access and stat purpose
int gfal_stat (const char *, struct stat *);
int gfal_lstat (const char *, struct stat *);
int gfal_access (const char *, int);


//classical read/write operations
int gfal_creat (const char *, mode_t);
int gfal_open (const char *, int, mode_t);
off_t gfal_lseek (int, off_t, int);
int gfal_close (int);
int gfal_read (int, void *, size_t);
int gfal_write (int, const void *, size_t);


// advanced use purpose ( set properties, guid, replicas )
ssize_t gfal_getxattr (const char *path, const char *name,
                        void *value, size_t size);
ssize_t gfal_listxattr (const char *path, const char *list, size_t size);



// directory management purpose
int gfal_mkdir (const char *, mode_t);
DIR *gfal_opendir (const char *);
int gfal_closedir (DIR *);
struct dirent *gfal_readdir (DIR *);


int gfal_rmdir (const char *);
int gfal_unlink (const char *);


// error management
void gfal_posix_clear_error();
void gfal_posix_release_error();
char* gfal_posix_strerror_r(char* buff_err, size_t s_err);
void gfal_posix_print_error();
int gfal_posix_code_error();


// define the verbose mode
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

int gfal_is_nobdii ();

