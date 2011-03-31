/*
 *  Doc File for the low level lfc API in the 2.X revisions
 * 
 * 	author : Devresse Adrien
 * 
*/

/**
 * \defgroup lfc_low_group internal LFC functions
 * 
 * */

/**
 * \addtogroup lfc_low_group
 *  @{
*/

static int lfc_mkdirp_trans (const char *, mode_t, char *, int, int);

static int lfc_init (char *errbuf, int errbufsz) 

char * lfc_get_catalog_endpoint (char *errbuf, int errbufsz);

int lfc_replica_exists (const char *guid, char *errbuf, int errbufsz);

int lfc_accessl (const char *path, const char *guid, int mode, char *errbuf, int errbufsz);

int lfc_chmodl (const char *path, mode_t mode, char *errbuf, int errbufsz);

/** 
 * lfc_guidforpfn : Get the guid for a replica.  If the replica does not
  exist, fail with ENOENT 
  * */
char * lfc_guidforpfn (const char *pfn, char *errbuf, int errbufsz);

/**
 *  lfc_guidsforpfns : Get the guid for a replica.  If the replica does not
  exist, fail with ENOENT 
  * */
int lfc_guidsforpfns (int nbfiles, const char **pfns, int amode, char ***guids, int **statuses, 
    char *errbuf, int errbufsz);
/**
 * test if guid exists
 */
int lfc_guid_exists (const char *guid, char *errbuf, int errbufsz);

char ** lfc_get_replicas (const char * lfn, const char *guid, char *errbuf, int errbufsz);

/**
 *  Unregister replicas (SURLs) from a GUIDs, and remove all links to a GUID
 * if there are no more replicas 
 * */
int lfc_unregister_pfns (int nbguids, const char **guids, const char **pfns, int **results, 
    char *errbuf, int errbufsz);

char * lfc_guidfromlfn (const char *lfn, char *errbuf, int errbufsz);

char ** lfc_get_aliases (const char *lfn, const char *guid, char *errbuf, int errbufsz);

int lfc_register_file (const char *lfn, const char *guid, const char *surl, mode_t mode,
		GFAL_LONG64 size, int bool_createonly, char *errbuf, int errbufsz);

int lfc_register_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz);

int lfc_unregister_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz);

int lfc_mkdirp (const char *path, mode_t mode, char *errbuf, int errbufsz);

int lfc_renamel (const char *old_name, const char *new_name, char *errbuf, int errbufsz);

DIR * lfc_opendirlg (const char *dirname, const char *guid, char *errbuf, int errbufsz);

int lfc_rmdirl (const char *dirname, char *errbuf, int errbufsz);

int lfc_maperror (struct proto_ops *pops, int ioflag);

int lfc_setsize (const char *lfn, GFAL_LONG64 size, char *errbuf, int errbufsz);

int lfc_statl (const char *lfn, const char *guid, struct stat64 *buf, char *errbuf, int errbufsz);

int lfc_lstatl (const char *lfn, struct stat64 *buf, char *errbuf, int errbufsz);

int lfc_fillsurls (gfal_file gf, char *errbuf, int errbufsz);

int lfc_remove (gfal_file gfile, char *errbuf, int errbufsz);

/**
	@}
*/
