/*
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: gfal_api.h,v $ $Revision: 1.36 $ $Date: 2007/06/06 12:17:14 $ CERN Jean-Philippe Baud
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

#ifndef GFAL_LONG64_FORMAT
#if defined(__ia64__) || defined(__x86_64)
#define GFAL_LONG64_FORMAT "%ld"
#elif defined(_WIN32)
#define GFAL_LONG64_FORMAT "%I64d"
#else
#define GFAL_LONG64_FORMAT "%lld"
#endif
#endif
#ifndef GFAL_LONG64
#if defined(__ia64__) || defined(__x86_64)
#define GFAL_LONG64 long
#elif defined(_WIN32)
#define GFAL_LONG64 __i64
#else
#define GFAL_LONG64 long long
#endif
#endif

#define VO_MAXLEN  32
#define ERRMSG_LEN 256

struct se_filestatus {
	char *surl;
	char *turl;
	char *token;
	int status;
};

struct srm_filestatus {
	char	*surl;
	char	*turl;
	int		fileid;
	int		status;
};

struct srmv2_filestatus {
	char 	*surl;
	char 	*turl;
	int 	status;
	char 	*explanation;
};

struct srmv2_pinfilestatus {
	char 	*surl;
	int 	pinlifetime;
	int 	status;
	char 	*explanation;
};

#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
struct se_mdfilestatus {
	char 	*surl;
	struct stat64	stat;
	int 	status;
};

struct srm_mdfilestatus {
	char 	*surl;
	struct stat64	stat;
	int		fileid;
	int 	status;
};

struct srmv2_mdfilestatus {
	char 	*surl;
	struct stat64	stat;
	int 	status;
	char 	*explanation;
};
#endif

/* to remove warnings concerning lfc_maperror and struct proto_ops */
struct proto_ops;

		/* User-callable entry points */

int gfal_access (const char *, int);
int gfal_chmod (const char *, mode_t);
int gfal_close (int);
int gfal_closedir (DIR *);
int gfal_creat (const char *, mode_t);
int gfal_creat64 (const char *, mode_t);
void gfal_errmsg (char *, int, const char *);
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

                  /* catalog operation entry points */
int create_alias (const char *, const char *, GFAL_LONG64, char *, int);
int create_alias_m (const char *, const char *, mode_t, GFAL_LONG64, char *, int);
char *getbestfile(char **, int size, char *, int);
char *get_catalog_endpoint(char *, int);
int getfilesizeg(const char *, GFAL_LONG64 *, char *, int);
int guid_exists (const char *, char *, int);
char *guidforpfn (const char *, char *, int);
char *guidfromlfn (const char *, char *, int);
char **lfnsforguid (const char *, char *, int);
int register_alias (const char *, const char *, char *, int);
int register_pfn (const char *, const char *, char *, int);
int unregister_alias (const char *, const char *, char *, int);
int unregister_pfn (const char *, const char *, char *, int);
char *surlfromguid (const char *, char *, int);
char **surlsfromguid (const char *, char *, int);

/* legacy method for EDG Catalog where size is set on pfn, not guid */
int setfilesize (const char *, GFAL_LONG64, char *, int);

                /* storage operation entry points */
int deletesurl (const char *, char *, int, int);
int deletesurl2 (const char *, char *, char *, int, int);
char *get_default_se(char *, char *, int);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
int getfilemd (const char *, struct stat64 *, char *, int, int);
#endif
int set_xfer_done (const char *, int, int, char *, int, char *, int, int);
int set_xfer_running (const char *, int, int, char *, char *, int, int);
char *turlfromsurl (const char *, char **, int, int *, int *, char **, char *, int, int);
char *turlfromsurlx (const char *, GFAL_LONG64, char **, int, int *, int *, char **, char *, int, int);
char *turlfromsurl2 (const char *, GFAL_LONG64, const char *, char **, int, int *, int *, char **, char *, int, int);

		/* Internal functions */
