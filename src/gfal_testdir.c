/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: gfal_testdir.c,v $ $Revision: 1.1.1.1 $ $Date: 2003/11/19 12:56:29 $ CERN Jean-Philippe Baud
 */

#include <stdio.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	struct dirent *d;
	DIR *dir;
	int rc;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}

	if ((dir = gfal_opendir (argv[1])) == NULL) {
		perror ("gfal_opendir");
		exit (1);
	}

#if defined(__USE_FILE_OFFSET64)
	while (d = gfal_readdir64 (dir)) {
#else
	while (d = gfal_readdir (dir)) {
#endif
		printf ("%s\n", d->d_name);
	}

	if (gfal_closedir (dir) < 0) {
		perror ("gfal_closedir");
		exit (1);
	}
	exit (0);
}
