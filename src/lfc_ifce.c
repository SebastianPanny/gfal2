/*
 * Copyright (C) 2004 by CERN
 */

/*
 * @(#)$RCSfile: lfc_ifce.c,v $ $Revision: 1.65 $ $Date: 2009/07/03 14:12:59 $ CERN James Casey
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "gfal_internals.h"
#include "lfc_api.h"
#include "serrno.h"

static const char gfal_remote_type[] = "LFC";

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

struct fc_ops fcops;
char lfc_endpoint[GFAL_HOSTNAME_MAXLEN] = "";
char lfc_env[GFAL_HOSTNAME_MAXLEN];

static int lfc_mkdirp_trans (const char *, mode_t, char *, int, int);

static int 
lfc_init (char *errbuf, int errbufsz) {
	char *lfc_host = NULL;
	char *lfc_port = NULL;

	if (*lfc_endpoint == 0) {
		/* Try first from env */
		if ((lfc_host = getenv ("LFC_HOST")) != NULL) {
			if (strlen (lfc_host) + 6 >= GFAL_HOSTNAME_MAXLEN) {
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_init][ENAMETOOLONG] %s: Host name too long", lfc_host);
				errno = ENAMETOOLONG;
				return (-1);
			}

			lfc_port = getenv ("LFC_PORT");
			if (lfc_port && strlen (lfc_port) > 5) {
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_init][EINVAL] %s: Invalid LFC port number", lfc_port);
				errno = EINVAL;
				return (-1);
			}

			if (lfc_port)
				snprintf (lfc_endpoint, GFAL_HOSTNAME_MAXLEN, "%s:%s", lfc_host, lfc_port);
			else
				snprintf (lfc_endpoint, GFAL_HOSTNAME_MAXLEN, "%s", lfc_host);
		} else if (!gfal_is_nobdii ()) { /* get endpoint from MDS */
			if (get_lfc_endpoint (&lfc_host, errbuf, errbufsz) < 0 || lfc_host == NULL)
				return (-1);

			if (strlen (lfc_host) >= GFAL_HOSTNAME_MAXLEN) {
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_init][ENAMETOOLONG] %s: Host name too long", lfc_host);
				free (lfc_host);
				errno = ENAMETOOLONG;
				return (-1);
			}

			if (strncmp (lfc_host, "lfc://", 6) == 0)
				strncpy (lfc_endpoint, lfc_host + 6, GFAL_HOSTNAME_MAXLEN);
			else /* just a plain hostname */
				strncpy (lfc_endpoint, lfc_host, GFAL_HOSTNAME_MAXLEN);
			free (lfc_host);

			if ((lfc_port = strchr (lfc_endpoint, ':')) != NULL)
				*lfc_port = '\0';

			/* export host and port to environment */
			snprintf (lfc_env, GFAL_HOSTNAME_MAXLEN, "LFC_HOST=%s", lfc_endpoint);
			if (putenv (lfc_env) < 0) {
				*lfc_endpoint = 0;
				return (-1);
			}

			if (lfc_port && *(lfc_port + 1) != '\0') {
				if (strlen (lfc_port) > 5) {
					gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_init][EINVAL] %s: Invalid LFC port number", lfc_port);
					*lfc_endpoint = 0;
					errno = EINVAL;
					return (-1);
				}
				snprintf (lfc_env, GFAL_HOSTNAME_MAXLEN, "LFC_PORT=%s", lfc_port);
				if (putenv (lfc_env) < 0) {
					*lfc_endpoint = 0;
					return (-1);
				}

				*lfc_port = ':';
			}
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
					"[GFAL][lfc_init][EINVAL] You have to define 'LFC_HOST' environment variable, when BDII calls are disabled");
			lfc_host = NULL;
			errno = EINVAL;
			return (-1);
		}
		{
			void *dlhandle;

			if ((dlhandle = dlopen (NULL, RTLD_LAZY)) == NULL) {
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_init][] %s", dlerror ());
				*lfc_endpoint = 0;
				return (-1);
			}
			
			fcops.addreplica = (int (*) (const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *)) dlsym (dlhandle, "lfc_addreplica");

			if (fcops.addreplica == NULL) {
				if ((dlhandle = dlopen ("liblfc.so", RTLD_LAZY)) == NULL) {
					gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_init][] liblfc.so: %s", dlerror ());
					*lfc_endpoint = 0;
					return (-1);
				}

				fcops.addreplica = (int (*) (const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *)) dlsym (dlhandle, "lfc_addreplica");
			}

			fcops.serrno = (int *) dlsym (dlhandle, "serrno");
			fcops.sstrerror = (char * (*) (int)) dlsym (dlhandle, "sstrerror");
			fcops.creatg = (int (*) (const char *, const char *, mode_t)) dlsym (dlhandle, "lfc_creatg");
			fcops.delreplica = (int (*) (const char *, struct lfc_fileid *, const char *)) dlsym (dlhandle, "lfc_delreplica");
			fcops.aborttrans = (int (*) ()) dlsym (dlhandle, "lfc_aborttrans");
			fcops.endtrans = (int (*) ()) dlsym (dlhandle, "lfc_endtrans");
			fcops.getpath = (int (*) (char *, u_signed64, char *)) dlsym (dlhandle, "lfc_getpath");
			fcops.getlinks = (int (*) (const char *, const char *, int *, struct lfc_linkinfo **)) dlsym (dlhandle, "lfc_getlinks");
			fcops.getreplica = (int (*) (const char *, const char *, const char *, int *, struct lfc_filereplica **)) dlsym (dlhandle, "lfc_getreplica");
			fcops.lstat = (int (*) (const char *, struct lfc_filestat *)) dlsym (dlhandle, "lfc_lstat");
			fcops.mkdirg = (int (*) (const char *, const char *, mode_t)) dlsym (dlhandle, "lfc_mkdirg");
			fcops.seterrbuf = (int (*) (char *, int)) dlsym (dlhandle, "lfc_seterrbuf");
			fcops.setfsizeg = (int (*) (const char *, u_signed64, const char *, char *)) dlsym (dlhandle, "lfc_setfsizeg");
			fcops.setfsize = (int (*) (const char *, struct lfc_fileid *, u_signed64)) dlsym (dlhandle, "lfc_setfsize");
			fcops.starttrans = (int (*) (const char*, const char*)) dlsym (dlhandle, "lfc_starttrans");
			fcops.statg = (int (*) (const char *, const char *, struct lfc_filestatg *)) dlsym (dlhandle, "lfc_statg");
			fcops.statr = (int (*) (const char *, struct lfc_filestatg *)) dlsym (dlhandle, "lfc_statr");
			fcops.symlink = (int (*) (const char *, const char *)) dlsym (dlhandle, "lfc_symlink");
			fcops.unlink = (int (*) (const char *)) dlsym (dlhandle, "lfc_unlink");
			fcops.access = (int (*) (const char *, int)) dlsym (dlhandle, "lfc_access");
			fcops.chmod = (int (*) (const char *, mode_t)) dlsym (dlhandle, "lfc_chmod");
			fcops.rename = (int (*) (const char *, const char *)) dlsym (dlhandle, "lfc_rename");
			fcops.opendirg = (lfc_DIR * (*) (const char *, const char *)) dlsym (dlhandle, "lfc_opendirg");
			fcops.rmdir = (int (*) (const char *)) dlsym (dlhandle, "lfc_rmdir");
			fcops.startsess = (int (*) (char *, char *)) dlsym (dlhandle, "lfc_startsess");
			fcops.endsess = (int (*) ()) dlsym (dlhandle, "lfc_endsess");
		}
	}
	fcops.seterrbuf (errbuf, errbufsz);
	return (0);
}

