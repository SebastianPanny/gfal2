/**
 * Compile command : gcc gfal_testget.c `pkg-config --libs --cflags libgfal2`
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/param.h>

#define DEFPOLLINT 10
#define TURL_MAX_SIZE 1024


#ifndef _GFAL_1_X

#include <gfal_api.h>

 // NORMAL NEW WAY TO USE IN GFAL 2.0

int main(int argc,char **argv)
{
	int errsz = 500;
	char err[errsz];

	char turl_buff[TURL_MAX_SIZE];

	if (argc < 2){
		fprintf (stderr, "usage: %s SURLs\n", argv[0]);
		exit (1);
	}
	gfal_set_verbose (GFAL_VERBOSE_VERBOSE);
	
	ssize_t res = gfal_getxattr(argv[1], "srm.turl", turl_buff,TURL_MAX_SIZE );
	
	if (res > 0)
		printf("SURL %s Ready - TURL: %s\n", argv[1], turl_buff);
	else {
		printf("SURL %s Failed:\n%s\n", argv[1]);
		gfal_posix_check_error();
	}
		
	return((res>0)?0:-1);
}

#endif


/*  This is the OLD WAY GFAL 1.0 to do IT, DONT USE IT WITH GFAL2.0, IT IS FOR A  EXAMPLE OF MIGRATION 1.X to 2.0
#ifdef _GFAL_1_X 
#define gfal_handle_free(x) gfal_internal_free(x)


#include <gfal_api.h>

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
#endif
*/
