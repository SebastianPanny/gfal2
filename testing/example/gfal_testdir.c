/**
 * Compile command : gcc gfal_testdir.c `pkg-config --libs --cflags libgfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"

main(int argc, char **argv)
{
	struct dirent *d;
	DIR *dir;

	if (argc != 2) {
		fprintf (stderr, "usage: %s filename\n", argv[0]);
		exit (1);
	}

	if ((dir = gfal_opendir (argv[1])) == NULL) {
		perror ("gfal_opendir");
		exit (1);
	}

#if defined(__USE_FILE_OFFSET64)
	while ((d = gfal_readdir64 (dir))) {
#else
	while ((d = gfal_readdir (dir))) {
#endif
		printf ("%s\n", d->d_name);
	}

	if (gfal_closedir (dir) < 0) {
		perror ("gfal_closedir");
		exit (1);
	}
	exit (0);
}
