/*
 * Copyright (C) 2004 by CERN
 */

/*
 * @(#)$RCSfile: lfc_ifce.c,v $ $Revision: 1.49 $ $Date: 2008/03/07 12:52:45 $ CERN James Casey
 */
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "lfc_api.h"
#include "gfal_api.h"
#include "serrno.h"

#define ALLOC_BLOCK_SIZE 16 /* the block size to allocate new pointers in */

struct fc_ops {
	int	*serrno;
	char	*(*sstrerror)(int);
	int	(*addreplica)(const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *);
	int	(*creatg)(const char *, const char *, mode_t);
	int	(*delreplica)(const char *, struct lfc_fileid *, const char *);
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
char *lfc_host = NULL;

static char lfc_env[128];
static char lfc_penv[128];

/** extract a hostname from a SURL.  We search for "://" to get the start of
  the hostname.  Then we keep going to the next slash, colon or end of the
  SURL. */
static  char *
get_hostname(const char *path, char *errbuf, int errbufsz) {
	char *start;
	char *cp;
	char *result;
	char c;
	char sav_path[CA_MAXSFNLEN+1];

	strcpy(sav_path, path);

	start = strchr(sav_path, ':');
	if ( start == NULL || *(start+1) != '/' || *(start+2) != '/') {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid syntax", path);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (NULL);
	}
	/* point start to beginning of hostname */
	start += 3;
	for(cp = start; *cp != '\0' && *cp != ':' && *cp != '/'; cp++) ;

	c = *cp;
	*cp = '\0';
	if((result = strdup(start)) == NULL) {
		errno = ENOMEM;
		return (NULL);
	}
	*cp = c;
	return result;
}

static int 
lfc_init (char *errbuf, int errbufsz) {
	char *lfc_endpoint = NULL;
	char *p;
	char *lfc_port = NULL;
	char errmsg[ERRMSG_LEN];

	if (lfc_host == NULL) {
		/* Try first from env */
		if((lfc_host = getenv("LFC_HOST")) != NULL) {
			lfc_port = getenv("LFC_PORT");
		} else { /* get endpoint from MDS */
			if(get_lfc_endpoint (&lfc_endpoint, errbuf, errbufsz) < 0)
				return (-1);

			if (strncmp(lfc_endpoint, "lfc://", 6) == 0) {
				if((lfc_host = strdup(lfc_endpoint + 6)) == NULL) {
					free(lfc_endpoint);
					errno = ENOMEM;
					return (-1);
				}
			} else { /* just a plain hostname */
				lfc_host = lfc_endpoint;
			}

			if ((lfc_port = strchr (lfc_host, ':')) != NULL) {
				*lfc_port = '\0';
				lfc_port++;
			}
		}
		if(strlen(lfc_host) == 0) {
			gfal_errmsg(errbuf, errbufsz, "LFC host is set to empty string");
			free(lfc_endpoint);
			lfc_host = NULL;
			errno = EINVAL;
			return (-1);
		}

		if (10 + strlen(lfc_host) + 6 > 128) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Host name too long", lfc_host);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			free(lfc_endpoint);
			lfc_host = NULL;
			errno = ENAMETOOLONG;
			return (-1);
		}
		sprintf(lfc_env, "LFC_HOST=%s", lfc_host);
		if(putenv(lfc_env) < 0)
			return (-1);

		if(lfc_port && *lfc_port != '\0') {
			if(strlen(lfc_port) > 5) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid LFC port number", lfc_port);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				free(lfc_endpoint);
				lfc_host = NULL;
				errno = EINVAL;
				return (-1);
			}
			sprintf(lfc_penv, "LFC_PORT=%s", lfc_port);
			if(putenv(lfc_penv) < 0) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Host name too long");
				gfal_errmsg(errbuf, errbufsz, "Cannot set environment variable LFC_HOST");
				free(lfc_endpoint);
				lfc_host = NULL;
				return (-1);
			}
		}
		{
			void *dlhandle;

			if ((dlhandle = dlopen (NULL, RTLD_LAZY)) == NULL) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s", dlerror ());
				gfal_errmsg(errbuf, errbufsz, errmsg);
				if (lfc_endpoint) free(lfc_endpoint);
				lfc_host = NULL;
				return (-1);
			}
			
			fcops.addreplica = (int (*) (const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *)) dlsym (dlhandle, "lfc_addreplica");

			if (fcops.addreplica == NULL) {
				if ((dlhandle = dlopen ("liblfc.so", RTLD_LAZY)) == NULL) {
					snprintf (errmsg, ERRMSG_LEN - 1, "liblfc.so: %s", dlerror ());
					gfal_errmsg(errbuf, errbufsz, errmsg);
					if (lfc_endpoint) free(lfc_endpoint);
					lfc_host = NULL;
					return (-1);
				}

				fcops.addreplica = (int (*) (const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *)) dlsym (dlhandle, "lfc_addreplica");
			}

			fcops.serrno = (int *) dlsym (dlhandle, "serrno");
			fcops.sstrerror = (char * (*) (int)) dlsym (dlhandle, "sstrerror");
			fcops.creatg = (int (*) (const char *, const char *, mode_t)) dlsym (dlhandle, "lfc_creatg");
			fcops.delreplica = (int (*) (const char *, struct lfc_fileid *, const char *)) dlsym (dlhandle, "lfc_delreplica");
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
	fcops.seterrbuf(errbuf, errbufsz);
	return (0);
}

