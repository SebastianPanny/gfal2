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
 
 

#include "../common/gfal_types.h"
#include <glib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

int gfal_access (const char *, int);

int gfal_chmod(const char* path, mode_t mode);

ssize_t gfal_getxattr (const char *path, const char *name,
                        void *value, size_t size);

int gfal_rename (const char *oldpath, const char * newpath);

int gfal_stat (const char *, struct stat *);

int gfal_lstat (const char *, struct stat *);

int gfal_mkdir(const char *, mode_t mode);

DIR* gfal_opendir(const char* name);

struct dirent* gfal_readdir(DIR* dir);

void gfal_posix_clear_error();

void gfal_posix_release_error();

char* gfal_posix_strerror_r(char* buff_err, size_t s_err);

void gfal_posix_print_error();

int gfal_posix_code_error();

int gfal_close (int);

int gfal_closedir (DIR *);

int gfal_creat (const char *, mode_t);


off_t gfal_lseek (int, off_t, int);

int gfal_mkdir (const char *, mode_t);
	   
int gfal_open (const char *, int, mode_t);

DIR *gfal_opendir (const char *);

int gfal_read (int, void *, size_t);

struct dirent *gfal_readdir (DIR *);



int gfal_rmdir (const char *);

ssize_t gfal_setfilchg (int, const void *, size_t);

int gfal_unlink (const char *);

int gfal_write (int, const void *, size_t);



#if defined(linux) || defined(sun)
struct dirent *gfal_readdir (DIR *);
#endif

#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)


off64_t gfal_lseek64 (int, off64_t, int);





int gfal_lstat64 (const char *, struct stat64 *);





int gfal_stat64 (const char *, struct stat64 *);


#if defined(linux) || defined(sun)
struct dirent64 *gfal_readdir64 (DIR *);
#endif
#endif



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


