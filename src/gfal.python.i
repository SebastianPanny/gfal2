%module gfal_python

%{
#include "gfal_api.h"
%}

%include "lcg-typemaps.python.i"

extern int deletesurl (const char *surl, char *errbuf, int errbufsz, int timeout);
extern int deletesurl2 (const char *surl, char *spacetokendesc, char *errbuf,
			int errbufsz, int timeout);

extern int getfilemd (const char *surl, struct stat64 *statbuf, char *errbuf,
			int errbufsz, int timeout);

extern int set_xfer_done (const char *surl, int reqid, int fileid, char *token,
			int oflag, char *errbuf, int errbufsz, int timeout);
extern int set_xfer_running (const char *surl, int reqid, int fileid, char *token,
			char *errbuf, int errbufsz, int timeout);

extern char *turlfromsurl (const char *surl, char **protocols, int oflag, int *OUTPUT,
			int *OUTPUT, char **OUTPUT, char *errbuf, int errbufsz,
			int timeout);
extern char *turlfromsurlx (const char *surl, GFAL_LONG64 filesize, char **protocols,
			int oflag, int *OUTPUT, int *OUTPUT, char **OUTPUT, char *errbuf,
			int errbufsz, int timeout);
extern char *turlfromsurl2 (const char *surl, GFAL_LONG64 filesize,
			const char *spacetokendesc, char **protocols, int oflag,
			int *OUTPUT, int *OUTPUT, char **OUTPUT, char *errbuf, int errbufsz,
			int timeout);

extern int srm_get (int LEN, char **LIST, int LEN, char **LIST, int *OUTPUT, char **OUTPUT,
			struct srm_filestatus **filestatuses, int timeout);
extern int srm_getx (int LEN, char **LIST, int LEN, char **LIST, int *OUTPUT, char **OUTPUT,
			struct srm_filestatus **filestatuses, char *errbuf, int errbufsz,
			int timeout);
extern int srm_getstatus (int LEN, char **LIST, int reqid, char *token,
			struct srm_filestatus **filestatuses, int timeout);
extern int srm_getstatusx (int LEN, char **LIST, int reqid, char *token,
			struct srm_filestatus **filestatuses, char *errbuf, int errbufsz,
			int timeout);

extern int srmv2_prestage (int LEN, char **LIST, char *spacetokendesc, int LEN,	char **LIST,
			char **OUTPUT, struct srmv2_filestatus **filestatuses,
			char *errbuf, int errbufsz, int timeout);
extern int srmv2_prestagestatus (int LEN, char **LIST, char *reqtoken,
			struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz,
			int timeout);
