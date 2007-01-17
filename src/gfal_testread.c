/*
 * Copyright (C) 2003 by CERN
 */

/*
 * @(#)$RCSfile: gfal_testread.c,v $ $Revision: 1.2 $ $Date: 2007/01/17 13:56:01 $ CERN Jean-Philippe Baud
 */

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "gfal_api.h"
#define BLKLEN 65536

main(int argc, char **argv)
{
	int fd;
	int i;
	char ibuf[BLKLEN];
	int rc;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}

	printf ("opening %s\n", argv[1]);
	if ((fd = gfal_open (argv[1], O_RDONLY, 0)) < 0) {
		perror ("gfal_open");
		exit (1);
	}
	printf ("open successful, fd = %d (errno = %d)\n", fd, errno);

	if ((rc = gfal_read (fd, ibuf, BLKLEN)) != BLKLEN) {
		if (rc < 0)
			perror ("gfal_read");
		else
			fprintf (stderr, "gfal_read returns %d (errno = %d)\n", rc, errno);
		(void) gfal_close (fd);
		exit (1);
	}
	printf ("read successful (errno = %d)\n", errno);

	if ((rc = gfal_close (fd)) < 0) {
		perror ("gfal_close");
		exit (1);
	}
	printf ("close successful\n");
	exit (0);
}
