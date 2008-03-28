/*
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: gfal_api.h,v $ $Revision: 1.54 $ $Date: 2008/03/28 16:33:39 $ CERN Jean-Philippe Baud
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

/* Macro function to print debug info if LCG_GFAL_DEBUG is defined */
#ifdef LCG_GFAL_DEBUG
#define GFAL_DEBUG(format, ...) \
	fprintf (stderr, format, ## __VA_ARGS__)
#else
#define GFAL_DEBUG(format, ...)
#endif

#define VO_MAXLEN            255
#define FQAN_MAXLEN          511
#define HOSTNAME_MAXLEN      255
#define ERRMSG_LEN           1024
#define DEFAULT_BDII_TIMEOUT 60

enum se_type {TYPE_NONE = 0, TYPE_SRM, TYPE_SRMv2, TYPE_SE};

enum TFileLocality_
{
	GFAL_LOCALITY_UNKNOWN = 0,
	GFAL_LOCALITY_ONLINE_,	///< xs:string value="ONLINE"
	GFAL_LOCALITY_NEARLINE_,	///< xs:string value="NEARLINE"
	GFAL_LOCALITY_ONLINE_USCOREAND_USCORENEARLINE,	///< xs:string value="ONLINE_AND_NEARLINE"
	GFAL_LOCALITY_LOST,	///< xs:string value="LOST"
	GFAL_LOCALITY_NONE_,	///< xs:string value="NONE"
	GFAL_LOCALITY_UNAVAILABLE	///< xs:string value="UNAVAILABLE"
};
/// Typedef synonym for enum ns1__TFileLocality.
typedef enum TFileLocality_ TFileLocality;


typedef struct gfal_filestatus_ {
	char *surl;
	char *turl;
	int status;
	char *explanation;
	int	pinlifetime;
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
	struct stat64 stat;
#endif
	struct gfal_filestatus_ *subpaths;
	int nbsubpaths;
	TFileLocality locality;
} gfal_filestatus;

typedef struct gfal_request_ {
	/* [optional]
	 * if set to 1, 'surls' to NULL, and endpoint specified,
	 * 'nbfiles' SURLs will be generated */
	int							generatesurls;

	/* [optional]
	 * only used to generate SURLs, if specified */
	char *						relative_path;

	/* [required]
	 * number of files in 'surls' */
	int							nbfiles;

	/* [required]
	 * list of surls */
	char **						surls;

	/* [optional]
	 * endpoint */
	char *						endpoint;
	
	/* [required by gfal_turlsfromsurls]
	 * 0 for get request, 1 for put request */
	int							oflag;
	
	/* [optional]
	 * list of file sizes
	 * can be useful for gfal_turlsfromsurls with put request */
	GFAL_LONG64 *				filesizes;
	
	/* [optional]
	 * SE type to use *by default* */
	enum se_type				defaultsetype;
	
	/* [optional]
	 * SE type to use - will fail if this type cannot be used */
	enum se_type				setype;
	
	/* [optional]
	 * if set to 1, no bdii call will be done
	 * endpoint must be set up for SRM-compliant SE */
	int							no_bdii_check;

	/* [optional]
	 * timeout */
	int							timeout;

	/* [optional]
	 * list of protocols to use */
	char **						protocols;

	/* [optional] only used with SRMv2 */
	char *						srmv2_spacetokendesc;
	int							srmv2_desiredpintime;
	int							srmv2_lslevels;
	int							srmv2_lsoffset;
	int							srmv2_lscount;
} *gfal_request;

typedef struct gfal_internal_ {
	// INPUTS

	int							generatesurls;
	char *						relative_path;
	int							nbfiles;
	char **						surls;
	char *						endpoint;
	int							oflag;
	GFAL_LONG64 *				filesizes;
	enum se_type				defaultsetype;
	enum se_type				setype;
	int							no_bdii_check;
	int							timeout;
	char **						protocols;

	// Only used with SRMv2
	char *						srmv2_spacetokendesc;
	int							srmv2_desiredpintime;
	int							srmv2_lslevels;
	int							srmv2_lsoffset;
	int							srmv2_lscount;

	// OUTPUTS

	// For Classic SEs
	struct sfn_filestatus *		sfn_statuses;

	// For SRMv1 SEs
	int							srm_reqid;
	struct srm_filestatus *		srm_statuses;
	struct srm_mdfilestatus *	srm_mdstatuses;

	// For SRMv2.2 SEs
	char *						srmv2_token;
	struct srmv2_filestatus *	srmv2_statuses;
	struct srmv2_pinfilestatus *srmv2_pinstatuses;
	struct srmv2_mdfilestatus *	srmv2_mdstatuses;

	int free_endpoint;

	// Common results
	int returncode; // size of the new results
	int results_size; // size of the old results
	gfal_filestatus *results;
} *gfal_internal;

struct sfn_filestatus {
	char *surl;
	char *turl;
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
	char 	*turl;
	int 	status;
	char 	*explanation;
	int 	pinlifetime;
};

#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
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
	struct srmv2_mdfilestatus *subpaths;
	int nbsubpaths;
	TFileLocality locality;
};
#endif

