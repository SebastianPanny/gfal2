/**
 * Compile command : gcc gfal_teststat.c `pkg-config --libs --cflags libgfal2`
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	struct stat statbuf;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}
	if (gfal_stat (argv[1], &statbuf) < 0) {
		perror ("gfal_stat");
		exit (1);
	}
	printf ("stat successful\n");
	printf ("mode = %o\n", statbuf.st_mode);
	printf ("nlink = %d\n", statbuf.st_nlink);
	printf ("uid = %d\n", statbuf.st_uid);
	printf ("gid = %d\n", statbuf.st_gid);
	printf ("size = %ld\n", statbuf.st_size);
	exit (0);
}
