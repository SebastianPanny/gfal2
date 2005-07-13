/*
 * Copyright (C) 2005 by CERN
 */

/*
 * @(#)$RCSfile: gfal_testget.c,v $ $Revision: 1.2 $ $Date: 2005/07/13 11:22:10 $ CERN Jean-Philippe Baud
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "gfal_api.h"
#define DEFPOLLINT 10

main(argc, argv)
int argc;
char **argv;
{
	struct srm_filestatus *filestatuses;
	int i;
	int nbfiles;
	int nbprotocols;
	int nbreplies;
	static char *protocols[] = {"rfio"};
	int r = 0;
	int reqid;
	char **surls;

	if (argc < 2) {
		fprintf (stderr, "usage: %s SURLs\n", argv[0]);
		exit (1);
	}
	nbfiles = argc - 1;
	surls = &argv[1];

	nbprotocols = sizeof(protocols) / sizeof(char *);

	if ((nbreplies = srm_get (nbfiles, surls, nbprotocols, protocols,
	    &reqid, NULL, &filestatuses, 0)) < 0) {
		perror ("srm_get");
		exit (1);
	}

	/* process files as soon as they are "ready" */

	while (1) {
		for (i = 0; i < nbreplies; i++) {
			if ((filestatuses+i)->status == 1) {	/* file ready */
				/* process it if not yet done */
			}
			if ((filestatuses+i)->surl)
				free ((filestatuses+i)->surl);
			if ((filestatuses+i)->turl)
				free ((filestatuses+i)->turl);
		}
		free (filestatuses);

		/* if processing complete, break */ if (r) break;

		sleep ((r++ == 0) ? 1 : DEFPOLLINT);
		if ((nbreplies = srm_getstatus (nbfiles, surls, reqid, NULL,
		    &filestatuses, 0)) < 0) {
			perror ("srm_getstatus");
			exit (1);
		}
	}
	exit (0);
}