/* to remove warnings concerning lfc_maperror and struct proto_ops */
struct proto_ops;

		/* User-callable entry points */

int gfal_set_vo (const char *vo);
char *gfal_get_vo (char *errbuf, int errbufsz);
int gfal_get_fqan (char ***fqan, char *errbuf, int errbufsz);
const char *gfal_version ();
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

int gfal_deletesurls (gfal_internal, char *, int);
int gfal_turlsfromsurls (gfal_internal, char *, int);
int gfal_ls (gfal_internal, char *, int);
int gfal_get (gfal_internal, char *, int);
int gfal_getstatus (gfal_internal, char *, int);
int gfal_prestage (gfal_internal, char *, int);
int gfal_prestagestatus (gfal_internal, char *, int);
int gfal_pin (gfal_internal, char *, int);
int gfal_release (gfal_internal, char *, int);
int gfal_set_xfer_done (gfal_internal, char *, int);
int gfal_set_xfer_running (gfal_internal, char *, int);
int gfal_abortrequest (gfal_internal, char *, int);
int gfal_abortfiles (gfal_internal, char *, int);
gfal_request gfal_request_new ();
int gfal_init (gfal_request, gfal_internal *, char *, int);
int gfal_get_results (gfal_internal, gfal_filestatus **);
int gfal_get_ids (gfal_internal, int *, int **, char **);
int gfal_set_ids (gfal_internal, int, const int *, int, const char *, char *, int);
void gfal_internal_free (gfal_internal);

                  /* catalog operation entry points */
int create_alias (const char *, const char *, GFAL_LONG64, char *, int);
int create_alias_m (const char *, const char *, mode_t, GFAL_LONG64, char *, int);
char *getbestfile(char **, int size, char *, int);
char *get_catalog_endpoint(char *, int);
int getfilesizeg(const char *, GFAL_LONG64 *, char *, int);
int guid_exists (const char *, char *, int);
int gfal_guidsforpfns (int, const char **, char ***, int **, char *, int);
char *guidforpfn (const char *, char *, int);
char *guidfromlfn (const char *, char *, int);
char **lfnsforguid (const char *, char *, int);
int register_alias (const char *, const char *, char *, int);
int register_pfn (const char *, const char *, char *, int);
int unregister_alias (const char *, const char *, char *, int);
int gfal_unregister_pfns (int, const char **, const char **, int, int **, char *, int);
char *surlfromguid (const char *, char *, int);
char **surlsfromguid (const char *, char *, int);

/* legacy method for EDG Catalog where size is set on pfn, not guid */
int setfilesize (const char *, GFAL_LONG64, char *, int);

                /* storage operation entry points */