char *
lfc_get_catalog_endpoint (char *errbuf, int errbufsz) {
	if (lfc_init (errbuf, errbufsz) < 0)
		return (NULL);
	return lfc_endpoint;
}

int
lfc_replica_exists (const char *guid, char *errbuf, int errbufsz) {
	int size = 0;
	struct lfc_filereplica* replicas = NULL;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);
	if (fcops.getreplica (NULL, guid, NULL, &size, &replicas) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getreplica][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, guid, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	free (replicas);
	return (size > 0 ? 1 : 0);
}

int
lfc_accessl (const char *path, const char *guid, int mode, char *errbuf, int errbufsz)
{
	char *lfn = (char *) path;
	int sav_errno = 0;

	if (path == NULL && guid == NULL) {
		errno = EINVAL;
		return (-1);
	}
	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if (path == NULL) {
		struct lfc_linkinfo* links = NULL;
		int size = 0;

		if (fcops.startsess (lfc_endpoint, (char*) gfal_version ()) < 0) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_startsess][] %s: %s",
					gfal_remote_type, lfc_endpoint, fcops.sstrerror (*fcops.serrno));
			sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		}
		if (!sav_errno && fcops.getlinks (NULL, guid, &size, &links) < 0) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getlinks][] %s: %s: %s",
					gfal_remote_type, lfc_endpoint, guid, fcops.sstrerror (*fcops.serrno));
			sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		}
		if (!sav_errno) {
			if (links && size > 0) {
				lfn = strdup (links[0].path);
				free (links);
			} else {
				if (links) free (links);
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getlinks][]  %s: %s: No associated LFN",
						gfal_remote_type, lfc_endpoint, guid);
				sav_errno = ENOENT;
			}
		}
	}

	if (!sav_errno && fcops.access (lfn, mode) < 0) {
		sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_access][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, path, fcops.sstrerror (*fcops.serrno));
	}

	if (path == NULL) {
		if (lfn) free (lfn);
		fcops.endsess ();
	}
	errno = sav_errno;
	return (!sav_errno ? 0 : -1);
}

int
lfc_chmodl (const char *path, mode_t mode, char *errbuf, int errbufsz)
{
	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.chmod (path, mode) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_chmod][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, path, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	errno = 0;
	return (0);
}

/** lfc_guidforpfn : Get the guid for a replica.  If the replica does not
  exist, fail with ENOENT */
char *
lfc_guidforpfn (const char *pfn, char *errbuf, int errbufsz)
{
	char *p;
	struct lfc_filestatg statg;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (NULL);

	if (fcops.statr (pfn, &statg) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statr][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, pfn, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}
	if ((p = strdup (statg.guid)) == NULL) {
		errno = ENOMEM;
		return (NULL);
	}
	return (p);
}

/** lfc_guidsforpfns : Get the guid for a replica.  If the replica does not
  exist, fail with ENOENT */