int purify_surl (const char *, char *, const int);
int deletepfn (const char *, const char *, char *, int);
int get_bdii (char *, int, int *, char *, int);
int get_cat_type(char **);
int get_ce_ap (const char *, char **, char *, int);
int get_lfc_endpoint (char **, char *, int);
int get_rls_endpoints (char **, char **, char *, int);
int get_sa_root (const char *, const char *, char **, char *, int);
int get_sa_path (const char *, const char *, char **, char **, char *, int);
int get_se_typeandendpoint (const char *, char **, char **, char *, int);
int get_seap_info (const char *, char ***, int **, char *, int);
int get_srm_types_and_endpoints (const char *, char ***, char ***, char *, int);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
int se_getfilemd (int, const char **, const char *, struct se_mdfilestatus **, char *, int, int);
int srm_getfilemd (int, const char **, const char *, struct srm_mdfilestatus **, char *, int, int);
int srmv2_getfilemd (int, const char **, const char *, struct srmv2_mdfilestatus **, char **, char *, int, int);
#endif
int lfc_maperror (struct proto_ops *, int);
int lfc_getfilesizeg(const char *, GFAL_LONG64 *, char *, int);
int lfc_setsize (const char *, GFAL_LONG64, char *, int);
int lfc_accessl (const char *, int, char *, int);
int lfc_chmodl (const char *, mode_t, char *, int);
int lfc_renamel (const char *, const char *, char *, int);
DIR *lfc_opendirlg (const char *, const char *, char *, int);
int lfc_rmdirl (const char *, char *, int);
int lfc_create_alias (const char *, const char *, mode_t, GFAL_LONG64, char *, int);
int lfc_deletepfn (const char *, const char *, char *, int);
int lfc_deletesurl (const char *, char *, int);
char *lfc_get_catalog_endpoint(char *, int);
char *lfc_guidforpfn (const char *, char *, int);
char *lfc_guidfromlfn (const char *, char *, int);
int lfc_guid_exists (const char *, char *, int);
char **lfc_lfnsforguid (const char *, char *, int);
int lfc_mkdirp (const char*, mode_t, char *, int);
int lfc_register_alias (const char *, const char *, char *, int);
int lfc_register_pfn (const char *, const char *, char *, int);
int lfc_replica_exists(const char*, char*, int);
char *lfc_surlfromguid (const char *, char *, int);
char **lfc_surlsfromguid (const char *, char *, int);
int lfc_unregister_alias (const char *, const char *, char *, int);
int lfc_unregister_pfn (const char *, const char *, char *, int);
int lrc_deletepfn (const char *, const char *, char *, int);
int lrc_deletesurl (const char *, char *, int);
char *lrc_get_catalog_endpoint(char *, int);
char *lrc_guidforpfn (const char *, char *, int);
int lrc_guid_exists (const char *, char *, int);
int lrc_replica_exists(const char* ,char*, int);
int lrc_setfilesize (const char *, GFAL_LONG64, char *, int);
char *lrc_surlfromguid (const char *, char *, int);
char **lrc_surlsfromguid (const char *, char *, int);
int parsesurl (const char *, char *, int, char **, char *, int);
int parseturl (const char *, char *, int, char *, int, char **, char*, int);
int replica_exists(const char*, char*, int);
char *rmc_guidfromlfn (const char *, char *, int);
char **rmc_lfnsforguid (const char *, char *, int);
int rmc_register_alias (const char *, const char *, char *, int);
int rmc_register_pfn (const char *, const char *, char *, int);
int rmc_unregister_alias (const char *, const char *, char *, int);
int rmc_unregister_pfn (const char *, const char *, char *, int);
int se_deletesurls (int, const char **, const char*, struct se_filestatus **, char *, int, int);
int se_mkdir (const char *, const char *, char *, int, int);
int se_turlsfromsurls (int, const char **, const char *, char **, int, struct se_filestatus **, char *, int, int);
int se_set_xfer_done (const char *, const char *, int, char *, int, int);
int se_set_xfer_running (const char *, const char *, char *, int, int);
int srm_deletesurls (int, const char **, const char *, char *, int, int);
int srm_get (int, char **, int, char **, int *, char **, struct srm_filestatus **, int);
int srm_getx (int, char **, int, char **, int *, char **, struct srm_filestatus **, char *, int, int);
int srm_getxe (int, char **, int, char **, int *, char **, struct srm_filestatus **, const char *, char *, int, int);
int srm_getstatus (int, char **, int, char *, struct srm_filestatus **, int );
int srm_getstatusx (int, char **, int, char *, struct srm_filestatus **, char *, int, int);
int srm_getstatusxe (int, char **, int, char *, struct srm_filestatus **, const char *, char *, int, int);
int srm_set_xfer_done (const char *, int, int, char *, int, int);
int srm_set_xfer_running (const char *, int, int, char *, int, int);
int srm_turlsfromsurls (int, const char **, const char *, GFAL_LONG64 *, char **, int, int *, char **, struct srm_filestatus **, char *, int, int);
int srmv2_deletesurls (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_get (int, const char **, const char *, int, char **, char **, struct srmv2_filestatus **, char *, int, int);
int srmv2_gete (int, const char **, const char *, const char *, int, char **, char **, struct srmv2_filestatus **, char *, int, int);
char *srmv2_getspacetoken (const char *, const char *, char *, int, int);
int srmv2_getstatus (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_getstatuse (const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_prestage (int, const char **, const char *, int, char **, char **, struct srmv2_filestatus **, char *, int, int);
int srmv2_prestagee (int, const char **, const char *, const char *, int, char **, char **, struct srmv2_filestatus **, char *, int, int);
int srmv2_prestagestatus (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_prestagestatuse (const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_set_xfer_done_get (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_set_xfer_done_put (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_set_xfer_running (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_turlsfromsurls_get (int, const char **, const char *, GFAL_LONG64 *, const char *, char **, char **, struct srmv2_filestatus **, char *, int, int);
int srmv2_turlsfromsurls_put (int, const char **, const char *, GFAL_LONG64 *, const char *, char **, char **, struct srmv2_filestatus **, char *, int, int);
int srmv2_pin (int, const char **, const char *, const char *, int, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_release (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
char *turlfromsfn (const char *, char **, char *, int);

#ifdef __cplusplus
}
#endif

#endif