char *
lfc_get_catalog_endpoint(char *errbuf, int errbufsz) {
	if(lfc_init(errbuf, errbufsz) < 0)
		return (NULL);
	return lfc_host;
}

int
lfc_replica_exists(const char *guid, char *errbuf, int errbufsz) {
	int size = 0;
	struct lfc_filereplica* replicas = NULL;

	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);
	if (fcops.getreplica (NULL, guid, NULL, &size, &replicas) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, guid, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	free (replicas);
	return (size > 0 ? 1 : 0);
}

int
lfc_getfilesizeg(const char *guid, GFAL_LONG64 *sizep, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statg;

	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if(fcops.statg(NULL, guid, &statg) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, guid, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	*sizep = (u_signed64) statg.filesize;
	return (0);
}

int
lfc_accessl (const char *path, int mode, char *errbuf, int errbufsz)
{
	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.access (path, mode) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, path, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	return (0);
}

int
lfc_chmodl (const char *path, mode_t mode, char *errbuf, int errbufsz)
{
	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.chmod (path, mode) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, path, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	return (0);
}

/** lfc_guidforpfn : Get the guid for a replica.  If the replica does not
  exist, fail with ENOENT */
char *
lfc_guidforpfn (const char *pfn, char *errbuf, int errbufsz)
{
	char *p;
	struct lfc_filestatg statg;

	if(lfc_init(errbuf, errbufsz) < 0)
		return (NULL);

	if(fcops.statr(pfn, &statg) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, pfn, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}
	if((p = strdup(statg.guid)) == NULL) {
		errno = ENOMEM;
		return (NULL);
	}
	return (p);
}

/** lfc_guidsforpfns : Get the guid for a replica.  If the replica does not
  exist, fail with ENOENT */
lfc_guidsforpfns (int nbfiles, const char **pfns, char ***guids, int **statuses, char *errbuf, int errbufsz)
{
	int i;
	char *p;
	struct lfc_filestatg statg;
	char errmsg[ERRMSG_LEN];

	if (nbfiles < 1 || pfns == NULL || guids == NULL) {
		gfal_errmsg(errbuf, errbufsz, "Function 'lfc_guidsforpfns': invalid arguments");
		errno = EINVAL;
		return (-1);
	}

	*guids = NULL;

	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if ((*guids = (char **) calloc (nbfiles + 1, sizeof (char *))) == NULL ||
			(*statuses = (int *) calloc (nbfiles, sizeof (int))) == NULL)
		return (-1);

	if (fcops.startsess (lfc_host, (char*) gfal_version ()) < 0) {
		snprintf (errmsg, ERRMSG_LEN, "%s: %s", lfc_host, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		free (guids);
		*guids = NULL;
		free (*statuses);
		*statuses = NULL;
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	for (i = 0; i < nbfiles; ++i) {
		if(fcops.statr(pfns[i], &statg) < 0) {
			(*statuses)[i] = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			(*guids)[i] = NULL;
		}
		else if(((*guids)[i] = strdup(statg.guid)) == NULL) {
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
	}

	if (fcops.endsess () < 0) {
		snprintf (errmsg, ERRMSG_LEN, "%s: %s", lfc_host, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
	}
	return (0);
}

int
lfc_guid_exists (const char *guid, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statg;

	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if(fcops.statg(NULL, guid, &statg) < 0) {
		if(*fcops.serrno == ENOENT) 
			return (0);

		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, guid, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}
	return (1);
}

int
lfc_register_pfn (const char *guid, const char *pfn, char *errbuf, int errbufsz)
{
	char *hostname;

	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if((hostname = get_hostname(pfn, errbuf, errbufsz)) == NULL) {
		return (-1);
	}
	/* We always have available permanent files at the minute */
	if(fcops.addreplica(guid, NULL, hostname, pfn, '-', '\0', NULL, NULL) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s", lfc_host, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		free(hostname);
		return (-1);
	}
	free(hostname);
	return (0);
}

char **
lfc_surlsfromguid (const char *guid, char *errbuf, int errbufsz)
{
	int i, size = 0;
	struct lfc_filereplica* list = NULL;
	char **replicas = NULL;

	if(lfc_init(errbuf, errbufsz) < 0)
		return (NULL);

	if (fcops.getreplica (NULL, guid, NULL, &size, &list) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, guid, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}
	/* no results */
	if (size <= 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: No such GUID", guid);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ENOENT;
		if (list) free (list);
		return (NULL);
	} else if (list == NULL) {
		errno = ENOMEM;
		return (NULL);
	}

	replicas = (char **) calloc (size + 1, sizeof (char *));
	if (replicas == NULL) {
		free (list);
		return (NULL);
	}

	for (i = 0; i < size; ++i) {
		if (list[i].sfn)
			replicas[i] = strdup (list[i].sfn);
	}
	free (list);
	replicas[i] = NULL;

	return (replicas);
}

char *
lfc_surlfromguid (const char *guid, char *errbuf, int errbufsz)
{
	char **surls;
	char **cp;
	char *result;
	int size = 0;

	if((surls = lfc_surlsfromguid(guid, errbuf, errbufsz)) == NULL) {
		return (NULL);
	} else if (*surls == NULL) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: No such GUID", guid);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ENOENT;
		return (NULL);
	}

	while (surls[size] != NULL) ++size;
	result = getbestfile(surls, size, errbuf, errbufsz);

	for(cp = surls; *cp != NULL; ++cp) {
		if(*cp != result) {
			free (*cp);
		}
	}
	free (surls);
	return (result);
}

/* Unregister replicas (SURLs) from a GUIDs, and remove all links to a GUID
 * if there are no more replicas */
	int
lfc_unregister_pfns (int nbguids, const char **guids, const char **pfns, int verbose, int **results, char *errbuf, int errbufsz)
{
	int i, j, size;
	struct lfc_filereplica* replist;
	struct lfc_linkinfo* linklist;
	char *lfn;
	char errmsg[ERRMSG_LEN];

	if (nbguids < 1 || guids == NULL || pfns == NULL || results == NULL) {
		gfal_errmsg(errbuf, errbufsz, "Function 'lfc_unregister_pfns': invalid arguments");
		errno = EINVAL;
		return (-1);
	}

	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if ((*results = (int *) calloc (nbguids, sizeof (int))) == NULL)
			return (-1);

	if (fcops.startsess (lfc_host, "") < 0) {
		snprintf (errmsg, ERRMSG_LEN, "%s: %s", lfc_host, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		free (*results);
		*results = NULL;
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	for (i = 0; i < nbguids; ++i) {
		if(fcops.delreplica(guids[i], NULL, pfns[i]) < 0) {
			snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", pfns[i], lfc_host, fcops.sstrerror(*fcops.serrno));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			(*results)[i] = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		} else {
			(*results)[i] = 0;
			if (verbose)
				printf ("[guid:%s] %s - UNREGISTERED\n", guids[i], pfns[i]);
		}

		/* Let's check if there are more replicas */
		size = 0;
		replist = NULL;

		if (fcops.getreplica (NULL, guids[i], NULL, &size, &replist) < 0) {
			snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", guids[i], lfc_host, fcops.sstrerror(*fcops.serrno));
			gfal_errmsg(errbuf, errbufsz, errmsg);
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
			snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", guids[i], lfc_host, fcops.sstrerror(*fcops.serrno));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			continue;
		}
		if (size <= 0 || linklist == NULL)
			/* there are no aliases to unregister!? */
			continue;

		for (j = 0; j < size; ++j) {
			lfn = linklist[j].path;
			if (!lfn) continue;

			if(fcops.unlink(lfn) < 0) {
				snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfn, lfc_host, fcops.sstrerror(*fcops.serrno));
				gfal_errmsg(errbuf, errbufsz, errmsg);
			}

			if (verbose)
				printf ("[guid:%s] lfn:%s - UNREGISTERED\n", guids[i], lfn);
		}
		free (linklist);
	}

	if (fcops.endsess () < 0) {
		snprintf (errmsg, ERRMSG_LEN, "%s: %s", lfc_host, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
	}

	return (0);
}

char *
lfc_guidfromlfn (const char *lfn, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statg;
	char *p;

	if(lfc_init(errbuf, errbufsz) < 0)
		return (NULL);

	if(fcops.statg(lfn, NULL, &statg) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, lfn, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}
	if((p = strdup(statg.guid)) == NULL)
		errno = ENOMEM;
	return (p);
}

char **
lfc_lfnsforguid (const char *guid, char *errbuf, int errbufsz)
{
	struct lfc_linkinfo* list = NULL;
	int i, size = 0;
	char **lfns = NULL;

	if (lfc_init (errbuf, errbufsz) < 0)
		return (NULL);

	if (fcops.getlinks (NULL, guid, &size, &list) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, guid, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}
	/* no results */
	if (size <= 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: No such GUID", guid);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ENOENT;
		if (list) free (list);
		return (NULL);
	} else if (list == NULL) {
		errno = ENOMEM;
		return (NULL);
	}

	lfns = (char **) calloc (size, sizeof (char *));
	if (lfns == NULL) {
		free (list);
		return (NULL);
	}

	for (i = 0; i < size; ++i) {
		if (list[i].path)
			lfns[i] = strdup (list[i].path);
	}
	free (list);

	return (lfns);
}

int
lfc_create_alias (const char *guid, const char *lfn, mode_t mode, GFAL_LONG64 size, char *errbuf, int errbufsz)
{
	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	fcops.starttrans(NULL, (char*) gfal_version ());
	if(fcops.creatg(lfn, guid, mode) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, lfn, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}
	if(fcops.setfsizeg(guid, size, NULL, NULL) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: :%s: %s", lfc_host, lfn, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}
	fcops.endtrans();
	return (0);
}

int
lfc_register_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statg;
	char master_lfn[CA_MAXPATHLEN+1];

	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	fcops.starttrans(NULL, (char*) gfal_version ());
	if(fcops.statg(NULL, guid, &statg) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, guid, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}
	/* now we do a getpath() to get the master lfn */
	if (fcops.getpath(lfc_host, statg.fileid, master_lfn) <0 ) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, guid, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	/* and finally register */
	if(fcops.symlink(master_lfn, lfn) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, lfn, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}
	fcops.endtrans();
	return (0);
}

int 
lfc_unregister_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statg;
	struct lfc_filestat stat;

	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	fcops.starttrans(NULL, (char*) gfal_version ());
	/*  In the case of the master lfn being unlinked already, statg will
		return ENOENT.  We then check lstat in case it's a hanging link ?  */
	if(fcops.statg(lfn, guid, &statg) < 0 ) {
		if (*fcops.serrno == ENOENT) {
			if(fcops.lstat(lfn, &stat) < 0 ) {
				char errmsg[ERRMSG_LEN];
				snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, lfn, fcops.sstrerror(*fcops.serrno));
				gfal_errmsg(errbuf, errbufsz, errmsg);
				errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
				return (-1);
			} else {
				/* all ok, continue */
			}
		} else {
			char errmsg[ERRMSG_LEN];
			snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, lfn, fcops.sstrerror(*fcops.serrno));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			return (-1);
		}
	}

	/* lfn maps to the guid - unlink it */
	if(fcops.unlink(lfn) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, lfn, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}
	fcops.endtrans();
	return (0);
}

int 
lfc_mkdirp(const char *path, mode_t mode, char *errbuf, int errbufsz)
{
	int c;
	char *lastslash = NULL;
	char *p;
	char *q;
	char sav_path[CA_MAXPATHLEN+1];
	struct lfc_filestatg statbuf;
	uuid_t uuid;
	char uuid_buf[CA_MAXGUIDLEN+1];

	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if (strlen (path) >= sizeof(sav_path)) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: Path too long", path);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	if (path[0] != '/') {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: Invalid path", path);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}
	strcpy (sav_path, path);
	if (fcops.statg (sav_path, NULL, &statbuf) == 0)
		/* the directoty already exists, nothing to do... */
		return (0);

	p = strrchr (sav_path, '/');
	lastslash = p;
	while (p > sav_path) {
		*p = '\0';
		c = fcops.statg (sav_path, NULL, &statbuf);
		if (c == 0)
			break;
		if (*fcops.serrno != ENOENT) {
			char errmsg[ERRMSG_LEN];
			snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, sav_path, fcops.sstrerror(*fcops.serrno));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
			return (c);
		}
		q = strrchr (sav_path, '/');
		*p = '/';
		p = q;
	}
	c = 0;
	while (c == 0 && p) {
		*p = '/';
		if ((p = strchr (p + 1, '/')))
			*p = '\0';
		uuid_generate(uuid);
		uuid_unparse(uuid, uuid_buf);
		c = fcops.mkdirg (sav_path, uuid_buf, mode);
		if(c != 0) {
			char errmsg[ERRMSG_LEN];
			snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, sav_path, fcops.sstrerror(*fcops.serrno));
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		}
	}
	return (c);
}

