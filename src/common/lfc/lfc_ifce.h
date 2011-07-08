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
#if 0
/* * @(#)$RCSfile: lfc_ifce.c,v $ $Revision: 1.67 $ $Date: 2010/01/22 15:25:54 $ CERN James Casey
 */
#ifndef GFAL_LFC_IFCE_H_
#define GFAL_LFC_IFCE_H_

#include <sys/types.h>
#include <lfc_api.h>

#include "gfal_types.h"

#define ALLOC_BLOCK_SIZE 16 /* the block size to allocate new pointers in */

struct fc_ops {
	int	*serrno;
	char	*(*sstrerror)(int);
	int	(*addreplica)(const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *);
	int	(*creatg)(const char *, const char *, mode_t);
	int	(*delreplica)(const char *, struct lfc_fileid *, const char *);
	int	(*aborttrans)();
	int	(*endtrans)();
	int	(*getpath)(char *, u_signed64, char *);
	int (*getlinks)(const char *, const char *, int *, struct lfc_linkinfo **);
	int (*getreplica)(const char *, const char *, const char *, int *, struct lfc_filereplica **);
	int	(*lstat)(const char *, struct lfc_filestat *);
	int	(*mkdirg)(const char *, const char *, mode_t);
	int	(*seterrbuf)(char *, int);
	int	(*setfsizeg)(const char *, u_signed64, const char *, char *);
	int	(*setfsize)(const char *, struct lfc_fileid *, u_signed64);
	int	(*starttrans)(const char *, const char *);
	int	(*statg)(const char *, const char *, struct lfc_filestatg *);
	int	(*statr)(const char *, struct lfc_filestatg *);
	int	(*symlink)(const char *, const char *);
	int	(*unlink)(const char *);
	int	(*access)(const char *, int);
	int	(*chmod)(const char *, mode_t);
	int	(*rename)(const char *, const char *);
	lfc_DIR *(*opendirg)(const char *, const char *);
	int	(*rmdir)(const char *);
	int (*startsess) (char *, char *); 
	int (*endsess) ();
};

char * lfc_get_catalog_endpoint (char *errbuf, int errbufsz);

int lfc_replica_exists (const char *guid, char *errbuf, int errbufsz);

int lfc_accessl (const char *path, const char *guid, int mode, char *errbuf, int errbufsz);

int lfc_chmodl (const char *path, mode_t mode, char *errbuf, int errbufsz);

/** lfc_guidforpfn : Get the guid for a replica.  If the replica does not
  exist, fail with ENOENT */
char * lfc_guidforpfn (const char *pfn, char *errbuf, int errbufsz);

/** lfc_guidsforpfns : Get the guid for a replica.  If the replica does not
  exist, fail with ENOENT */
int lfc_guidsforpfns (int nbfiles, const char **pfns, int amode, char ***guids, int **statuses, 
    char *errbuf, int errbufsz);

int lfc_guid_exists (const char *guid, char *errbuf, int errbufsz);

char ** lfc_get_replicas (const char * lfn, const char *guid, char *errbuf, int errbufsz);

/* Unregister replicas (SURLs) from a GUIDs, and remove all links to a GUID
 * if there are no more replicas */
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

#endif /* #define GFAL_LFC_IFCE_H_ */

#endif
