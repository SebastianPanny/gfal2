/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: gfal_testcreatdir.c,v $ $Revision: 1.1 $ $Date: 2007/01/17 13:56:01 $ CERN Remi Mollon
 */

#include <stdio.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	int rc;
	char *rootdir;
	char olddir[1024], newdir[1024];

	if (argc != 2) {
		fprintf (stderr, "usage: %s rootdir\n", argv[0]);
		exit (1);
	}

	rootdir = argv[1];
	snprintf (olddir, 1024, "%s/olddir", rootdir);
	snprintf (newdir, 1024, "%s/newdir", rootdir);

	printf ("Creating directory 'olddir'...\n");
	if (gfal_mkdir (olddir, 0700) < 0) {
		perror ("gfal_mkdir");
		exit (1);
	}

	printf ("Renaming directory 'olddir' to 'newdir'...\n");
	if (gfal_rename (olddir, newdir) < 0) {
		perror ("gfal_rename");
		exit (1);
	}

	printf ("Removing directory 'newdir'...\n");
	if (gfal_rmdir (newdir) < 0) {
		perror ("gfal_rmdir");
		exit (1);
	}

	printf ("All is ok.\n");
	exit (0);
}
