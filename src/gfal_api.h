/*
 * Copyright (C) 2003-2004 by CERN
 */

/*
 * @(#)$RCSfile: gfal_api.h,v $ $Revision: 1.9 $ $Date: 2004/08/20 14:50:33 $ CERN Jean-Philippe Baud
 */

#ifndef _GFAL_API_H
#define _GFAL_API_H
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

		/* User-callable entry points */

int gfal_access (const char *, int);
int gfal_chmod (const char *, mode_t);
int gfal_close (int);
int gfal_closedir (DIR *);
int gfal_creat (const char *, mode_t);
int gfal_creat64 (const char *, mode_t);
off_t gfal_lseek (int, off_t, int);
int gfal_lstat (const char *, struct stat *);
int gfal_mkdir (const char *, mode_t);
int gfal_open (const char *, int, mode_t);
int gfal_open64 (const char *, int, mode_t);
DIR *gfal_opendir (const char *);
ssize_t gfal_read (int, void *, size_t);
struct dirent *gfal_readdir (DIR *);
int gfal_rename (const char *, const char *);
int gfal_rmdir (const char *);
ssize_t gfal_setfilchg (int, const void *, size_t);
int gfal_stat (const char *, struct stat *);
int gfal_unlink (const char *);
ssize_t gfal_write (int, const void *, size_t);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
off64_t gfal_lseek64 (int, off64_t, int);
int gfal_lstat64 (const char *, struct stat64 *);
#if defined(linux) || defined(sun)
struct dirent64 *gfal_readdir64 (DIR *);
#endif
int gfal_stat64 (const char *, struct stat64 *);
#else
#if defined(__USE_FILE_OFFSET64)
off_t gfal_lseek64 (int, off_t, int);
int gfal_lstat64 (const char *, struct stat *);
struct dirent *gfal_readdir64 (DIR *);
int gfal_stat64 (const char *, struct stat *);
#endif
#endif

		/* Internal functions */

int deletepfn (const char *, const char *);
int deletesurl (const char *);
int get_bdii (char *, int, int *);
int get_ce_ap (const char *, char **);
int get_rls_endpoints (char **, char **);
int get_sa_root (const char *, const char *, char **);
int get_se_endpoint (const char *, char **);
int get_se_port (const char *, int *);
int get_se_type (const char *, char **);
int get_seap_info (const char *, char ***, int **);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
int getfilemd (const char *, struct stat64 *);
int se_getfilemd (const char *, struct stat64 *);
int srm_getfilemd (const char *, struct stat64 *);
#endif
char *guidforpfn (const char *);
char *guidfromlfn (const char *);
char **lfnsforguid (const char *);
int lrc_guid_exists (const char *);
int parsesurl (const char *, char **, char **);
int parseturl (const char *, char **, char **);
int register_alias (const char *, const char *);
int register_pfn (const char *, const char *);
int se_deletesurl (const char *);
int se_set_xfer_done (const char *, int, int, char *, int);
int se_set_xfer_running (const char *, int, int, char *);
char *se_turlfromsurl (const char *, char **, int, int *, int *, char **);
int set_xfer_done (const char *, int, int, char *, int);
int set_xfer_running (const char *, int, int, char *);
int setfilesize (const char *, long long);
int setypefromsurl (const char *, char **);
int srm_deletesurl (const char *);
int srm_set_xfer_done (const char *, int, int, char *, int);
int srm_set_xfer_running (const char *, int, int, char *);
char *srm_turlfromsurl (const char *, char **, int, int *, int *, char **);
int srm_turlsfromsurls (int, const char **, long long *, char **, int, int *, int **, char **, char ***);
char *surlfromguid (const char *);
char **surlsfromguid (const char *);
char *turlfromsfn (const char *, char **);
char *turlfromsurl (const char *, char **, int, int *, int *, char **);
int unregister_alias (const char *, const char *);
int unregister_pfn (const char *, const char *);
#endif
