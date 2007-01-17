/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: gfal_testchmod.c,v $ $Revision: 1.1 $ $Date: 2007/01/17 13:56:01 $ CERN Remi Mollon
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	int rc, mode, error;
	char *file;

	if (argc != 3) {
		fprintf (stderr, "usage: %s file mode\n", argv[0]);
		exit (1);
	}

	file = argv[1];
	mode = strtol (argv[2], NULL, 8);
	if (errno > 0) {
		perror ("strtol");
		exit (1);
	}

	printf ("Checking RW access to '%s'...\n",file);
	if (gfal_access (file, R_OK|W_OK) < 0) {
		error = 1;
		perror ("gfal_access");
	}

	printf ("Changing mode of '%s' to %o...\n", file, mode);
	if (gfal_chmod (file, mode) < 0) {
		error = 1;
		perror ("gfal_chmod");
	}

	if (error) exit (1);

	printf ("All is ok.\n");
	exit (0);
}
