/*
 * Copyright (C) 2005 by CERN
 */

/*
 * @(#)$RCSfile: gfal_testget.c,v $ $Revision: 1.6 $ $Date: 2009/02/09 15:27:42 $ CERN Jean-Philippe Baud
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "gfal_api.h"
#define DEFPOLLINT 10

main(argc, argv)
int argc;
char **argv;
{
	gfal_request req = NULL;
	gfal_internal gobj = NULL;
	gfal_filestatus *filestatuses = NULL;
	int sav_errno = 0, n = 0, i = 0, nberrors = 0;
	static char *protos[] = {"rfio", "dcap", "gsiftp"};

	if (argc < 2) {
		fprintf (stderr, "usage: %s SURLs\n", argv[0]);
		exit (1);
	}

	gfal_set_verbose (0);

	if ((req = gfal_request_new ()) == NULL)
		exit (1);
	req->nbfiles = argc - 1;
	req->surls = argv + 1;
	req->protocols = protos;

	if (gfal_init (req, &gobj, NULL, 0) < 0) {
		sav_errno = errno;
		free (req);
		errno = sav_errno;
		perror (argv[0]);
		exit (1);
	}
	free (req);

	if (gfal_turlsfromsurls (gobj, NULL, 0) < 0) {
		sav_errno = errno;
		gfal_internal_free (gobj);
		errno = sav_errno;
		perror (argv[0]);
		exit (1);
	}

	if ((n = gfal_get_results (gobj, &filestatuses)) < 0) {
		sav_errno = errno;
		gfal_internal_free (gobj);
		errno = sav_errno;
		perror (argv[0]);
		exit (1);
	}

	if (filestatuses == NULL) {
		fprintf (stderr, "%s: Internal error (memory corruption?)\n", argv[0]);
		exit (1);
	}

	for (i = 0; i < n; ++i) {
		if (filestatuses[i].status == 0)
			printf("SURL %s Ready - TURL: %s\n", filestatuses[i].surl, filestatuses[i].turl);
		else {
			++nberrors;
			if (filestatuses[i].explanation)
				printf("SURL %s Failed:\n%s\n", filestatuses[i].surl, filestatuses[i].explanation);
			else
				printf("SURL %s Failed:\n%s\n", filestatuses[i].surl, strerror (filestatuses[i].status));
		}
	}

	gfal_internal_free (gobj);
	exit (nberrors > 0 ? 1 : 0);
}
