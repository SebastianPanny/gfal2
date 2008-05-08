/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: sfn_ifce.c,v $ $Revision: 1.7 $ $Date: 2008/05/08 13:16:36 $ CERN Remi Mollon
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "gfal_api.h"
#include "gfal.h"



sfn_deletesurls (int nbfiles, const char **surls, struct sfn_filestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int i;
	char *protocols[] = {"gsiftp", ""};

	if (sfn_turlsfromsurls (nbfiles, surls, protocols, statuses, errbuf, errbufsz) < 0)
		return (-1);

	if (*statuses == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < nbfiles; ++i) {
		if ((*statuses)[i].turl == NULL && (*statuses)[i].status == 0)
			(*statuses)[i].status = EFAULT;
		if ((*statuses)[i].status != 0)
			continue;

		if (gridftp_delete ((*statuses)[i].turl, errbuf, errbufsz, timeout) < 0)
			(*statuses)[i].status = errno;
	}

	return (nbfiles);
}

sfn_getfilemd (int nbfiles, const char **surls, struct srmv2_mdfilestatus **statuses, char *errbuf, int errbufsz, int timeout)
{
	int i, j;
	struct sfn_filestatus *turlstatuses;
	char *protocols[] = {"gsiftp", ""};
	int nbresults;
	char **filenames;
	struct stat64 *statbufs;

	*statuses = NULL;

	if (sfn_turlsfromsurls (nbfiles, surls, protocols, &turlstatuses, errbuf, errbufsz) < 0)
		return (-1);

	if (turlstatuses == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	if ((*statuses = (struct srmv2_mdfilestatus *) calloc (nbfiles, sizeof (struct srmv2_mdfilestatus))) == NULL)
		return (-1);

	for (i = 0; i < nbfiles; ++i) {
		if (turlstatuses[i].turl == NULL && turlstatuses[i].status == 0)
			turlstatuses[i].status = EFAULT;
		if (turlstatuses[i].status != 0)
			continue;

		(*statuses)[i].surl = turlstatuses[i].surl;
		filenames = NULL;
		statbufs = NULL;

		if (gridftp_ls (turlstatuses[i].turl, &nbresults, &filenames, &statbufs, errbuf, errbufsz, timeout) < 0 ||
				nbresults < 1 || filenames == NULL || statbufs == NULL) {
			if (filenames) free (filenames);
			if (statbufs) free (statbufs);
			(*statuses)[i].status = errno;
			continue;
		}

		if (nbresults > 1) {
			(*statuses)[i].nbsubpaths = nbresults;
			if (((*statuses)[i].subpaths = (struct srmv2_mdfilestatus *) calloc (nbresults, sizeof (struct srmv2_mdfilestatus))) == NULL) {
				free (filenames);
				free (statbufs);
				return (-1);
			}

			for (j = 0; j < nbresults; ++j) {
				(*statuses)[i].subpaths[j].surl = filenames[j];
				(*statuses)[i].subpaths[j].stat = statbufs[j];
			}
		} else {
			if (filenames[0]) free (filenames[0]);
			(*statuses)[i].stat = statbufs[0];
		}

		free (filenames);
		free (statbufs);
	}

	return (nbfiles);
}

sfn_turlsfromsurls (int nbfiles, const char **sfns, char **protocols, struct sfn_filestatus **statuses, char *errbuf, int errbufsz)
{
	char **ap;
	int i,j, k;
	int len;
	char *p;
	int *pn;
	char *proto = NULL;
	char server[64];
   
	if (!protocols)
		protocols = get_sup_proto ();

	if ((*statuses = (struct sfn_filestatus *) calloc (nbfiles, sizeof (struct sfn_filestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < nbfiles; ++i) {
		if (sfns[i] == NULL) {
			(*statuses)[i].status = EINVAL;
			continue;
		}

		(*statuses)[i].surl = strdup (sfns[i]);

		if ((p = strchr (sfns[i] + 6, '/')) == NULL ||
				(len = p - (sfns[i] + 6)) > sizeof(server)) {
			(*statuses)[i].status = ENAMETOOLONG;
			continue;
		}

		/* check that the SE supports at least one protocol */
		strncpy (server, sfns[i] + 6, len);
		*(server + len) = '\0';
		if (get_seap_info (server, &ap, &pn, errbuf, errbufsz) < 0) {
			(*statuses)[i].status = errno;
			errno = 0;
			continue;
		}

		for (j = 0; protocols[j][0] != '\0' && proto == NULL; ++j) {
			for (k = 0; ap[k] && proto == NULL; ++k) {
				if (strcmp (ap[k], protocols[j]) == 0)
					proto = protocols[j];
			}
		}
		for (k = 0; ap[k]; ++k)
			free (ap[k]);
		free (ap);
		free (pn);
		if (!proto) {
			(*statuses)[i].status = EPROTONOSUPPORT;
			continue;
		}

		/* Replace 'sfn' by the protocol, and add an extra '/' after the host name */
		asprintf (&((*statuses)[i].turl), "%s://%s/%s", proto, server, p);
		if ((*statuses)[i].turl == NULL)
			(*statuses)[i].status = ENOMEM;
	}

	return (nbfiles);
}
