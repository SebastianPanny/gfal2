/*
 * Copyright (C) 2005-2006 by CERN
 */

/*
 * @(#)$RCSfile: sfn_ifce.c,v $ $Revision $ $Date $ CERN Remi Mollon
 */

#include <sys/types.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "gfal_api.h"
#include "gfal.h"



sfn_deletesurls (int nbfiles, const char **surls, struct sfn_filestatus **statuses, char *errbuf, int errbufsz)
{
	int i;
	struct proto_ops *pops = NULL;
	char *pfn;
	char protocol[64];
	char pathbuf[1024];

	if (sfn_turlsfromsurls (nbfiles, surls, NULL, statuses, errbuf, errbufsz) < 0)
		return (-1);

	if (*statuses == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < nbfiles; ++i) {
		if ((*statuses)[i].status != 0 || (*statuses)[i].turl == NULL)
			continue;
		if (parseturl ((*statuses)[i].turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, errbufsz) == 0) {
			if (pops == NULL && (pops = find_pops (protocol)) == NULL) {
				(*statuses)[i].status = EPROTONOSUPPORT;
				continue;
			}
			if (pops->unlink (pfn) < 0)
				(*statuses)[i].status = pops->maperror (pops, 0);
		}
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
	char **protoarray;
	char server[64];
	char *turl;
	char errmsg[ERRMSG_LEN];

	/* check that there is at least one protocol */
	if (protocols == NULL) {
		protoarray = get_sup_proto ();
	} else
		protoarray = protocols;

	if (protoarray == NULL || protoarray[0] == NULL || protoarray[0][0] == '\0') {
		snprintf (errmsg, ERRMSG_LEN - 1, "You must specified at least one protocol");
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EPROTONOSUPPORT;
		return (-1);
	}

	if ((*statuses = (struct sfn_filestatus *) calloc (nbfiles, sizeof (struct sfn_filestatus))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	for (i = 0; i < nbfiles; ++i) {
		memset (*statuses + i, 0, sizeof (struct sfn_filestatus));

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

		for (j = 0; protoarray[j][0] != '\0' && proto == NULL; ++j) {
			for (k = 0; ap[k] && proto == NULL; ++k) {
				if (strcmp (ap[k], protoarray[j]) == 0)
					proto = protoarray[j];
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

		asprintf (&((*statuses)[i].turl), "%s%s", proto, sfns[i] + 3);
		if ((*statuses)[i].turl == NULL)
			(*statuses)[i].status = ENOMEM;
	}

	return (nbfiles);
}