int deletesurl (const char *, char *, int, int);
int deletesurl2 (const char *, char *, char *, int, int);
char *get_default_se(char *, int);
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
int get_sa_path (const char *, const char *, char **, char **, char *, int);
int get_seap_info (const char *, char ***, int **, char *, int);
int get_se_types_and_endpoints (const char *, char ***, char ***, char *, int);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
int srm_getfilemd (int, const char **, const char *, struct srm_mdfilestatus **, char *, int, int);
int srmv2_getfilemd (int, const char **, const char *, int, int, int, struct srmv2_mdfilestatus **, char **, char *, int, int);
#endif
int lfc_maperror (struct proto_ops *, int);
int lfc_getfilesizeg(const char *, GFAL_LONG64 *, char *, int);
int lfc_setsize (const char *, GFAL_LONG64, char *, int);
int lfc_accessl (const char *, int, char *, int);
int lfc_chmodl (const char *, mode_t, char *, int);
int lfc_renamel (const char *, const char *, char *, int);
int lfc_statl (const char *, const char *, struct stat *, char *, int);
DIR *lfc_opendirlg (const char *, const char *, char *, int);
int lfc_rmdirl (const char *, char *, int);
int lfc_create_alias (const char *, const char *, mode_t, GFAL_LONG64, char *, int);
int lfc_deletepfn (const char *, const char *, char *, int);
int lfc_deletesurl (const char *, char *, int);
char *lfc_get_catalog_endpoint(char *, int);
char *lfc_guidforpfn (const char *, char *, int);
int lfc_guidsforpfns (int, const char **, char ***, int **, char *, int);
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
int lfc_unregister_pfns (int, const char **, const char **, int, int **, char *, int);
int lrc_deletepfn (const char *, const char *, char *, int);
int lrc_deletesurl (const char *, char *, int);
char *lrc_get_catalog_endpoint(char *, int);
char *lrc_guidforpfn (const char *, char *, int);
int lrc_guid_exists (const char *, char *, int);
int lrc_replica_exists(const char* ,char*, int);
int lrc_setfilesize (const char *, GFAL_LONG64, char *, int);
int lrc_unregister_pfn (const char *, const char *, char *, int);
char *lrc_surlfromguid (const char *, char *, int);
char **lrc_surlsfromguid (const char *, char *, int);
int canonical_url (const char *, const char *, char *, int, char *, int);
int parseturl (const char *, char *, int, char *, int, char **, char*, int);
int replica_exists(const char*, char*, int);
char *rmc_guidfromlfn (const char *, char *, int);
char **rmc_lfnsforguid (const char *, char *, int);
int rmc_register_alias (const char *, const char *, char *, int);
int rmc_register_pfn (const char *, const char *, char *, int);
int rmc_unregister_alias (const char *, const char *, char *, int);
int gridftp_delete (char *, char *, int, int);
int gridftp_ls (char *, int *, char ***, struct stat64 **, char *, int, int);
int sfn_deletesurls (int, const char **, struct sfn_filestatus **, char *, int, int);
int sfn_getfilemd (int, const char **, struct srmv2_mdfilestatus **, char *, int, int);
int sfn_turlsfromsurls (int, const char **, char **, struct sfn_filestatus **, char *, int);
int srm_deletesurls (int, const char **, const char *, struct srm_filestatus **, char *, int, int);
int srm_get (int, const char **, int, char **, int *, char **, struct srm_filestatus **, int);
int srm_getx (int, const char **, int, char **, int *, struct srm_filestatus **, char *, int, int);
int srm_getxe (int, const char **, const char *, char **, int *, struct srm_filestatus **, char *, int, int);
int srm_getstatus (int, const char **, int, char *, struct srm_filestatus **, int );
int srm_getstatusx (int, const char **, int, struct srm_filestatus **, char *, int, int);
int srm_getstatusxe (int, const char *, struct srm_filestatus **, char *, int, int);
int srm_set_xfer_done (const char *, int, int, char *, int, int);
int srm_set_xfer_running (const char *, int, int, char *, int, int);
int srm_turlsfromsurls (int, const char **, const char *, GFAL_LONG64 *, char **, int, int *, struct srm_filestatus **, char *, int, int);
int srmv2_deletesurls (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_get (int, const char **, const char *, int, char **, char **, struct srmv2_filestatus **, char *, int, int);
int srmv2_gete (int, const char **, const char *, const char *, int, char **, char **, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_getstatus (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_getstatuse (const char *, const char *, struct srmv2_pinfilestatus **, char *, int, int);
char *srmv2_getspacetoken (const char *, const char *, char *, int, int);
int srmv2_makedirp (const char *, const char *, char *, int, int);
int srmv2_prestage (int, const char **, const char *, int, char **, char **, struct srmv2_filestatus **, char *, int, int);
int srmv2_prestagee (int, const char **, const char *, const char *, char **, char **, struct srmv2_filestatus **, char *, int, int);
int srmv2_prestagestatus (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_prestagestatuse (const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_set_xfer_done_get (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_set_xfer_done_put (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_set_xfer_running (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_turlsfromsurls_get (int, const char **, const char *, int, const char *, char **, char **, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_turlsfromsurls_put (int, const char **, const char *, GFAL_LONG64 *, int, const char *, char **, char **, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_pin (int, const char **, const char *, const char *, int, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_release (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_abortrequest (const char *, const char *, char *, int, int);
int srmv2_abortfiles (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_access (int, const char **, const char *, int, struct srmv2_filestatus **, char *, int, int);
char *turlfromsfn (const char *, char **, char *, int);

#ifdef __cplusplus
}
#endif

#endif