lfc_guidsforpfns (int nbfiles, const char **pfns, int amode, char ***guids, int **statuses, char *errbuf, int errbufsz)
{
	int i, size;
	struct lfc_filestatg statg;
	char actual_pfn[GFAL_PATH_MAXLEN];
	struct lfc_linkinfo* linklist;

	if (nbfiles < 1 || pfns == NULL || guids == NULL) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_guidsforpfns][EINVAL] Invalid arguments");
		errno = EINVAL;
		return (-1);
	}

	*guids = NULL;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if ((*guids = (char **) calloc (nbfiles + 1, sizeof (char *))) == NULL ||
			(*statuses = (int *) calloc (nbfiles, sizeof (int))) == NULL)
		return (-1);

	if (fcops.startsess (lfc_endpoint, (char*) gfal_version ()) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_startsess][] %s: %s",
				gfal_remote_type, lfc_endpoint, fcops.sstrerror (*fcops.serrno));
		free (guids);
		*guids = NULL;
		free (*statuses);
		*statuses = NULL;
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	for (i = 0; i < nbfiles; ++i) {
		if (purify_surl (pfns[i], actual_pfn, GFAL_PATH_MAXLEN) < 0) {
			(*statuses)[i] = errno;
			continue;
		}
		if (fcops.statr (actual_pfn, &statg) < 0) {
			(*statuses)[i] = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			(*guids)[i] = NULL;
			continue;
		}
		if (((*guids)[i] = strdup (statg.guid)) == NULL) {
			int j;
			for (j = 0; j < i; ++j)
				if ((*guids)[j]) free ((*guids)[j]);
			free (*guids);
			*guids = NULL;
			free (*statuses);
			*statuses = NULL;
			errno = ENOMEM;
			return (-1);
		}
		
		size = 0;
		linklist = NULL;

		if (fcops.getlinks (NULL, (*guids)[i], &size, &linklist) < 0 || size <= 0 || linklist == NULL) {
			(*statuses)[i] = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			free ((*guids)[i]);
			(*guids)[i] = NULL;
			continue;
		}
		if (fcops.access (linklist[0].path, amode) < 0) {
			(*statuses)[i] = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			free (linklist);
			free ((*guids)[i]);
			(*guids)[i] = NULL;
			continue;
		}
		free (linklist);
	}

	if (fcops.endsess () < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_endsess][] %s: %s",
				gfal_remote_type, lfc_endpoint, fcops.sstrerror (*fcops.serrno));
	}
	return (0);
}

int
lfc_guid_exists (const char *guid, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statg;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.statg (NULL, guid, &statg) < 0) {
		if (*fcops.serrno == ENOENT) 
			return (0);

		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statg][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, guid, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}
	return (1);
}

char **
lfc_get_replicas (const char * lfn, const char *guid, char *errbuf, int errbufsz)
{
	int i, size = 0;
	struct lfc_filereplica* list = NULL;
	char **replicas = NULL;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (NULL);

	if (fcops.getreplica (lfn, guid, NULL, &size, &list) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getreplica][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, lfn ? lfn : guid, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}
	/* no results */
	if (size < 0 || (size > 0 && list == NULL)) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getreplica][] %s: %s: Unknown error",
				gfal_remote_type, lfc_endpoint, lfn ? lfn : guid);
		errno = ECOMM;
		if (list) free (list);
		return (NULL);
	}

	if (size > 0) {
		replicas = (char **) calloc (size + 1, sizeof (char *));
		if (replicas == NULL) {
			free (list);
			return (NULL);
		}

		for (i = 0; i < size; ++i) {
			if (list[i].sfn)
				replicas[i] = strdup (list[i].sfn);
		}
		replicas[i] = NULL;
	}

	if (list) free (list);
	errno = 0;
	return (replicas);
}

/* Unregister replicas (SURLs) from a GUIDs, and remove all links to a GUID
 * if there are no more replicas */
	int
