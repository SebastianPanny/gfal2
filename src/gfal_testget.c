/*
 * Copyright (C) 2005 by CERN
 */

/*
 * @(#)$RCSfile: gfal_testget.c,v $ $Revision: 1.3 $ $Date: 2006/04/11 12:44:25 $ CERN Jean-Philippe Baud
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
	static char *protocols[] = {"rfio", "dcap", "gsiftp"};
	int r = 0;
	int reqid;
	char **surls;
	int num_done = 0;

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

	while (1) {
		for (i = 0; i < nbreplies; i++) {
			if ((filestatuses+i)->status == 1) {	
				/* file ready */
				num_done++;
				printf("SURL %s Ready - TURL: %s\n", (filestatuses+i)->surl, 
								     (filestatuses+i)->turl);
			} else if ((filestatuses+i)->status == 0) {
				printf("SURL %s Pending\n", (filestatuses+i)->surl);
			} else {
				printf("SURL %s Failed\n", (filestatuses+i)->surl);
			}
		}

		/* if processing complete, break */ if (num_done == nbreplies) break;

		sleep ((r++ == 0) ? 1 : DEFPOLLINT);
		if ((nbreplies = srm_getstatus (nbfiles, surls, reqid, NULL,
						&filestatuses, 0)) < 0) {
			perror ("srm_getstatus");
			exit (1);
		}
	}
	exit (0);
}
