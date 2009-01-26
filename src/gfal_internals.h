/*
 * Copyright (C) 2008-2010 by CERN
 */

/*
 * @(#)$RCSfile: gfal_internals.h,v $ $Revision: 1.3 $ $Date: 2009/01/26 08:29:09 $ CERN Remi Mollon
 */

#ifndef _GFAL_INTERNALS_H
#define _GFAL_INTERNALS_H

/* enforce proper calling convention */
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include "gfal_constants.h"
#include "gfal_types.h"

/* Macro function to print debug info if LCG_GFAL_DEBUG is defined */
#ifdef LCG_GFAL_DEBUG
#define GFAL_DEBUG(format, ...) \
	fprintf (stderr, format, ## __VA_ARGS__)
#else
#define GFAL_DEBUG(format, ...)
#endif


/******************** gfal.c ********************/

const char *gfal_version ();
//void gfal_errmsg (char *, int, const char *, int);
void gfal_errmsg (char *, int, int, const char *, ...);
char *gfal_get_userdn (char *errbuf, int errbufsz);
char *gfal_get_vo (char *errbuf, int errbufsz);
int gfal_get_fqan (char ***fqan, char *errbuf, int errbufsz);
int gfal_is_nobdii ();
int gfal_register_file (const char *, const char *, const char *, mode_t, GFAL_LONG64, int, char *, int);
void gfal_internal_free (gfal_internal);
void gfal_spacemd_free (int, gfal_spacemd *);
char *get_catalog_endpoint(char *, int);
int guid_exists (const char *, char *, int);
int gfal_guidsforpfns (int, const char **, int, char ***, int **, char *, int);
char *gfal_guidforpfn (const char *, char *, int);
char *guidfromlfn (const char *, char *, int);
char **gfal_get_aliases (const char *, const char *, char *, int);
int register_alias (const char *, const char *, char *, int);
int unregister_alias (const char *, const char *, char *, int);
int gfal_unregister_pfns (int, const char **, const char **, int **, char *, int);
char **gfal_get_replicas (const char *, const char *, char *, int);
char *gfal_get_hostname (const char *, char *, int);

/* legacy method for EDG Catalog where size is set on pfn, not guid */
int setfilesize (const char *, GFAL_LONG64, char *, int);

char *get_default_se(char *, int);
int purify_surl (const char *, char *, const int);
int setypesandendpointsfromsurl (const char *, char ***, char ***, char *, int);
int setypesandendpoints (const char *, char ***, char ***, char *, int);
int canonical_url (const char *, const char *, char *, int, char *, int);
int parseturl (const char *, char *, int, char *, int, char*, int);
int replica_exists(const char*, char*, int);
int getdomainnm (char *name, int namelen);
char **get_sup_proto ();
struct proto_ops *find_pops (const char *);
int mapposixerror (struct proto_ops *, int);


/******************** gfal_file.c ********************/

gfal_file gfal_file_new (const char *, const char *, int, char *, int);
int gfal_file_free (gfal_file);
const char *gfal_file_get_catalog_name (gfal_file);
const char *gfal_file_get_replica (gfal_file);
int gfal_file_get_replica_errcode (gfal_file);
const char *gfal_file_get_replica_errmsg (gfal_file);
int gfal_file_set_replica_error (gfal_file, int, const char *);
int gfal_file_set_turl_error (gfal_file, int, const char *);
int gfal_file_next_replica (gfal_file);
char *gfal_generate_lfn (char *, int);
char *gfal_generate_guid (char *, int);


/******************** mds_ifce.c ********************/

int get_bdii (char *, int, int *, char *, int);
int get_cat_type(char **);
int get_ce_ap (const char *, char **, char *, int);
int get_lfc_endpoint (char **, char *, int);
int get_rls_endpoints (char **, char **, char *, int);
int get_storage_path (const char *, const char *, char **, char **, char *, int);
int get_seap_info (const char *, char ***, int **, char *, int);
int get_se_types_and_endpoints (const char *, char ***, char ***, char *, int);


/******************** lfc_ifce.c ********************/

int lfc_maperror (struct proto_ops *, int);
int lfc_setsize (const char *, GFAL_LONG64, char *, int);
int lfc_accessl (const char *, const char *, int, char *, int);
int lfc_chmodl (const char *, mode_t, char *, int);
int lfc_renamel (const char *, const char *, char *, int);
int lfc_statl (const char *, const char *, struct stat64 *, char *, int);
int lfc_lstatl (const char *, struct stat64 *, char *, int);
DIR *lfc_opendirlg (const char *, const char *, char *, int);
int lfc_rmdirl (const char *, char *, int);
int lfc_register_file (const char *, const char *, const char *, mode_t,
		GFAL_LONG64, int, char *, int);
int lfc_deletepfn (const char *, const char *, char *, int);
int lfc_deletesurl (const char *, char *, int);
char *lfc_get_catalog_endpoint(char *, int);
char *lfc_guidforpfn (const char *, char *, int);
int lfc_guidsforpfns (int, const char **, int, char ***, int **, char *, int);
char *lfc_guidfromlfn (const char *, char *, int);
int lfc_guid_exists (const char *, char *, int);
char **lfc_get_aliases (const char *, const char *, char *, int);
int lfc_mkdirp (const char*, mode_t, char *, int);
int lfc_register_alias (const char *, const char *, char *, int);
int lfc_replica_exists(const char*, char*, int);
char **lfc_get_replicas (const char *, const char *, char *, int);
int lfc_unregister_alias (const char *, const char *, char *, int);
int lfc_unregister_pfns (int, const char **, const char **, int **, char *, int);
int lfc_fillsurls (gfal_file, char *, int);
int lfc_remove (gfal_file, char *, int);


/******************** lrc_ifce.c ********************/

int lrc_deletepfn (const char *, const char *, char *, int);
int lrc_deletesurl (const char *, char *, int);
char *lrc_get_catalog_endpoint(char *, int);
char *lrc_guidforpfn (const char *, char *, int);
int lrc_guid_exists (const char *, char *, int);
int lrc_register_pfn (const char *, const char *, char *, int);
int lrc_replica_exists(const char* ,char*, int);
int lrc_setfilesize (const char *, GFAL_LONG64, char *, int);
int lrc_unregister_pfn (const char *, const char *, char *, int);
char **lrc_surlsfromguid (const char *, char *, int);
int lrc_fillsurls (gfal_file);


/******************** rmc_ifce.c ********************/

char *rmc_guidfromlfn (const char *, char *, int);
char **rmc_lfnsforguid (const char *, char *, int);
int rmc_register_alias (const char *, const char *, char *, int);
int rmc_unregister_alias (const char *, const char *, char *, int);


/******************** gridftp_ifce.c ********************/

int gridftp_delete (char *, char *, int, int);
int gridftp_ls (char *, int *, char ***, struct stat64 **, char *, int, int);


/******************** sfn_ifce.c ********************/

int sfn_deletesurls (int, const char **, struct sfn_filestatus **, char *, int, int);
int sfn_getfilemd (int, const char **, struct srmv2_mdfilestatus **, char *, int, int);
int sfn_turlsfromsurls (int, const char **, char **, struct sfn_filestatus **, char *, int);


/******************** srm_ifce.c ********************/

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
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
int srm_getfilemd (int, const char **, const char *, struct srm_mdfilestatus **, char *, int, int);
#endif


/******************** srm2_2_ifce.c ********************/

int srmv2_deletesurls (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_rmdir (const char *, const char *, int, struct srmv2_filestatus **, char *, int, int);
int srmv2_get (int, const char **, const char *, int, char **, char **, struct srmv2_filestatus **, char *, int, int);
int srmv2_gete (int, const char **, const char *, const char *, int, char **, char **, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_getstatus (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_getstatuse (const char *, const char *, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_getspacetokens (const char *, const char *, int *, char ***, char *, int, int);
int srmv2_getspacemd (int, const char **, const char *, gfal_spacemd **, char *, int, int);
char *srmv2_getbestspacetoken (const char *, const char *, GFAL_LONG64, char *, int, int);
int srmv2_makedirp (const char *, const char *, char *, int, int);
int srmv2_prestage (int, const char **, const char *, int, char **, int, char **, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_prestagee (int, const char **, const char *, const char *, char **, int, char **, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_prestagestatus (int, const char **, const char *, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_prestagestatuse (const char *, const char *, struct srmv2_pinfilestatus **, char *, int, int);
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
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
int srmv2_getfilemd (int, const char **, const char *, int, int *, int, struct srmv2_mdfilestatus **, char **, char *, int, int);
#endif

#ifdef __cplusplus
}
#endif

#endif