lfc_unregister_pfns (int nbguids, const char **guids, const char **pfns, int **results, char *errbuf, int errbufsz)
{
	int i, j, size, rc;
	struct lfc_filereplica* replist;
	struct lfc_linkinfo* linklist;
	char *lfn;
	char surl_cat[GFAL_PATH_MAXLEN];

	if (nbguids < 1 || guids == NULL || pfns == NULL || results == NULL) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_unregister_pfns][EINVAL] Invalid arguments");
		errno = EINVAL;
		return (-1);
	}

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if ((*results = (int *) calloc (nbguids, sizeof (int))) == NULL)
			return (-1);

	if (fcops.startsess (lfc_endpoint, "") < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_startsess][] %s: %s",
				gfal_remote_type, lfc_endpoint, fcops.sstrerror (*fcops.serrno));
		free (*results);
		*results = NULL;
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	for (i = 0; i < nbguids; ++i) {
		if (guids[i] == NULL || pfns[i] == NULL)
			continue;
		if (purify_surl (pfns[i], surl_cat, GFAL_PATH_MAXLEN) < 0) {
			(*results)[i] = EINVAL;
			continue;
		}
		rc = fcops.delreplica (guids[i], NULL, surl_cat);
		if (rc < 0 && *fcops.serrno == ENOENT) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
					"[GFAL][lfc_unregister_pfns][] Failed removing SURL without SRM-version-specific information, trying with full SURL...\n");
			rc = fcops.delreplica (guids[i], NULL, pfns[i]);
		}
		if (rc < 0) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_delreplica][] %s: %s: %s",
					gfal_remote_type, pfns[i], lfc_endpoint, fcops.sstrerror (*fcops.serrno));
			(*results)[i] = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		} else {
			(*results)[i] = 0;
			gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[guid:%s] %s - UNREGISTERED", guids[i], pfns[i]);
		}

		/* Let's check if there are more replicas */
		size = 0;
		replist = NULL;

		if (fcops.getreplica (NULL, guids[i], NULL, &size, &replist) < 0) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getreplica][] %s: %s: %s",
					gfal_remote_type, guids[i], lfc_endpoint, fcops.sstrerror (*fcops.serrno));
			continue;
		}
		if (replist) free (replist);
		if (size > 0)
			/* there are still some replicas, we don't need to do anything else */
			continue;

		/* at that stage, we know that guids[i] has no replicas */
		size = 0;
		linklist = NULL;

		if (fcops.getlinks (NULL, guids[i], &size, &linklist) < 0) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getlinks][] %s: %s: %s",
					gfal_remote_type, guids[i], lfc_endpoint, fcops.sstrerror (*fcops.serrno));
			continue;
		}
		if (size <= 0 || linklist == NULL)
			/* there are no aliases to unregister!? */
			continue;

		for (j = 0; j < size; ++j) {
			lfn = linklist[j].path;
			if (!lfn) continue;

			if (fcops.unlink (lfn) < 0) {
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_unlink][] %s: %s: %s",
						gfal_remote_type, lfn, lfc_endpoint, fcops.sstrerror (*fcops.serrno));
			}

			gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[guid:%s] lfn:%s - UNREGISTERED", guids[i], lfn);
		}
		free (linklist);
	}

	if (fcops.endsess () < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_endsess][] %s: %s",
				gfal_remote_type, lfc_endpoint, fcops.sstrerror (*fcops.serrno));
	}

	return (0);
}

char *
lfc_guidfromlfn (const char *lfn, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statg;
	char *p;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (NULL);

	if (fcops.statg (lfn, NULL, &statg) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statg][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, lfn, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}
	if ((p = strdup (statg.guid)) == NULL)
		errno = ENOMEM;
	return (p);
}

char **
lfc_get_aliases (const char *lfn, const char *guid, char *errbuf, int errbufsz)
{
	struct lfc_linkinfo* list = NULL;
	int i, size = 0;
	char **lfns = NULL;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (NULL);

	if (fcops.getlinks (lfn, guid, &size, &list) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getlinks][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, guid, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}
	/* no results */
	if (size <= 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getlinks][] %s: %s: No such GUID",
				gfal_remote_type, lfc_endpoint, guid);
		errno = ENOENT;
		if (list) free (list);
		return (NULL);
	} else if (list == NULL) {
		errno = ENOMEM;
		return (NULL);
	}

	lfns = (char **) calloc (size + 1, sizeof (char *));
	if (lfns == NULL) {
		free (list);
		return (NULL);
	}

	for (i = 0; i < size; ++i) {
		if (list[i].path)
			lfns[i] = strdup (list[i].path);
	}
	lfns[i] = NULL;
	free (list);

	return (lfns);
}