int
lfc_renamel (const char *old_name, const char *new_name, char *errbuf, int errbufsz)
{
	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.rename (old_name, new_name) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s", lfc_host, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	return (0);
}

DIR *
lfc_opendirlg (const char *dirname, const char *guid, char *errbuf, int errbufsz)
{
	lfc_DIR *dir;

	if (lfc_init(errbuf, errbufsz) < 0)
		return (NULL);

	if ((dir = fcops.opendirg (dirname, guid)) == NULL) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, dirname, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (NULL);
	}

	return ((DIR *)dir);
}

int
lfc_rmdirl (const char *dirname, char *errbuf, int errbufsz)
{
	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.rmdir (dirname) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, dirname, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
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
	if(lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.setfsize (lfn, NULL, size) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s", lfc_host, lfn, fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = *fcops.serrno < 1000 ? *fcops.serrno : ECOMM;
		return (-1);
	}

	return (0);
}

int
lfc_statl (const char *lfn, const char *guid, struct stat *buf, char *errbuf, int errbufsz)
{
	struct lfc_filestatg statbuf;

	if (lfc_init(errbuf, errbufsz) < 0)
		return (-1);

	if (fcops.statg (lfn, guid, &statbuf) < 0) {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN, "%s: %s: %s",
				lfc_host,
				lfn != NULL ? lfn : guid,
				fcops.sstrerror(*fcops.serrno));
		gfal_errmsg(errbuf, errbufsz, errmsg);
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
