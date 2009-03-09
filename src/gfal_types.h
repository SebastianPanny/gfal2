/*
 * Copyright (C) 2008-2010 by CERN
 */

/*
 * @(#)$RCSfile: gfal_types.h,v $ $Revision: 1.6 $ $Date: 2009/03/09 15:16:38 $ CERN Remi Mollon
 */

#ifndef _GFAL_TYPES_H
#define _GFAL_TYPES_H

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

enum TRetentionPolicy_
{
	GFAL_POLICY_UNKNOWN = 0,
	GFAL_POLICY_REPLICA,
	GFAL_POLICY_OUTPUT,
	GFAL_POLICY_CUSTODIAL
};
/// Typedef synonym for enum ns1__TRetentionPolicy.
typedef enum TRetentionPolicy_ TRetentionPolicy;

enum TAccessLatency_
{
	GFAL_LATENCY_UNKNOWN = 0,
	GFAL_LATENCY_ONLINE,
	GFAL_LATENCY_NEARLINE
};
/// Typedef synonym for enum ns1__TAccessLatency.
typedef enum TAccessLatency_ TAccessLatency;

enum gfal_cksm_type
{
	GFAL_CKSM_UNKNOWN = 0,
	GFAL_CKSM_CRC32,
	GFAL_CKSM_ADLER32,
	GFAL_CKSM_MD5,
	GFAL_CKSM_SHA1,
	GFAL_CKSM_SHA256,
	GFAL_CKSM_SHA512
};


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
	char *checksumtype;
	char *checksum;
    char **spacetokens;
    int nbspacetokens;
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
	char *checksumtype;
	char *checksum;
    char **spacetokens;
    int nbspacetokens;
};
#endif

typedef struct {
	char *spacetoken;
	char *owner;
	GFAL_LONG64 totalsize;
	GFAL_LONG64 guaranteedsize;
	GFAL_LONG64 unusedsize;
	int lifetimeassigned;
	int lifetimeleft;
	TRetentionPolicy retentionpolicy;
	TAccessLatency accesslatency;
} gfal_spacemd;

enum gfal_file_catalog {
	GFAL_FILE_CATALOG_UNKNOWN = 0,
	GFAL_FILE_CATALOG_LFC,
	GFAL_FILE_CATALOG_EDG
};

typedef struct _gfal_replica {
	char *surl;
	int errcode;
	char *errmsg;
} *gfal_replica;

typedef struct _gfal_file {
	char *file;                    // url given by user
	char *lfn;
	char *guid;
	enum gfal_file_catalog catalog;     // file catalog type: lfc or edg
	int nbreplicas;
	int nberrors;                  // nb of non-working replicas
	gfal_replica *replicas;
	int current_replica;
	gfal_internal gobj;
	char *turl;
	int errcode;
	char *errmsg;
} *gfal_file;

struct dir_info {
	DIR	*dir;
	struct proto_ops *pops;
};

struct proto_ops {
	char	*proto_name;
	int	libok;	/* -1 (library is not accessible), 0 (library not checked yet), 1 (library ok) */
	int	(*checkprotolib)(struct proto_ops *);
	int	(*geterror)();
	int	(*maperror)(struct proto_ops *, int);
	int	(*access)(const char *, int);
	int	(*chmod)(const char *, mode_t);
	int	(*close)(int);
	int	(*closedir)(DIR *);
	off_t	(*lseek)(int, off_t, int);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
	off64_t	(*lseek64)(int, off64_t, int);
#endif
	int	(*lstat)(const char *, struct stat *);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
	int	(*lstat64)(const char *, struct stat64 *);
#endif
	int	(*mkdir)(const char *, mode_t);
	int	(*open)(const char *, int, ...);
	DIR	*(*opendir)(const char *);
	ssize_t	(*read)(int, void *, size_t);
	struct dirent	*(*readdir)(DIR *);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
	struct dirent64	*(*readdir64)(DIR *);
#endif
	int	(*rename)(const char *, const char *);
	int	(*rmdir)(const char *);
	ssize_t	(*setfilchg)(int, const void *, size_t);
	int	(*stat)(const char *, struct stat *);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
	int	(*stat64)(const char *, struct stat64 *);
#endif
	int	(*unlink)(const char *);
	ssize_t	(*write)(int, const void *, size_t);
};

struct xfer_info {
	int	fd;
	int	size;
	gfal_file gfile;
	struct proto_ops *pops;
};

#ifdef __cplusplus
}
#endif

#endif