int
lfc_register_file (const char *lfn, const char *guid, const char *surl, mode_t mode,
		GFAL_LONG64 size, int bool_createonly, char *errbuf, int errbufsz)
{
	int sav_errno = 0, bool_exists = 0;
	char *matching_guid = NULL, *generated_guid = NULL, *generated_lfn = NULL;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	fcops.starttrans (NULL, (char*) gfal_version ());

	if (lfn || guid) {
		struct lfc_filestatg statg;

		if (fcops.statg (lfn, guid, &statg) < 0) {
			if ((sav_errno = *fcops.serrno) == ENOENT) {
				// lfn and guid have to be generated if == NULL
				sav_errno = 0;

				if (!lfn && (lfn = generated_lfn = gfal_generate_lfn (errbuf, errbufsz)) == NULL)
					sav_errno = errno > 0 ? errno : EINVAL;
				else if (!guid && (guid = generated_guid = gfal_generate_guid (errbuf, errbufsz)) == NULL)
					sav_errno = errno > 0 ? errno : EINVAL;

				if (sav_errno) {
					fcops.endtrans ();
					errno = sav_errno;
					return (-1);
				}
			} else {
				if (lfn)
					gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statg][] %s: lfn:%s: %s",
							gfal_remote_type, lfc_endpoint, lfn, fcops.sstrerror (sav_errno));
				else
					gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statg][] %s: guid:%s: %s",
							gfal_remote_type, lfc_endpoint, guid, fcops.sstrerror (sav_errno));

				fcops.endtrans ();
				errno = sav_errno < 1000 ? sav_errno : ECOMM;
				return (-1);
			}
		} else {
			if (bool_createonly) {
				if (lfn)
					gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statg][] %s: lfn:%s: %s",
							gfal_remote_type, lfc_endpoint, lfn, strerror (EEXIST));
				else
					gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statg][] %s: guid:%s: %s",
							gfal_remote_type, lfc_endpoint, guid, strerror (EEXIST));

				fcops.endtrans ();
				errno = EEXIST;
				return (-1);
			}
			if (!guid && (guid = matching_guid = strdup (statg.guid)) == NULL) {
				sav_errno = errno;
				fcops.endtrans ();
				errno = sav_errno;
				return (-1);
			}
			bool_exists = 1;
		}
	} else {
		if ((lfn = generated_lfn = gfal_generate_lfn (errbuf, errbufsz)) == NULL)
			sav_errno = errno > 0 ? errno : EINVAL;
		if (lfn && (guid = generated_guid = gfal_generate_guid (errbuf, errbufsz)) == NULL)
			sav_errno = errno > 0 ? errno : EINVAL;

		if (sav_errno) {
			fcops.endtrans ();
			if (generated_lfn) free (generated_lfn);
			errno = sav_errno;
			return (-1);
		}
	}

	gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "Using LFN: lfn:%s", lfn);
	gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "Using GUID: guid:%s", guid);

	if (!bool_exists) {
		// new lfc entry has to be created...
		char *p;
		char dirpath[1104];
		snprintf (dirpath, sizeof (dirpath), "%s", lfn);
		if ((p = strrchr (dirpath, '/')) == NULL) {
			fcops.endtrans ();
			if (generated_lfn) free (generated_lfn);
			if (generated_guid) free (generated_guid);
			errno = EINVAL;
			return (-1);
		}
		*p = 0;

		if (lfc_mkdirp_trans (dirpath, 0775, errbuf, errbufsz, 0) < 0) {
			sav_errno = errno;
			fcops.aborttrans ();
			if (generated_lfn) free (generated_lfn);
			if (generated_guid) free (generated_guid);
			errno = sav_errno;
			return (-1);
		}

		gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "Registering LFN: %s (%s)", lfn, guid);
		if (fcops.creatg (lfn, guid, mode) < 0) {
			sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;

			if (!generated_guid)
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_creatg][] %s: guid:%s: %s",
						gfal_remote_type, lfc_endpoint, guid, fcops.sstrerror (*fcops.serrno));
			else
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_creatg][] %s: lfn:%s: %s",
						gfal_remote_type, lfc_endpoint, lfn, fcops.sstrerror (*fcops.serrno));

			fcops.aborttrans ();
			if (generated_lfn) free (generated_lfn);
			if (generated_guid) free (generated_guid);
			errno = sav_errno;
			return (-1);
		}

		if (generated_lfn) free (generated_lfn);

		if (size > 0 && fcops.setfsizeg (guid, size, NULL, NULL) < 0) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_setfsizeg][] %s: guid:%s: %s",
					gfal_remote_type, lfc_endpoint, guid, fcops.sstrerror (*fcops.serrno));
			sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			fcops.aborttrans ();
			if (generated_guid) free (generated_guid);
			errno = sav_errno;
			return (-1);
		}
	}

	if (surl) {
		char *hostname = NULL;
		gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "Registering SURL: %s (%s)", surl, guid);

		if ((hostname = gfal_get_hostname (surl, errbuf, errbufsz)) == NULL) {
			sav_errno = errno;
			fcops.aborttrans ();
			if (matching_guid) free (matching_guid);
			if (generated_guid) free (generated_guid);
			errno = sav_errno;
			return (-1);
		}
		if (fcops.addreplica (guid, NULL, hostname, surl, '-', '\0', NULL, NULL) < 0) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_addreplica][] %s: %s: %s",
					gfal_remote_type, lfc_endpoint, surl, fcops.sstrerror (*fcops.serrno));
			sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			free (hostname);
			fcops.aborttrans ();
			if (matching_guid) free (matching_guid);
			if (generated_guid) free (generated_guid);
			errno = sav_errno;
			return (-1);
		}
		if (hostname) free (hostname);
	}

	fcops.endtrans ();
	if (matching_guid) free (matching_guid);
	if (generated_guid) free (generated_guid);
	errno = 0;
	return (0);
}

int
lfc_register_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statg;
	char master_lfn[CA_MAXPATHLEN+1];
	int sav_errno = 0, rc = 0;

	if (guid == NULL || lfn == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	fcops.starttrans (NULL, (char*) gfal_version ());
	if (fcops.statg (NULL, guid, &statg) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_startrans][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, guid, fcops.sstrerror (*fcops.serrno));
		sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		rc = -1;
	}
	/* now we do a getpath () to get the master lfn */
	if (rc == 0 && fcops.getpath (lfc_endpoint, statg.fileid, master_lfn) <0 ) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_getpath][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, guid, fcops.sstrerror (*fcops.serrno));
		sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		rc = -1;
	}

	/* and finally register */
	if (rc == 0 && fcops.symlink (master_lfn, lfn) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_symlink][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, lfn, fcops.sstrerror (*fcops.serrno));
		sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		rc = -1;
	}

	if (rc == 0)
		fcops.endtrans ();
	else
		fcops.aborttrans ();
	errno = sav_errno;
	return (rc);
}

