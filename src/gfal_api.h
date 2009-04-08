/*
 * Copyright (C) 2003-2005 by CERN
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


/******************** SRM functions ********************/

gfal_request gfal_request_new ();
int gfal_init (gfal_request, gfal_internal *, char *, int);
int gfal_deletesurls (gfal_internal, char *, int);
int gfal_removedir (gfal_internal, char *, int);
int gfal_ls (gfal_internal, char *, int);
int gfal_ls_end (gfal_internal, char *, int);
int gfal_turlsfromsurls (gfal_internal, char *, int);
int gfal_get (gfal_internal, char *, int);
int gfal_getstatus (gfal_internal, char *, int);
int gfal_bringonline (gfal_internal, char *, int);
int gfal_prestage (gfal_internal, char *, int);
int gfal_prestagestatus (gfal_internal, char *, int);
int gfal_pin (gfal_internal, char *, int);
int gfal_release (gfal_internal, char *, int);
int gfal_set_xfer_done (gfal_internal, char *, int);
int gfal_set_xfer_running (gfal_internal, char *, int);
int gfal_abortrequest (gfal_internal, char *, int);
int gfal_abortfiles (gfal_internal, char *, int);
int gfal_get_results (gfal_internal, gfal_filestatus **);
int gfal_get_ids_setype (gfal_internal, enum se_type *, int *, int **, char **);
int gfal_get_ids (gfal_internal, int *, int **, char **);
int gfal_set_ids (gfal_internal, int, const int *, int, const char *, char *, int);
void gfal_internal_free (gfal_internal);

#ifdef __cplusplus
}
#endif

#endif
