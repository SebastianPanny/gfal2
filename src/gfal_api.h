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

/*
 * @(#)$RCSfile: gfal_api.h,v $ $Revision: 1.69 $ $Date: 2009/04/08 14:23:09 $ CERN Jean-Philippe Baud
 */

#ifndef _GFAL_API_H
#define _GFAL_API_H

/* enforce proper calling convention */
#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "gfal_constants.h"
#include "gfal_types.h"


/******************** Common functions ********************/

const char *gfal_version ();
int gfal_set_verbose (int);
int gfal_set_vo (const char *vo);
void gfal_set_nobdii (int);
void gfal_set_timeout_connect (int);
int gfal_get_timeout_connect ();
void gfal_set_timeout_sendreceive (int);
int gfal_get_timeout_sendreceive ();
void gfal_set_timeout_bdii (int);
int gfal_get_timeout_bdii ();
void gfal_set_timeout_srm (int);
int gfal_get_timeout_srm ();
const char *gfal_get_checksum_algname (enum gfal_cksm_type);
enum gfal_cksm_type gfal_parse_checksum_algname (const char *);

#ifdef SWIGPYTHON
int gfal_get_errno ();
#endif


/******************** POSIX functions ********************/
/*
int gfal_access (const char *, int);
int gfal_chmod (const char *, mode_t);
int gfal_close (int);
int gfal_closedir (DIR *);
int gfal_creat (const char *, mode_t);
int gfal_creat64 (const char *, mode_t);
off_t gfal_lseek (int, off_t, int);
int gfal_mkdir (const char *, mode_t);
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
*/

/******************** SRM functions ********************/


#ifdef __cplusplus
}
#endif

#endif