int 
lfc_unregister_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statg;
	struct lfc_filestat stat;
	int sav_errno = 0, rc = 0;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	fcops.starttrans (NULL, (char*) gfal_version ());
	/*  In the case of the master lfn being unlinked already, statg will
		return ENOENT.  We then check lstat in case it's a hanging link ?  */
	if (fcops.statg (lfn, guid, &statg) < 0 ) {
		if (*fcops.serrno == ENOENT) {
			if (fcops.lstat (lfn, &stat) < 0 ) {
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_lstat][] %s: %s: %s",
						gfal_remote_type, lfc_endpoint, lfn, fcops.sstrerror (*fcops.serrno));
				sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
				rc = -1;
			}
		} else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statg][] %s: %s: %s",
					gfal_remote_type, lfc_endpoint, lfn, fcops.sstrerror (*fcops.serrno));
			sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			rc = -1;
		}
	}

	/* lfn maps to the guid - unlink it */
	if (rc == 0 && fcops.unlink (lfn) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_unlink][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, lfn, fcops.sstrerror (*fcops.serrno));
		sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		rc = -1;
	}

	if (rc == 0) {
		gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[guid:%s] lfn:%s - UNREGISTERED", guid, lfn);
		fcops.endtrans ();
	}
	else
		fcops.aborttrans ();

	errno = sav_errno;
	return (0);
}

int 
lfc_mkdirp (const char *path, mode_t mode, char *errbuf, int errbufsz)
{
	return (lfc_mkdirp_trans (path, mode, errbuf, errbufsz, 1));
}

int 
lfc_mkdirp_trans (const char *path, mode_t mode, char *errbuf, int errbufsz, int bool_starttrans)
{
	int sav_errno = 0, rc = 0;
	char *lastslash = NULL;
	char *p, *q;
	char sav_path[CA_MAXPATHLEN+1];
	struct lfc_filestatg statbuf;
	uuid_t uuid;
	char uuid_buf[CA_MAXGUIDLEN+1];

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if (strlen (path) >= sizeof (sav_path)) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_mkdirp][ENAMETOOLONG] %s: Path too long", path);
		errno = ENAMETOOLONG;
		return (-1);
	}
	if (path[0] != '/') {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][lfc_mkdirp][EINVAL] %s: Invalid path", path);
		errno = EINVAL;
		return (-1);
	}
	strcpy (sav_path, path);

	if (bool_starttrans)
		fcops.starttrans (NULL, (char*) gfal_version ());

	if (fcops.statg (sav_path, NULL, &statbuf) == 0) {
		/* the directoty already exists, nothing to do... */
		if (bool_starttrans)
			fcops.endtrans ();
		return (0);
	}

	p = strrchr (sav_path, '/');
	lastslash = p;
	while (rc == 0 && p > sav_path) {
		*p = '\0';
		if (fcops.statg (sav_path, NULL, &statbuf) == 0)
			break;
		if (*fcops.serrno != ENOENT) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statg][] %s: %s: %s",
					gfal_remote_type, lfc_endpoint, sav_path, fcops.sstrerror (*fcops.serrno));
			sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			rc = -1;
		} else {
			q = strrchr (sav_path, '/');
			*p = '/';
			p = q;
		}
	}
	while (rc == 0 && p) {
		*p = '/';
		if ((p = strchr (p + 1, '/')))
			*p = '\0';
		uuid_generate (uuid);
		uuid_unparse (uuid, uuid_buf);
		rc = fcops.mkdirg (sav_path, uuid_buf, mode);
		if (rc != 0) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_mkdirg][] %s: %s: %s",
					gfal_remote_type, lfc_endpoint, sav_path, fcops.sstrerror (*fcops.serrno));
			sav_errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		}
	}

	if (bool_starttrans) {
		if (rc == 0)
			fcops.endtrans ();
		else
			fcops.aborttrans ();
	}
	errno = sav_errno;
	return (rc);
}

int
lfc_renamel (const char *old_name, const char *new_name, char *errbuf, int errbufsz)
{
	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.rename (old_name, new_name) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_rename][] %s: %s",
				gfal_remote_type, lfc_endpoint, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	return (0);
}

DIR *
lfc_opendirlg (const char *dirname, const char *guid, char *errbuf, int errbufsz)
{
	lfc_DIR *dir;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (NULL);

	if ((dir = fcops.opendirg (dirname, guid)) == NULL) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_opendirg][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, dirname, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}

	return ((DIR *)dir);
}

int
lfc_rmdirl (const char *dirname, char *errbuf, int errbufsz)
{
	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.rmdir (dirname) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_rmdir][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, dirname, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	return (0);
}

int
lfc_maperror (struct proto_ops *pops, int ioflag)
{
	return (*fcops.serrno < 1000 ? *fcops.serrno : ECOMM);
}

int
lfc_setsize (const char *lfn, GFAL_LONG64 size, char *errbuf, int errbufsz)
{
	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.setfsize (lfn, NULL, size) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_setfsize][] %s: %s: %s",
				gfal_remote_type, lfc_endpoint, lfn, fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	return (0);
}

int
lfc_statl (const char *lfn, const char *guid, struct stat64 *buf, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statbuf;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.statg (lfn, guid, &statbuf) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_statg][] %s: %s: %s",
				gfal_remote_type,
				lfc_endpoint,
				lfn != NULL ? lfn : guid,
				fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	buf->st_mode = statbuf.filemode;
	buf->st_nlink = statbuf.nlink;
	buf->st_uid = statbuf.uid;
	buf->st_gid = statbuf.gid;
	buf->st_size = statbuf.filesize;
	buf->st_atime = statbuf.atime;
	buf->st_ctime = statbuf.ctime;
	buf->st_mtime = statbuf.mtime;

	return (0);
}

