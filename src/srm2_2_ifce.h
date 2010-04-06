/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Inspired by MinUnit -- a minimal unit testing framework for C
 * (http://www.jera.com/techinfo/jtns/jtn002.html)
 */
#ifndef SRM2_2_IFCE_H
#define SRM2_2_IFCE_H

#include "gfal_types.h"

#define GFAL_SRM_RETURN_ERROR -1
#define GFAL_SRM_RETURN_OK 0

int srmv2_deletesurls (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_rmdir (const char *, const char *, int, struct srmv2_filestatus **, char *, int, int);

/*
 * @param protocols The list of supported protocols. This is a NULL-terminated
 *        list.
 */
int srmv2_get (int, const char **, const char *, char ** protocols, char **,
    struct srmv2_filestatus **, char *, int, int);

/* CODEREVIEW START */
/*
 * @param protocols The list of supported protocols. This is a NULL-terminated
 *        list.
 */
int srmv2_gete(
   int nbfiles, const char **surls, const char *srm_endpoint, int desiredpintime,
   const char *spacetokendesc, char **protocols, char **reqtoken,
   struct srmv2_pinfilestatus **filestatuses, char *errbuf, int errbufsz,
   int timeout);
/* CODEREVIEW END */

int srmv2_getstatus (int, const char **, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_getstatuse (const char *, const char *, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_getspacetokens (const char *, const char *, int *, char ***, char *, int, int);
int srmv2_getspacemd (int, const char **, const char *, gfal_spacemd **, char *, int, int);
char *srmv2_getbestspacetoken (const char *, const char *, GFAL_LONG64, char *, int, int);
int srmv2_makedirp (const char *, const char *, char *, int, int);

/*
 * @param protocols The list of supported protocols. This is a NULL-terminated
 *        list.
 */
int srmv2_bringonline(
    int nbfiles, const char **surls, const char *srm_endpoint, int desiredpintime,
    const char *spacetokendesc, char **protocols, char **reqtoken,
    struct srmv2_pinfilestatus **filestatuses, char *errbuf, int errbufsz,
    int timeout);

int srmv2_prestage (int, const char **, const char *, int, char **, int, char **, struct srmv2_pinfilestatus **, char *, int, int);

/*
 * @param protocols The list of supported protocols. This is a NULL-terminated
 *        list.
 */
int srmv2_prestagee (
    int nbfiles, const char **surls, const char *srm_endpoint, int desiredpintime,
    const char *spacetokendesc, char **protocols, char **reqtoken,
    struct srmv2_pinfilestatus **filestatuses, char *errbuf, int errbufsz,
    int timeout);

int srmv2_prestagestatus (int, const char **, const char *, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_prestagestatuse (const char *, const char *, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_set_xfer_done_get (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_set_xfer_done_put (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_set_xfer_running (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);

/*
 * @param protocols The list of supported protocols. This is a NULL-terminated
 *        list.
 */
int srmv2_turlsfromsurls_get (
    int nbfiles, const char **surls, const char *srm_endpoint, int desiredpintime,
    const char *spacetokendesc, char **protocols, char **reqtoken,
    struct srmv2_pinfilestatus **filestatuses, char *errbuf, int errbufsz,
    int timeout);

/*
 * @param protocols The list of supported protocols. This is a NULL-terminated
 *        list.
 */
int srmv2_turlsfromsurls_put(
    int nbfiles, const char **surls, const char *srm_endpoint,
    GFAL_LONG64 *filesizes, int desiredpintime, const char *spacetokendesc,
    char **protocols, char **reqtoken, struct srmv2_pinfilestatus **filestatuses,
    char *errbuf, int errbufsz, int timeout);

int srmv2_pin (int, const char **, const char *, const char *, int, struct srmv2_pinfilestatus **, char *, int, int);
int srmv2_release (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_abortrequest (const char *, const char *, char *, int, int);
int srmv2_abortfiles (int, const char **, const char *, const char *, struct srmv2_filestatus **, char *, int, int);
int srmv2_access (int, const char **, const char *, int, struct srmv2_filestatus **, char *, int, int);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
int srmv2_getfilemd (int, const char **, const char *, int, int *, int, struct srmv2_mdfilestatus **, char **, char *, int, int);
#endif

#endif /* #ifndef SRM2_2_IFCE_H */
