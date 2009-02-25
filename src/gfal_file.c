/*
 * Copyright (C) 2008-2010 by CERN
 */

/*
 * @(#)$RCSfile: gfal_file.c,v $ $Revision: 1.3 $ $Date: 2009/02/25 13:38:08 $ CERN Remi Mollon
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include <uuid/uuid.h>
#include "gfal_internals.h"

static const char *gfal_file_catalog_names[] = {
	"UNKNOWN",
	"LFC",
	"EDG"
};


gfal_file
gfal_file_new (const char *file, const char *defproto, int bool_tobecreated, char *errbuf, int errbufsz) {
	gfal_file gf;
	char actual_file[1104];

	if (file == NULL)
		return (NULL);

	if (canonical_url (file, defproto, actual_file, sizeof (actual_file), errbuf, errbufsz) < 0)
		return (NULL);

	gf = (gfal_file) calloc (1, sizeof (struct _gfal_file));
	if (gf == NULL)
		return (NULL);

	if (strncmp (actual_file, "lfn:", 4) == 0) {
		gf->lfn = strdup (actual_file + 4);
		if (gf->lfn == NULL) {
			gfal_file_free (gf);
			return (NULL);
		}
		gf->file = gf->lfn;
	} else if (strncmp (actual_file, "guid:", 5) == 0) {
		/* we check the format of the given GUID */
		uuid_t uuid;
		if (uuid_parse (actual_file + 5, uuid) < 0) {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_file_new][EINVAL] %s: Invalid GUID format", file);
			errno = EINVAL; /* invalid guid */
			return (NULL);
		}

		gf->guid = strdup (actual_file + 5); /* removing 'guid:' */
		if (gf->guid == NULL) {
			gfal_file_free (gf);
			return (NULL);
		}
		gf->file = gf->guid;
	} else if (strncmp (actual_file, "srm:", 4) == 0 || strncmp (actual_file, "sfn:", 4) == 0) {
		gf->nbreplicas = 1;
		gf->replicas = (gfal_replica *) calloc (1, sizeof (gfal_replica));
		if (gf->replicas == NULL) {
			gfal_file_free (gf);
			return (NULL);
		}
		*(gf->replicas) = (gfal_replica) calloc (1, sizeof (struct _gfal_replica));
		if (*(gf->replicas) == NULL) {
			gfal_file_free (gf);
			return (NULL);
		}
		gf->replicas[0]->surl = strdup (actual_file);
		if (gf->replicas[0]->surl == NULL) {
			gfal_file_free (gf);
			return (NULL);
		}
		gf->file = gf->replicas[0]->surl;
	} else {
		gf->turl = strdup (actual_file);
		if (gf->turl == NULL) {
			gfal_file_free (gf);
			return (NULL);
		}
		gf->file = gf->turl;
	}

	if (gf->lfn != NULL || gf->guid != NULL) {
		char *cat_type;
		if (get_cat_type (&cat_type) < 0) {
			gfal_file_free (gf);
			return (NULL);
		}

		if (strcmp (cat_type, "lfc") == 0)
			gf->catalog = GFAL_FILE_CATALOG_LFC;
		else if (strcmp (cat_type, "edg") == 0)
			gf->catalog = GFAL_FILE_CATALOG_EDG;

		free (cat_type);
		if (gf->catalog == GFAL_FILE_CATALOG_UNKNOWN) {
			errno = EPROTONOSUPPORT;
			return (NULL);
		}

		if (gf->catalog == GFAL_FILE_CATALOG_LFC)
			lfc_fillsurls (gf, errbuf, errbufsz);
		else if (gf->catalog == GFAL_FILE_CATALOG_EDG) {
			if (gf->guid == NULL) {
				if ((gf->guid = rmc_guidfromlfn (gf->lfn, errbuf, errbufsz)) == NULL) {
					gfal_file_free (gf);
					return (NULL);
				}
			}
			lrc_fillsurls (gf);
		}
		else {
			gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_file_new][EPROTONOSUPPORT] File Catalog must be \"lfc\" or \"edg\"");
			errno = EPROTONOSUPPORT;
			return (NULL);
		}


		if (bool_tobecreated) {
			if (gf->nbreplicas > 0)
				gf->errcode = EEXIST;
			else if (gf->errcode == ENOENT) {
				gf->errcode = 0;
				if (gf->errmsg) {
					free (gf->errmsg);
					gf->errmsg = NULL;
				}
			}
		}
	}

	return (gf);
}