int
lfc_lstatl (const char *lfn, struct stat64 *buf, char *errbuf, int errbufsz)
{
	struct lfc_filestat statbuf;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.lstat (lfn, &statbuf) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_lstat][] %s: %s: %s",
				gfal_remote_type,
				lfc_endpoint,
				lfn,
				fcops.sstrerror (*fcops.serrno));
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	buf->st_mode = statbuf.filemode;
	buf->st_nlink = statbuf.nlink;
	buf->st_uid = statbuf.uid;
	buf->st_gid = statbuf.gid;
	buf->st_size = statbuf.filesize;
	buf->st_atime = statbuf.atime;
	buf->st_ctime = statbuf.ctime;
	buf->st_mtime = statbuf.mtime;

	return (0);
}

int
lfc_fillsurls (gfal_file gf, char *errbuf, int errbufsz)
{
	int size = 0;
	struct lfc_filereplica* list = NULL;
	char errmsg[GFAL_ERRMSG_LEN];

	if (gf == NULL || (gf->lfn == NULL && gf->guid == NULL)) {
		errno = EINVAL;
		return (-1);
	}

	if (lfc_init (errmsg, GFAL_ERRMSG_LEN) < 0) {
		gf->errmsg = strdup (errmsg);
		gf->errcode = errno;
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, errmsg);
		return (-1);
	}

	if (fcops.startsess (lfc_endpoint, (char*) gfal_version ()) < 0) {
		gf->errcode = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		asprintf (&(gf->errmsg), "[%s][lfc_startsess][] %s: %s", gfal_remote_type, lfc_endpoint, fcops.sstrerror (*fcops.serrno));
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, gf->errmsg);
		return (-1);
	}

	if (gf->guid == NULL) {
		struct lfc_filestatg statg;

		if (fcops.statg (gf->lfn, NULL, &statg) < 0) {
			gf->errcode = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			asprintf (&(gf->errmsg), "[%s][lfc_statg][] %s: %s: %s", gfal_remote_type,
					lfc_endpoint, gf->lfn, fcops.sstrerror (*fcops.serrno));
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, gf->errmsg);
			fcops.endsess ();
			return (-1);
		}
		if (S_ISDIR (statg.filemode)) {
			gf->errcode = EISDIR;
			fcops.endsess ();
			return (-1);
		}
		gf->guid = strdup (statg.guid);
		if (gf->guid == NULL) {
			gf->errcode = errno;
			fcops.endsess ();
			return (-1);
		}
	}

	if (fcops.getreplica (gf->lfn, gf->guid, NULL, &size, &list) < 0) {
		asprintf (&(gf->errmsg), "[%s][lfc_getreplica][] %s: %s: %s", gfal_remote_type, lfc_endpoint,
				gf->lfn ? gf->lfn : gf->guid, fcops.sstrerror (*fcops.serrno));
		gf->errcode = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, gf->errmsg);
		fcops.endsess ();
		return (-1);
	}
	/* no results */
	if (size < 0 || (size > 0 && list == NULL)) {
		asprintf (&(gf->errmsg), "[%s][lfc_getreplica][] %s: %s: Unknown error", gfal_remote_type, lfc_endpoint, gf->guid);
		gf->errcode = ECOMM;
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, gf->errmsg);
		if (list) free (list);
		fcops.endsess ();
		return (-1);
	}

	if (size > 0) {
		// Put SURLs in order of preference:
		//   1. SURLs from default SE
		//   2. SURLs from local domain
		//   3. Others
		char dname[GFAL_HOSTNAME_MAXLEN];
		int i, random_ind;
		int next_defaultse = 0, next_local = 0, next_others = 0;
		char  *surl, *surl_tmp1, *surl_tmp2, *p1, *p2, *p3;
		char *default_se;

		srand ((unsigned) time (NULL));
		*dname = '\0';
		getdomainnm (dname, sizeof (dname));

		gf->nbreplicas = size;
		gf->replicas = (gfal_replica *) calloc (size, sizeof (gfal_replica));
		if (gf->replicas == NULL) {
			gf->errcode = errno;
			free (list);
			fcops.endsess ();
			return (-1);
		}

		/* and get the default SE, it there is one */
		default_se = get_default_se (errmsg, GFAL_ERRMSG_LEN);

		for (i = 0; i < size; i++) {
			if ((surl = list[i].sfn) == NULL ||
					(strncmp (surl, "srm://", 6) && strncmp (surl, "sfn://", 6)))
				// skip entries not in the form srm: or sfn:
				continue;

			if ((p1 = strchr (surl + 6, '/')) == NULL) continue; // no host name
			if ((p2 = strchr (surl + 6, '.')) == NULL) continue; // no domain name
			*p1 = '\0';

			if ((p3 = strchr (surl + 6, ':')))
				// remove port number
				*p3 = '\0';

			if ((gf->replicas[i] = (gfal_replica) calloc (1, sizeof (struct _gfal_replica))) == NULL) {
				gf->errcode = errno;
				free (list);
				fcops.endsess ();
				return (-1);
			}

			if (default_se != NULL && strcmp (surl + 6, default_se) == 0) {
				// SURL from default SE
				// Normally only 1 SURL from default SE, so no need of randomize place
				*p1 = '/';
				if (p3) *p3 = ':';

				surl_tmp1 = gf->replicas[next_defaultse]->surl;
				surl_tmp2 = gf->replicas[next_local]->surl;
				gf->replicas[next_defaultse]->surl = strdup (surl);
				if (next_local > next_defaultse && surl_tmp1 && gf->replicas[next_local])
					gf->replicas[next_local]->surl = surl_tmp1;
				if (next_others > next_local && surl_tmp2 && gf->replicas[next_others])
					gf->replicas[next_others]->surl = surl_tmp2;

				++next_defaultse;
				++next_local;
				++next_others;
				continue;
			}

			if (strcmp (p2 + 1, dname) == 0) {
				// SURL from local domain
				// Put it at random place amongst local surls
				*p1 = '/';
				if (p3) *p3 = ':';
				random_ind = (rand () % (next_local - next_defaultse + 1)) + next_defaultse;

				surl_tmp1 = gf->replicas[random_ind]->surl;
				surl_tmp2 = gf->replicas[next_local]->surl;
				gf->replicas[random_ind]->surl = strdup (surl);
				if (next_local > random_ind && surl_tmp1 && gf->replicas[next_local])
					gf->replicas[next_local]->surl = surl_tmp1;
				if (next_others > next_local && surl_tmp2 && gf->replicas[next_others])
					gf->replicas[next_others]->surl = surl_tmp2;

				++next_local;
				++next_others;
				continue;
			}

			// Other SURL...
			// Put it at random place amongst other surls
			*p1 = '/';
			if (p3) *p3 = ':';
			random_ind = (rand () % (next_others - next_local + 1)) + next_local;

			surl_tmp1 = gf->replicas[random_ind]->surl;
			gf->replicas[random_ind]->surl = strdup (surl);
			if (surl_tmp1 && gf->replicas[next_others])
				gf->replicas[next_others]->surl = surl_tmp1;

			++next_others;
		}
	}

	if (list) free (list);
	fcops.endsess ();
	errno = 0;
	return (0);
}

	int
