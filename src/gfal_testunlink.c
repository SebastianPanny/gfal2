/*
 * Copyright (C) 2004 by CERN
 */

/*
 * @(#)$RCSfile: gfal_testunlink.c,v $ $Release$ $Date: 2004/04/19 08:53:44 $ CERN Jean-Philippe Baud
 */

#include <fcntl.h>
#include <stdio.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	struct stat statbuf;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}
	if (gfal_unlink (argv[1]) < 0) {
		perror ("gfal_unlink");
		exit (1);
	}
	exit (0);
}
