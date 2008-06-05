%module gfal

%{
#include "gfal_api.h"
%}

%include "lcg-typemaps.python.i"

extern const char *gfal_version ();

extern int gfal_init (gfal_request req, gfal_internal *gfal, char *errbuf, int errbufsz);
extern int gfal_get_results (gfal_internal req, gfal_filestatus **statuses);
extern int gfal_get_ids (gfal_internal req, int *OUTPUT, int **OUTPUT, char **OUTPUT);
extern int gfal_set_ids (gfal_internal req, int LEN, const int *LIST, int srm_reqid,
        const char *srmv2_reqtoken, char *errbuf, int errbufsz);
extern void gfal_internal_free (gfal_internal req);
extern int gfal_deletesurls (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_removedir (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_turlsfromsurls (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_ls (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_get (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_getstatus (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_prestage (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_prestagestatus (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_pin (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_release (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_set_xfer_done (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_set_xfer_running (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_abortrequest (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_abortfiles (gfal_internal req, char *errbuf, int errbufsz);