lfc_remove (gfal_file gfile, char *errbuf, int errbufsz)
{
	int i, nberrors = 0;
	char errmsg[GFAL_ERRMSG_LEN];

	if (gfile == NULL || gfile->guid == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (lfc_init (errmsg, GFAL_ERRMSG_LEN) < 0) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, errmsg);
		gfile->errcode = errno;
		gfile->errmsg = strdup (errmsg);
		return (-1);
	}

	if (fcops.startsess (lfc_endpoint, (char *) gfal_version ()) < 0) {
		gfile->errcode = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		snprintf (errmsg, GFAL_ERRMSG_LEN, "[%s][lfc_startsess][] %s: %s", gfal_remote_type, lfc_endpoint, fcops.sstrerror (*fcops.serrno));
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, errmsg);
		gfile->errmsg = strdup (errmsg);
		return (-1);
	}

	for (i = 0; i < gfile->nbreplicas; ++i) {
		if (gfile->replicas[i] == NULL ||
				gfile->replicas[i]->surl == NULL ||
				gfile->replicas[i]->errcode != 0)
			continue;

		if (fcops.delreplica (gfile->guid, NULL, gfile->replicas[i]->surl) < 0) {
			++nberrors;
			snprintf (errmsg, GFAL_ERRMSG_LEN, "[%s][lfc_delreplica][] %s: %s: %s", gfal_remote_type,
					lfc_endpoint, gfile->replicas[i]->surl, fcops.sstrerror (*fcops.serrno));
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, errmsg);
			gfal_file_set_replica_error (gfile, *fcops.serrno < 1000 ? *fcops.serrno : ECOMM, errmsg);
		}
		gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[INFO] %s - %s > UNREGISTERED REPLICA",
				gfile->lfn ? gfile->lfn : gfile->guid, gfile->replicas[i]->surl);
	}

	if (gfile->nberrors == 0) {
		// If all replicas have been removed and unregister, then we can remove the links
		int size = 0;
		struct lfc_linkinfo* linklist = NULL;

		if (fcops.getlinks (gfile->lfn, gfile->guid, &size, &linklist) < 0) {
			++nberrors;
			gfile->errcode = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			snprintf (errmsg, GFAL_ERRMSG_LEN, "[%s][lfc_getlinks][] %s: %s: %s", gfal_remote_type,
					lfc_endpoint, gfile->lfn ? gfile->lfn : gfile->guid,
					fcops.sstrerror (*fcops.serrno));
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, errmsg);
			gfile->errmsg = strdup (errmsg);
		}
		else if (size > 0 && linklist != NULL) {
			for (i = 0; i < size; ++i) {
				if (!linklist[i].path)
					continue;
				if (fcops.unlink (linklist[i].path) < 0) {
					++nberrors;
					gfile->errcode = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
					gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[%s][lfc_unlink][] %s: %s: %s", gfal_remote_type,
							lfc_endpoint, gfile->lfn ? gfile->lfn : gfile->guid,
							fcops.sstrerror (*fcops.serrno));
				}
				gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[INFO] %s - %s > UNREGISTERED ALIAS",
						gfile->lfn ? gfile->lfn : gfile->guid, linklist[i].path);
			}
			free (linklist);
			if (gfile->errcode > 0)
				asprintf (&(gfile->errmsg), "[GFAL][lfc_remove][] %s: not able to remove all aliases",
						gfile->lfn ? gfile->lfn : gfile->guid);
		}
	}

	fcops.endsess ();
	return (nberrors ? -1 : 0);
}