int
gfal_file_free (gfal_file gf) {
	if (gf == NULL)
		return (0);

	if (gf->lfn != NULL)
		free (gf->lfn);
	if (gf->guid != NULL)
		free (gf->guid);
	if (gf->gobj != NULL)
		gfal_internal_free (gf->gobj);
	if (gf->turl != NULL)
		free (gf->turl);
	if (gf->errmsg != NULL)
		free (gf->errmsg);

	if (gf->replicas) {
		int i;

		for (i = 0; i < gf->nbreplicas; ++i) {
			if (gf->replicas[i] == NULL)
				continue;
			if (gf->replicas[i]->surl != NULL)
				free (gf->replicas[i]->surl);
			if (gf->replicas[i]->errmsg != NULL)
				free (gf->replicas[i]->errmsg);
			free (gf->replicas[i]);
		}

		free (gf->replicas);
	}

	free (gf);
	return (0);
}

const char *
gfal_file_get_catalog_name (gfal_file gf) {
	if (gf == NULL)
		return (NULL);
	return (gfal_file_catalog_names[gf->catalog]);
}

const char *
gfal_file_get_replica (gfal_file gf) {
	if (gf == NULL || gf->errcode != 0 || gf->nbreplicas < 1 || gf->replicas == NULL ||
			gf->current_replica < 0 || gf->current_replica >= gf->nbreplicas)
		return (NULL);
	return (gf->replicas[gf->current_replica]->surl);
}

int
gfal_file_get_replica_errcode (gfal_file gf) {
	if (gf == NULL || gf->errcode != 0 || gf->nbreplicas < 1 || gf->replicas == NULL ||
			gf->current_replica < 0 || gf->current_replica >= gf->nbreplicas)
		return (-1);
	return (gf->replicas[gf->current_replica]->errcode);
}

const char *
gfal_file_get_replica_errmsg (gfal_file gf) {
	if (gf == NULL || gf->errcode != 0 || gf->nbreplicas < 1 || gf->replicas == NULL ||
			gf->current_replica < 0 || gf->current_replica >= gf->nbreplicas)
		return (NULL);
	return (gf->replicas[gf->current_replica]->errmsg);
}

int
gfal_file_set_replica_error (gfal_file gf, int errcode, const char *errmsg) {
	if (gf == NULL || gf->errcode != 0 || gf->nbreplicas < 1 || gf->replicas == NULL ||
			gf->current_replica < 0 || gf->current_replica >= gf->nbreplicas ||
			gf->replicas[gf->current_replica] == NULL)
		return (-1);

	gf->replicas[gf->current_replica]->errcode = errcode > 0 ? errcode : EINVAL;

	if (errmsg && errmsg[0])
		gf->replicas[gf->current_replica]->errmsg = strdup (errmsg);
	++gf->nberrors;

	gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[WARNING] %s: %s", gf->replicas[gf->current_replica]->surl,
			errmsg && errmsg[0] ? errmsg : strerror (gf->replicas[gf->current_replica]->errcode));

	if (gf->nberrors >= gf->nbreplicas) {
		char *file;

		if ((file = gf->lfn) || (file = gf->guid)) {
			gf->errcode = EINVAL;
			asprintf (&(gf->errmsg), "%s: no valid replicas", file);
		} else if (gf->replicas[0]->surl) {
			gf->errcode = errcode > 0 ? errcode : EINVAL;
			asprintf (&(gf->errmsg), "%s: %s", gf->replicas[0]->surl, errmsg);
		} else
			gf->errcode = EINVAL;
	}
	return (0);
}

int
gfal_file_set_turl_error (gfal_file gf, int errcode, const char *errmsg) {
	if (gf == NULL || gf->errcode != 0)
		return (-1);

	if (gf->nbreplicas > 0)
		return (gfal_file_set_replica_error (gf, errcode, errmsg));

	gf->errcode = errcode > 0 ? errcode : EINVAL;
	if (errmsg && errmsg[0])
		asprintf (&(gf->errmsg), "%s: %s", gf->turl, errmsg);
	return (0);
}

int
gfal_file_next_replica (gfal_file gf) {
	int i, bool_ok;

	if (gf == NULL || gf->errcode != 0 || gf->nbreplicas < 1 || gf->replicas == NULL ||
			gf->current_replica < 0 || gf->current_replica >= gf->nbreplicas)
		return (-1);

	if (gf->replicas[gf->current_replica] &&
			gf->replicas[gf->current_replica]->errcode == 0)
		gfal_file_set_replica_error (gf, EINVAL, NULL);

	if (gf->gobj) {
		// next replica needs another gfal_internal object 
		gfal_internal_free (gf->gobj);
		gf->gobj = NULL;
	}

	i = bool_ok = 0;
	do {
		gf->current_replica = (gf->current_replica + 1) % gf->nbreplicas;
		bool_ok = gf->replicas[gf->current_replica]->errcode == 0;
		++i;
	}
	while (!bool_ok && i < gf->nbreplicas);

	if (!bool_ok) {
		// sth weird happened -> corrupted data
		gf->errcode = ENOMEM;
		asprintf (&(gf->errmsg), "[ERROR] Corrupted GFAL data");
		return (-1);
	}

	return (0);
}
