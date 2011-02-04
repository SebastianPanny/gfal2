/*
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
 */

/*
 * @(#)$RCSfile: gfal.c,v $ $Revision: 1.141 $ $Date: 2009/12/09 13:18:13 $ CERN Jean-Philippe Baud
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <time.h>
#include <uuid/uuid.h>
#include "voms_apic.h"
#include "gfal_api.h"
#include "gfal_internals.h"
#include "lfc_ifce.h"
#include <gfal_srm_ifce.h>
#include <gfal_srm_ifce_types.h>

#if !defined(OFF_MAX)
#define OFF_MAX 2147483647
#endif





static struct dir_info *di_array[GFAL_OPEN_MAX];
static struct xfer_info *xi_array[GFAL_OPEN_MAX];
static char *gfal_userdn = NULL;
static char *gfal_vo = NULL;
static char *gfal_fqan[GFAL_FQAN_MAX];
static int gfal_nb_fqan = 0;
static int vomsdataparsed = 0;
static int nobdii = 0;
static int purifydisabled = -1;

/*
 * Verbose level
 *   API mode (no messages on stderr) by default
 *   CLI has to set it to '0' to get normal error messages
 */
static int gfal_verbose = -1;

static enum status_type {DEFAULT_STATUS = 0, MD_STATUS, PIN_STATUS};

static struct dir_info *alloc_di (DIR *);
static struct xfer_info *alloc_xi (int);
static struct dir_info *find_di (DIR *);
static struct xfer_info *find_xi (int);
static void free_di (struct dir_info *);
static int free_xi (int);
static int mdtomd32 (struct stat64 *, struct stat *);
static int copy_gfal_results (gfal_internal, enum status_type);
static int check_gfal_internal (gfal_internal, int, char *, int);
static void free_gfal_results (gfal_filestatus *, int);
static void free_srmv2_mdstatuses (struct srmv2_mdfilestatus *, int);

/* the version should be set by a "define" at the makefile level */
static const char gfalversion[] = VERSION;

    int
gfal_parse_vomsdata (char *errbuf, int errbufsz)
{
    if (!vomsdataparsed) {
        struct vomsdata *vd;
        int i, len, error;
        char *pos, *p1, *p2;
        char errmsg[GFAL_ERRMSG_LEN];

        if ((vd = VOMS_Init ("", "")) == NULL ||
                !VOMS_SetVerificationType (VERIFY_NONE, vd, &error) ||
                !VOMS_RetrieveFromProxy (RECURSE_CHAIN, vd, &error)) {
            if (error != VERR_NOEXT) { /* error is not "VOMS extension not found!" */
                VOMS_ErrorMessage (vd, error, errmsg, GFAL_ERRMSG_LEN);
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][VOMS_RetrieveFromProxy][] %s", errmsg);
            }
            VOMS_Destroy (vd);
            return (-1);
        }
        else if (!vd->data || !vd->data[0]) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_parse_vomsdata][] Unable to get VOMS info from the proxy (Memory problem?)");
            return (-1);
        }

        gfal_userdn = strdup (vd->data[0]->user);
        gfal_vo = gfal_vo == NULL ? strdup (vd->data[0]->voname) : gfal_vo;

        for (i = 0; vd->data[0]->fqan[i] != NULL; ++i) {
            if ((gfal_fqan[i] = strdup (vd->data[0]->fqan[i])) == NULL)
                return (-1);

            pos = p1 = gfal_fqan[i];
            if (*p1 != '/') {
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_parse_vomsdata][] Invalid FQAN: %s", gfal_fqan[i]);
                return (-1);
            }

            /* 'Role=NULL' and 'Capability=NULL' are removed from the fqan */
            while (p1 != NULL) {
                p2 = strchr (p1 + 1, '/');
                if (p2) *p2 = '\0';

                if (!strstr (p1, "=NULL")) {
                    if (pos == p1)
                        pos = p2;
                    else {
                        len = strlen (p1);
                        memmove (pos, p1, len + 1);
                        pos += len;
                    }
                }

                p1 = p2;
                if (p1) *p1 = '/';
            }

            if (pos) *pos = '\0';
        }

        gfal_fqan[i] = NULL;
        gfal_nb_fqan = i;
        vomsdataparsed = 1;
        VOMS_Destroy (vd);
    }

    return (0);
}

    char *
gfal_get_userdn (char *errbuf, int errbufsz)
{
    if (gfal_userdn == NULL)
        gfal_parse_vomsdata (errbuf, errbufsz);

    if (gfal_userdn == NULL) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_get_userdn][] Unable to get the user's DN from the proxy");
        errno = EINVAL;
    }

    return (gfal_userdn);
}

    int
gfal_set_vo (const char *vo)
{
    if (gfal_vo == NULL) {
        char errmsg[GFAL_ERRMSG_LEN];

        gfal_parse_vomsdata (errmsg, GFAL_ERRMSG_LEN);
        if (gfal_vo != NULL && strcmp (gfal_vo, vo) != 0)
            gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_ERROR,
                    "[WARNING] specified VO and proxy VO are different!");
    }

    if ((gfal_vo = strdup (vo)) == NULL)
        return (-1);

    return (0);
}

    char *
gfal_get_vo (char *errbuf, int errbufsz)
{
    if (gfal_vo == NULL && (gfal_vo = getenv ("LCG_GFAL_VO")) == NULL)
        gfal_parse_vomsdata (errbuf, errbufsz);

    if (gfal_vo == NULL) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_get_vo][EINVAL] Unable to get the VO name neither from environment (LCG_GFAL_VO) nor from the proxy");
        errno = EINVAL;
    }

    return (gfal_vo);
}

    int
gfal_get_fqan (char ***fqan, char *errbuf, int errbufsz)
{
    if (fqan == NULL) return (-1);

    gfal_parse_vomsdata (errbuf, errbufsz);
    *fqan = gfal_fqan;
   
    
    return (gfal_nb_fqan);
}

    int
gfal_is_purifydisabled ()
{
    if (purifydisabled < 0)
        purifydisabled = getenv ("LCG_GFAL_FULL_SURLS_IN_FC") == NULL ? 0 : 1;

    return (purifydisabled);
}

    static struct dir_info *
alloc_di (DIR *dir)
{
    int i;

    for (i = 0; i < GFAL_OPEN_MAX; i++) {
        if (di_array[i] == NULL) {
            if ((di_array[i] = (struct dir_info *) calloc (1, sizeof(struct dir_info))) == NULL)
                return (NULL);
            di_array[i]->dir = dir;
            return (di_array[i]);
        }
    }
    errno = EMFILE;
    return (NULL);
}

    static struct xfer_info *
alloc_xi (int fd)
{
    if (fd >= 0 && fd < GFAL_OPEN_MAX && xi_array[fd] == NULL)
        return (xi_array[fd] = (struct xfer_info *) calloc (1, sizeof(struct xfer_info)));
    errno = EBADF;
    return (NULL);
}

    static struct dir_info *
find_di (DIR *dir)
{
    int i;

    for (i = 0; i < GFAL_OPEN_MAX; i++) {
        if (di_array[i] && di_array[i]->dir == dir)
            return (di_array[i]);
    }
    errno = EBADF;
    return (NULL);
}

    static struct xfer_info *
find_xi (int fd)
{
    if (fd >= 0 && fd < GFAL_OPEN_MAX && xi_array[fd])
        return (xi_array[fd]);
    errno = EBADF;
    return (NULL);
}

    static void
free_di (struct dir_info *di)
{
    free (di);
    di = NULL;
}

    static int
free_xi (int fd)
{
    if (fd >= 0 && fd < GFAL_OPEN_MAX && xi_array[fd]) {
        if (xi_array[fd]->gfile)
            gfal_file_free (xi_array[fd]->gfile);
        free (xi_array[fd]);
        xi_array[fd] = NULL;
    }
    return (0);
}

    int
gfal_access (const char *path, int amode)
{
    int rc = 0, sav_errno = 0;
    int bool_issurlok = 0;
    char errbuf[GFAL_ERRMSG_LEN];
    char pfn[1104];
    struct proto_ops *pops = NULL;
    char protocol[64];
    char **supported_protocols;
    gfal_file gfile;
    gfal_request req;
    gfal_filestatus *filestatuses;
    const char *current_surl;

    if ((gfile = gfal_file_new (path, "file", 0, errbuf, GFAL_ERRMSG_LEN)) == NULL)
        return (-1);
    if (gfile->errcode != 0) {
        sav_errno = gfile->errcode;
        gfal_file_free (gfile);
        errno = sav_errno;
        return (-1);
    }

    if (gfile->catalog == GFAL_FILE_CATALOG_LFC && gfile->guid != NULL) {
        rc = lfc_accessl (gfile->lfn, gfile->guid, amode, NULL, 0);
        sav_errno = errno;
        gfal_file_free (gfile);
        errno = sav_errno;
        return (rc);
    }

    if (gfile->nbreplicas <= 0 && gfile->turl == NULL) {
        errno = ENOENT;
        return (-1);
    }

    if ((req = gfal_request_new ()) == NULL) {
        sav_errno = errno;
        gfal_file_free (gfile);
        errno = sav_errno;
        return (-1);
    }

    supported_protocols = get_sup_proto ();

    req->nbfiles = 1;
    req->defaultsetype = TYPE_SRMv2;
    req->no_bdii_check = gfal_is_nobdii ();
    req->protocols = supported_protocols;

    while (!bool_issurlok && gfile->errcode == 0) {
        bool_issurlok = 1;

        if (gfile->nbreplicas > 0) {
            current_surl = gfal_file_get_replica (gfile);
            req->surls = (char **) &current_surl;

            if (!(bool_issurlok = gfal_init (req, &gfile->gobj, errbuf, GFAL_ERRMSG_LEN) >= 0))
                gfal_file_set_replica_error (gfile, errno, errbuf);

            if (bool_issurlok)
            {
                if (gfile->gobj->setype == TYPE_SRMv2)
                {

                    struct srm_context context;
                    struct srm_checkpermission_input checkpermission_input;


                    srm_context_init(&context,gfile->gobj->endpoint,errbuf,GFAL_ERRMSG_LEN,gfal_verbose);

                    checkpermission_input.nbfiles = gfile->gobj->nbfiles;
                    checkpermission_input.amode = amode;
                    checkpermission_input.surls = gfile->gobj->surls;

                	gfile->gobj->returncode = srm_check_permission(&context,&checkpermission_input, &(gfile->gobj->srmv2_statuses));

/*TODO                    gfile->gobj->returncode = srmv2_access (gfile->gobj->nbfiles,
                            (const char **) gfile->gobj->surls, gfile->gobj->endpoint, amode,
                            &(gfile->gobj->srmv2_statuses), errbuf, GFAL_ERRMSG_LEN,
                            gfile->gobj->timeout);
*/
                    if (!(bool_issurlok = gfile->gobj->returncode >= 0))
                        gfal_file_set_replica_error (gfile, errno, errbuf);

                    if (bool_issurlok)
                        bool_issurlok = copy_gfal_results (gfile->gobj, DEFAULT_STATUS) >= 0;

                    if (bool_issurlok && (!(bool_issurlok = gfal_get_results (gfile->gobj, &filestatuses) >= 0) ||
                                !(bool_issurlok = filestatuses != NULL))) {
                        snprintf (errbuf, GFAL_ERRMSG_LEN, "Internal error");
                        gfal_file_set_replica_error (gfile, errno, errbuf);
                    }

                    if (bool_issurlok && !(bool_issurlok = filestatuses[0].status == 0))
                        gfal_file_set_replica_error (gfile, filestatuses[0].status, filestatuses[0].explanation);

                    sav_errno = gfile->errcode;
                    rc = gfile->errcode == 0 ? 0 : -1;
                    gfal_file_free (gfile);
                    errno = sav_errno;
                    return (rc);
                } else {
                    if (!(bool_issurlok = gfal_turlsfromsurls (gfile->gobj, errbuf, GFAL_ERRMSG_LEN) >= 0))
                        gfal_file_set_replica_error (gfile, errno, errbuf);

                    if (bool_issurlok && (!(bool_issurlok = gfal_get_results (gfile->gobj, &filestatuses) >= 0) ||
                                !(bool_issurlok = filestatuses != NULL))) {
                        snprintf (errbuf, GFAL_ERRMSG_LEN, "Internal error");
                        gfal_file_set_replica_error (gfile, errno, errbuf);
                    }

                    if (bool_issurlok && !(bool_issurlok = filestatuses[0].status == 0))
                        gfal_file_set_replica_error (gfile, filestatuses[0].status, filestatuses[0].explanation);

                    if (bool_issurlok) {
                        gfile->turl = strdup (filestatuses[0].turl);
                        if (gfile->turl == NULL) {
                            sav_errno = errno;
                            free (req);
                            gfal_file_free (gfile);
                            errno = sav_errno;
                            return (-1);
                        }
                    }
                }
            }
        }

        if (bool_issurlok && !(bool_issurlok = parseturl (gfile->turl, protocol, sizeof(protocol),
                        pfn, sizeof(pfn), errbuf, GFAL_ERRMSG_LEN) >= 0))
            gfal_file_set_turl_error (gfile, errno, errbuf);

        if (bool_issurlok) {
            pops = find_pops (protocol);
            if (!(bool_issurlok = pops != NULL))
                gfal_file_set_turl_error (gfile, EPROTONOSUPPORT, NULL);
        }

        if (bool_issurlok) {
            if ((rc = pops->access (pfn, amode)) < 0)
                sav_errno = pops->maperror (pops, 0);
        }

        if (!bool_issurlok)
            gfal_file_next_replica (gfile);
    }

    free (req);

    if (!bool_issurlok) {
        sav_errno = gfile->errcode ? gfile->errcode : ENOENT;
        rc = -1;
    }

    gfal_file_free (gfile);
    errno = sav_errno;
    return (rc);
}

    int
gfal_chmod (const char *path, mode_t mode)
{
    char pathbuf[1104];
    char *cat_type = NULL;
    int islfc;

    if (canonical_url (path, "file", pathbuf, 1104, NULL, 0) < 0)
        return (-1);

    if (get_cat_type (&cat_type) < 0)
        return (-1);
    islfc = strcmp (cat_type, "lfc") == 0;
    free (cat_type);

    if (islfc && strncmp (pathbuf, "lfn:", 4) == 0)
        return (lfc_chmodl (pathbuf + 4, mode, NULL, 0));

    /* gfal_chmod is only supported with LFC and LFNs */
    errno = EPROTONOSUPPORT;
    return (-1);
}

    int
gfal_close (int fd)
{
    int rc;
    int sav_errno = 0;
    struct xfer_info *xi;

    if (fd < 0 || (xi = find_xi (fd)) == NULL)
        return (-1);
    if ((rc = xi->pops->close (fd)) < 0)
        sav_errno = xi->pops->maperror (xi->pops, 1);

    /* set status "done" */
    if (xi->gfile && xi->gfile->gobj)
        gfal_set_xfer_done (xi->gfile->gobj, NULL, 0);

    /* set the size for a (new) lfn */
    if (xi->size >= 0 && xi->gfile && xi->gfile->lfn) {
        char *cat_type = NULL;
        int islfc;

        if (get_cat_type (&cat_type) < 0)
            return (-1);
        islfc = strcmp (cat_type, "lfc") == 0;
        free (cat_type);

        if (islfc)
            lfc_setsize (xi->gfile->lfn, xi->size, NULL, 0);
    }

    free_xi (fd);
    errno = sav_errno;
    return (rc);
}

    int
gfal_closedir (DIR *dir)
{
    struct dir_info *di;
    int rc;

    di = find_di (dir);

    if (di == NULL || di->pops == NULL) {
        return -1;
    }

    rc = di->pops->closedir (dir);

    if (rc < 0) {
        errno = di->pops->maperror (di->pops, 0);
    }

    if (strcmp (di->pops->proto_name, "lfc") == 0)
        free (di->pops);

    free_di (di);
    return (rc);
}

    int
gfal_creat (const char *filename, mode_t mode)
{
    return (gfal_open (filename, O_WRONLY|O_CREAT|O_TRUNC, mode));
}

    int
gfal_creat64 (const char *filename, mode_t mode)
{
    return (gfal_open64 (filename, O_WRONLY|O_CREAT|O_TRUNC, mode));
}

    int
gfal_set_verbose (int value)
{
    if (value < 0)
        return (-1);

    gfal_verbose = value;
    return (0);
}

    void
gfal_errmsg (char *errbuf, int errbufsz, int level, const char *format, ...)
{
    va_list ap;
    char *actual_format;

    if ((errbuf == NULL && level > gfal_verbose) || format == NULL || format[0] == 0)
        return;

    va_start (ap, format);
    asprintf (&actual_format, "%s\n", format);
    if (actual_format == NULL) return;

    if (errbuf == NULL)
        vfprintf (stderr, actual_format, ap);
    else
        vsnprintf (errbuf, errbufsz, actual_format, ap);

    free (actual_format);
}

    off_t
gfal_lseek (int fd, off_t offset, int whence)
{
    off_t offset_out;
    struct xfer_info *xi;

    if ((xi = find_xi (fd)) == NULL)
        return (-1);
    if ((offset_out = xi->pops->lseek (fd, offset, whence)) < 0)
        errno = xi->pops->maperror (xi->pops, 1);
    else errno = 0;
    return (offset_out);
}

    off64_t
gfal_lseek64 (int fd, off64_t offset, int whence)
{
    off64_t offset_out;
    struct xfer_info *xi;

    if ((xi = find_xi (fd)) == NULL)
        return (-1);
    if ((offset_out = xi->pops->lseek64 (fd, offset, whence)) < 0)
        errno = xi->pops->maperror (xi->pops, 1);
    else errno = 0;
    return (offset_out);
}

    int
gfal_mkdir (const char *dirname, mode_t mode)
{
    char path[1104], pfn[1104];
    struct proto_ops *pops = NULL;
    char protocol[64];

    if (canonical_url (dirname, "file", path, 1104, NULL, 0) < 0)
        return (-1);

    if (strncmp (path, "guid:", 5) == 0 ||
            strncmp (path, "sfn:", 4) == 0) {
        errno = EPROTONOSUPPORT;
        return (-1);
    }

    if (strncmp (path, "lfn:", 4) == 0) {
        int islfc;
        char *cat_type;
        if (get_cat_type (&cat_type) < 0)
            return (-1);

        islfc = strcmp (cat_type, "lfc") == 0;
        free (cat_type);

        /* Only LFC has a tree-like structure */
        if (islfc)
            return lfc_mkdirp (path + 4, mode, NULL, 0);

        /* So, mkdir is not supported for non-LFC file catalogs */
        errno = EPROTONOSUPPORT;
        return (-1);
    }

    if (strncmp (path, "srm:", 4) == 0) {
        // only with SRMv2 !
        int rc, i = 0;
        char **se_endpoints;
        char **se_types;
        char *srmv2_endpoint = NULL;
    	struct srm_mkdir_input mkdir_input;
    	struct srm_context context;

        if (setypesandendpointsfromsurl (path, &se_types, &se_endpoints, NULL, 0) < 0)
            return (-1);

        while (se_types[i]) {
            if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
                srmv2_endpoint = se_endpoints[i];
            else
                free (se_endpoints[i]);

            free (se_types[i]);
            ++i;
        }

        free (se_types);
        free (se_endpoints);

        if (srmv2_endpoint == NULL) {
            errno = EPROTONOSUPPORT;
            return (-1);
        }
        // init srm variables
        srm_context_init(&context,srmv2_endpoint,NULL,0,gfal_verbose);

        mkdir_input.dir_name = path;
        // srm function call
		rc = srm_mkdir(&context,&mkdir_input);
//TODO        rc = srmv2_makedirp (path, srmv2_endpoint, NULL, 0, 0);
        free (srmv2_endpoint);

        return (rc);
    }

    /* It is a TURL */
    if (parseturl (path, protocol, sizeof(protocol), pfn, sizeof(pfn), NULL, 0) < 0)
        return (-1);
    if ((pops = find_pops (protocol)) == NULL)
        return (-1);
    if (pops->mkdir (pfn, mode) < 0) {
        errno = pops->maperror (pops, 0);
        return (-1);
    }
    errno = 0;
    return (0);
}

    int
gfal_open (const char *filename, int flags, mode_t mode)
{
    char errbuf[GFAL_ERRMSG_LEN];
    int fd = -1;
    int newfile = 0;
    char protocol[64], pfn[1104];
    struct proto_ops *pops = NULL;
    char **supported_protocols = NULL;
    GFAL_LONG64 filesize = GFAL_NEWFILE_SIZE;
    struct xfer_info *xi = NULL;
    gfal_file gfile = NULL;
    gfal_request req = NULL;
    gfal_filestatus *filestatuses = NULL;
    int bool_issurlok = 0;
    int sav_errno = 0;

    supported_protocols = get_sup_proto ();

    if ((flags & (O_WRONLY | O_CREAT)) == (O_WRONLY | O_CREAT) ||
            (flags & (O_RDWR | O_CREAT)) == (O_RDWR | O_CREAT)) {
        /* writing in a file, so a new file */
        newfile = 1;
    }

    if ((gfile = gfal_file_new (filename, "file", newfile, NULL, 0)) == NULL) {
        sav_errno = errno;
        goto err;
    }
    if (gfile->errcode != 0) {
        sav_errno = gfile->errcode;
        goto err;
    }

    if ((req = gfal_request_new ()) == NULL) {
        sav_errno = errno;
        goto err;
    }

    req->nbfiles = 1;
    req->protocols = supported_protocols;
    req->no_bdii_check = gfal_is_nobdii ();

    if (newfile) {
        req->oflag = 1;
        req->filesizes = &filesize;
    }

    if (newfile && !gfile->turl && gfile->nbreplicas == 0) {
        char *default_se, *surl = NULL;

        if ((default_se = get_default_se(NULL, 0)) == NULL) {
            sav_errno = errno;
            goto err;
        }

        req->endpoint = default_se;
        req->generatesurls = 1;

        if (gfal_init (req, &(gfile->gobj), NULL, 0) < 0) {
            sav_errno = errno;
            goto err;
        }

        if (gfile->gobj != NULL && gfile->gobj->surls != NULL && gfile->gobj->surls[0] != NULL)
            surl = strdup (gfile->gobj->surls[0]);

        if (surl == NULL) {
            sav_errno = errno;
            goto err;
        }

        gfile->nbreplicas = 1;
        gfile->replicas = (gfal_replica *) calloc (1, sizeof (gfal_replica));
        if (gfile->replicas == NULL) {
            sav_errno = errno;
            goto err;
        }
        *(gfile->replicas) = (gfal_replica) calloc (1, sizeof (struct _gfal_replica));
        if (*(gfile->replicas) == NULL) {
            sav_errno = errno;
            goto err;
        }
        gfile->replicas[0]->surl = surl;
    }

    while (!bool_issurlok && gfile->errcode == 0) {
        bool_issurlok = 1;

        if (gfile->nbreplicas > 0) {
            if (gfile->gobj == NULL) {
                req->surls = (char **) &(gfile->replicas[gfile->current_replica]->surl);

                if (!(bool_issurlok = gfal_init (req, &gfile->gobj, errbuf, GFAL_ERRMSG_LEN) >= 0))
                    gfal_file_set_replica_error (gfile, errno, errbuf);
            }

            if (bool_issurlok && !(bool_issurlok = gfal_turlsfromsurls (gfile->gobj, errbuf, GFAL_ERRMSG_LEN) >= 0))
                gfal_file_set_replica_error (gfile, errno, errbuf);

            if (bool_issurlok && (!(bool_issurlok = gfal_get_results (gfile->gobj, &filestatuses) >= 0) ||
                        !(bool_issurlok = filestatuses != NULL))) {
                snprintf (errbuf, GFAL_ERRMSG_LEN, "Internal error");
                gfal_file_set_replica_error (gfile, errno, errbuf);
            }

            if (bool_issurlok && !(bool_issurlok = filestatuses[0].status == 0))
                gfal_file_set_replica_error (gfile, filestatuses[0].status, filestatuses[0].explanation);

            if (bool_issurlok) {
                gfile->turl = strdup (filestatuses[0].turl);
                if (gfile->turl == NULL) {
                    sav_errno = errno;
                    goto err;
                }
            }
        }

        if (bool_issurlok && !(bool_issurlok = parseturl (gfile->turl, protocol, sizeof(protocol),
                        pfn, sizeof(pfn), errbuf, GFAL_ERRMSG_LEN) >= 0))
            gfal_file_set_turl_error (gfile, errno, errbuf);

        if (bool_issurlok) {
            pops = find_pops (protocol);
            if (!(bool_issurlok = pops != NULL))
                gfal_file_set_turl_error (gfile, EPROTONOSUPPORT, NULL);
        }

        if (bool_issurlok) {
            fd = pops->open (pfn, flags, mode);
            if (!(bool_issurlok = fd >= 0))
                gfal_file_set_turl_error (gfile, pops->maperror (pops, 1), NULL);
        }

        if (!bool_issurlok)
            gfal_file_next_replica (gfile);
    }

    if (!bool_issurlok) {
        sav_errno = gfile->errcode ? gfile->errcode : 0;
        goto err;
    }

    if ((xi = alloc_xi (fd)) == NULL) {
        sav_errno = errno;
        goto err;
    }

    xi->fd = fd;
    xi->gfile = gfile;
    xi->pops = pops;

    if (newfile) xi->size = 0;
    else         xi->size = -1;

    if (gfile->gobj)
        gfal_set_xfer_running (gfile->gobj, NULL, 0);

    if (newfile && gfile->nbreplicas == 1 && (gfile->lfn || gfile->guid) &&
            gfile->replicas != NULL && gfile->replicas[0] != NULL &&
            gfile->replicas[0]->surl != NULL) {
        if (gfal_register_file (gfile->lfn, gfile->guid, gfile->replicas[0]->surl, mode, 0, 1, NULL, 0) < 0 ) {
            sav_errno = errno;
            goto err;
        }
    }

    if (req) free (req);

    errno = 0;
    return (fd);

err:
    if (fd >= 0) {
        gfal_deletesurls (gfile->gobj, NULL, 0);
        free_xi (fd);
    } else if (gfile) {
        gfal_file_free (gfile);
    }
    
    if (req) {
        free (req);
    }


    errno = sav_errno;
    return (-1);
}

    int
gfal_open64 (const char *filename, int flags, mode_t mode)
{
    return (gfal_open (filename, flags | O_LARGEFILE, mode));
}

    DIR *
gfal_opendir (const char *dirname)
{
    struct dir_info *di;
    DIR *dir;
    char path[1104], pfn[1104];
    struct proto_ops *pops = NULL;
    char protocol[64];
    int islfn = 0;

    if (canonical_url (dirname, "file", path, 1104, NULL, 0) < 0)
        return (NULL);

    if ((strncmp (path, "lfn:", 4) == 0 && (islfn = 1)) ||
            strncmp (path, "guid:", 5) == 0) {
        void *dlhandle;
        char *cat_type;

        if (get_cat_type (&cat_type) < 0)
            return (NULL);
        if (strcmp (cat_type, "lfc") != 0) {
            errno = EPROTONOSUPPORT;
            return (NULL);
        }
        free (cat_type);

        if (islfn) dir = (DIR *) lfc_opendirlg (path + 4, NULL, NULL, 0);
        else	   dir = (DIR *) lfc_opendirlg (NULL, path, NULL, 0);

        if (dir == NULL) return (NULL);
        if ((di = alloc_di (dir)) == NULL)
            return (NULL);
        if ((pops = (struct proto_ops *) malloc (sizeof(struct proto_ops))) == NULL) {
            errno = ENOMEM;
            return (NULL);
        }
        memset (pops, 0, sizeof(struct proto_ops));

        if ((dlhandle = dlopen ("liblfc.so", RTLD_LAZY)) == NULL)
            return (NULL);

        pops->proto_name = "lfc";
        pops->maperror = lfc_maperror;
        pops->readdir = (struct dirent * (*) (DIR *)) dlsym (dlhandle, "lfc_readdir");
        pops->readdir64 = (struct dirent64 * (*) (DIR *)) dlsym (dlhandle, "lfc_readdir64");
        pops->closedir = (int (*) (DIR *)) dlsym (dlhandle, "lfc_closedir");

        di->pops = pops;
        errno = 0;
        return dir;
    }

    if (strncmp (path, "srm:", 4) == 0 ||
            strncmp (path, "sfn:", 4) == 0) {
        errno = EPROTONOSUPPORT;
        return (NULL);
    }
    if (parseturl (path, protocol, sizeof(protocol), pfn, sizeof(pfn), NULL, 0) < 0)
        return (NULL);
    if ((pops = find_pops (protocol)) == NULL)
        return (NULL);
    if ((dir = pops->opendir (pfn)) == NULL) {
        errno = pops->maperror (pops, 0);
        return (NULL);
    }
    if ((di = alloc_di (dir)) == NULL)
        return (NULL);
    di->pops = pops;
    errno = 0;
    return (dir);
}

    ssize_t
gfal_read (int fd, void *buf, size_t size)
{
    int rc;
    struct xfer_info *xi;

    if ((xi = find_xi (fd)) == NULL)
        return (-1);
    if ((rc = xi->pops->read (fd, buf, size)) < 0)
        errno = xi->pops->maperror (xi->pops, 1);
    else
        errno = 0;
    return (rc);
}

    struct dirent *
gfal_readdir (DIR *dir)
{
    struct dirent *de;
    struct dir_info *di;

    if (dir == NULL || (di = find_di (dir)) == NULL)
        return (NULL);
    if ((de = di->pops->readdir (dir)) == NULL)
        errno = di->pops->maperror (di->pops, 0);
    else
        errno = 0;
    return (de);
}

    struct dirent64 *
gfal_readdir64 (DIR *dir)
{
    struct dirent64 *de;
    struct dir_info *di;

    if ((di = find_di (dir)) == NULL)
        return (NULL);
    if ((de = di->pops->readdir64 (dir)) == NULL)
        errno = di->pops->maperror (di->pops, 0);
    else
        errno = 0;
    return (de);
}

    int
gfal_rename (const char *old_name, const char *new_name)
{
    char path1[1104], pfn1[1104];
    char path2[1104], pfn2[1104];
    struct proto_ops *pops = NULL;
    char protocol1[64];
    char protocol2[64];

    if (canonical_url (old_name, "file", path1, 1104, NULL, 0) < 0 ||
            canonical_url (new_name, "file", path2, 1104, NULL, 0) < 0)
        return (-1);

    if (strncmp (path1, "lfn:", 4) == 0 && strncmp (path2, "lfn:", 4) == 0) {
        int islfc,isedg;
        char *cat_type;
        if (get_cat_type (&cat_type) < 0)
            return (-1);

        islfc = strcmp (cat_type, "lfc") == 0;
        isedg = strcmp (cat_type, "edg") == 0;
        free (cat_type);

        if (islfc)
            return lfc_renamel (path1 + 4, path2 + 4, NULL, 0);
        else if (isedg) {
            char *guid;
            int rc;

            if ((guid = guidfromlfn (path1 + 4, NULL, 0)) == NULL)
                return (-1);
            if (rmc_register_alias (guid, path2, NULL, 0) < 0) {
                free (guid);
                return (-1);
            }
            rc = rmc_unregister_alias (guid, path1, NULL, 0);
            free (guid);
            return (rc);
        }

        errno = EPROTONOSUPPORT;
        return (-1);
    }

    if (strncmp (path1, "lfn:", 4) == 0 ||
            strncmp (path1, "guid:", 5) == 0 ||
            strncmp (path1, "srm:", 4) == 0 ||
            strncmp (path1, "sfn:", 4) == 0) {
        errno = EPROTONOSUPPORT;
        return (-1);
    }
    if (strncmp (path2, "lfn:", 4) == 0 ||
            strncmp (path2, "guid:", 5) == 0 ||
            strncmp (path2, "srm:", 4) == 0 ||
            strncmp (path2, "sfn:", 4) == 0) {
        errno = EPROTONOSUPPORT;
        return (-1);
    }
    if (parseturl (path1, protocol1, sizeof(protocol1), pfn1, sizeof(pfn1), NULL, 0) < 0)
        return (-1);
    if (parseturl (path2, protocol2, sizeof(protocol2), pfn2, sizeof(pfn2), NULL, 0) < 0)
        return (-1);
    if (strcmp (protocol1, protocol2)) {
        errno = EINVAL;
        return (-1);
    }
    if ((pops = find_pops (protocol1)) == NULL)
        return (-1);
    if (pops->rename (pfn1, pfn2) < 0) {
        errno = pops->maperror (pops, 0);
        return (-1);
    }
    errno = 0;
    return (0);
}

    int
gfal_rmdir (const char *dirname)
{
    char path[1104], pfn[1104];
    struct proto_ops *pops = NULL;
    char protocol[64];

    if (canonical_url (dirname, "file", path, 1104, NULL, 0) < 0)
        return (-1);

    if (strncmp (path, "lfn:", 4) == 0) {
        int islfc;
        char *cat_type;
        if (get_cat_type (&cat_type) < 0)
            return (-1);

        islfc = strcmp (cat_type, "lfc") == 0;
        free (cat_type);

        if (islfc)
            return lfc_rmdirl (path + 4, NULL, 0);

        errno = EPROTONOSUPPORT;
        return (-1);
    }

    if (strncmp (path, "srm:", 4) == 0) {
        int rc, i = 0;
        char **se_endpoints;
        char **se_types;
        char *srmv2_endpoint = NULL;
        struct srm_context context;
        struct srm_rmdir_input rmdir_input;
        struct srm_rmdir_output rmdir_output;


        if (setypesandendpointsfromsurl (path, &se_types, &se_endpoints, NULL, 0) < 0)
            return (-1);

        while (se_types[i]) {
            if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
                srmv2_endpoint = se_endpoints[i];
            else
                free (se_endpoints[i]);

            free (se_types[i]);
            ++i;
        }

        free (se_types);
        free (se_endpoints);

        if (srmv2_endpoint == NULL) {
            errno = EPROTONOSUPPORT;
            return (-1);
        }
        srm_context_init(&context,srmv2_endpoint,NULL,0,gfal_verbose);

        rmdir_input.recursive = 0;
        rmdir_input.surl = path;

        if (srm_rmdir(&context,&rmdir_input,&rmdir_output) < 1 || !rmdir_output.statuses)
        {
        	free(srmv2_endpoint);
        	return (-1);
        }

/*TODO  if (srmv2_rmdir (path, srmv2_endpoint, 0, &filestatuses, NULL, 0, 0) < 1 || !filestatuses) {
            free (srmv2_endpoint);
            return (-1);
        }*/
        if (rmdir_output.statuses[0].surl) free (rmdir_output.statuses[0].surl);
        errno = rmdir_output.statuses[0].status;
        rc = rmdir_output.statuses[0].status == 0 ? 0 : -1;
        free (rmdir_output.statuses);
        free (srmv2_endpoint);
        return (rc);
    }

    if (strncmp (path, "guid:", 5) == 0 ||
            strncmp (path, "sfn:", 4) == 0) {
        errno = EPROTONOSUPPORT;
        return (-1);
    }
    if (parseturl (path, protocol, sizeof(protocol), pfn, sizeof(pfn), NULL, 0) < 0)
        return (-1);
    if ((pops = find_pops (protocol)) == NULL)
        return (-1);
    if (pops->rmdir (pfn) < 0) {
        errno = pops->maperror (pops, 0);
        return (-1);
    }
    errno = 0;
    return (0);
}

    ssize_t
gfal_setfilchg (int fd, const void *buf, size_t size)
{
    int rc;
    struct xfer_info *xi;

    if ((xi = find_xi (fd)) == NULL)
        return (-1);
    if ((rc = xi->pops->setfilchg (fd, buf, size)) < 0)
        errno = xi->pops->maperror (xi->pops, 1);
    else errno = 0;
    return (rc);
}


#if !defined(linux) || defined(_LARGEFILE64_SOURCE)
    int
gfal_stat_generic (const char *filename, int bool_link, struct stat64 *statbuf)
{
    int rc = 0, sav_errno = 0;
    char errbuf[GFAL_ERRMSG_LEN];
    gfal_file gfile;

    if ((gfile = gfal_file_new (filename, "file", 0, NULL, 0)) == NULL)
        return (-1);
    if (gfile->errcode != 0 && gfile->errcode != EISDIR) {
        sav_errno = gfile->errcode;
        gfal_file_free (gfile);
        errno = sav_errno;
        return (-1);
    }

    if (gfile->catalog == GFAL_FILE_CATALOG_LFC) {
        if (!bool_link)
            rc = lfc_statl (gfile->lfn, gfile->guid, statbuf, NULL, 0);
        else
            rc = lfc_lstatl (gfile->lfn, statbuf, NULL, 0);
        sav_errno = errno;
    }
    else if (gfile->nbreplicas > 0) {
        gfal_request req;
        gfal_filestatus *filestatuses;
        const char *current_surl;
        int bool_issurlok = 0;

        if ((req = gfal_request_new ()) == NULL) {
            sav_errno = errno;
            gfal_file_free (gfile);
            errno = sav_errno;
            return (-1);
        }

        req->nbfiles = 1;
        req->no_bdii_check = gfal_is_nobdii ();

        while (!bool_issurlok && gfile->errcode == 0) {
            current_surl = gfal_file_get_replica (gfile);
            req->surls = (char **) &current_surl;

            if (!(bool_issurlok = gfal_init (req, &gfile->gobj, errbuf, GFAL_ERRMSG_LEN) >= 0))
                gfal_file_set_replica_error (gfile, errno, errbuf);

            if (bool_issurlok && !(bool_issurlok = gfal_ls (gfile->gobj, errbuf, GFAL_ERRMSG_LEN) >= 0))
                gfal_file_set_replica_error (gfile, errno, errbuf);

            if (bool_issurlok && (!(bool_issurlok = gfal_get_results (gfile->gobj, &filestatuses) >= 0) ||
                        !(bool_issurlok = filestatuses != NULL))) {
                snprintf (errbuf, GFAL_ERRMSG_LEN, "Internal error");
                gfal_file_set_replica_error (gfile, errno, errbuf);
            }

            if (bool_issurlok && !(bool_issurlok = filestatuses[0].status == 0))
                gfal_file_set_replica_error (gfile, filestatuses[0].status, filestatuses[0].explanation);

            if (bool_issurlok)
                memcpy (statbuf, &(filestatuses[0].stat), sizeof (struct stat));

            if (!bool_issurlok)
                gfal_file_next_replica (gfile);
        }

        free (req);

        if (!bool_issurlok) {
            sav_errno = gfile->errcode ? gfile->errcode : ENOENT;
            rc = -1;
        } else {
            sav_errno = 0;
            rc = 0;
        }

        gfal_file_free (gfile);
        errno = sav_errno;
        return (rc);
    }

    if (gfile->turl != NULL) {
        struct proto_ops *pops = NULL;
        char protocol[64], pfn[1104];

        if ((rc = parseturl (gfile->turl, protocol, sizeof(protocol), pfn, sizeof(pfn), NULL, 0)) == 0) {
            if ((pops = find_pops (protocol)) != NULL) {
                if (!bool_link)
                    rc = pops->stat64 (pfn, statbuf);
                else
                    rc = pops->lstat64 (pfn, statbuf);
                if (rc < 0)
                    sav_errno = pops->maperror (pops, 0);
            } else {
                rc = -1;
                sav_errno = errno;
            }
        } else sav_errno = errno;
    }

    gfal_file_free (gfile);
    errno = sav_errno;
    return (rc);
}
#endif

#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
    int
gfal_stat (const char *filename, struct stat *statbuf)
{
    struct stat64 stat64buf;
    int rc = 0;

    if (gfal_stat_generic (filename, 0, &stat64buf) < 0)
        return (-1);

#if defined(__USE_FILE_OFFSET64) || __WORDSIZE == 64
    memcpy (statbuf, &stat64buf, sizeof (struct stat));
    rc = 0;
#else
    rc = mdtomd32 (&stat64buf, statbuf);
#endif
    return (0);
}
#endif


#if !defined(linux) || defined(_LARGEFILE64_SOURCE)
    int
gfal_stat64 (const char *filename, struct stat64 *statbuf)
    /*
#elif defined(__USE_FILE_OFFSET64)
gfal_stat64 (const char *filename, struct stat *statbuf)
#endif
#if !defined(linux) || defined(_LARGEFILE64_SOURCE) || defined(__USE_FILE_OFFSET64)
     */
{
    return (gfal_stat_generic (filename, 0, statbuf));
}
#endif

#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
    int
gfal_lstat (const char *filename, struct stat *statbuf)
{
    struct stat64 stat64buf;
    int rc = 0;

    if (gfal_stat_generic (filename, 1, &stat64buf) < 0)
        return (-1);

#if defined(__USE_FILE_OFFSET64) || __WORDSIZE == 64
    memcpy (statbuf, &stat64buf, sizeof (struct stat));
    rc = 0;
#else
    rc = mdtomd32 (&stat64buf, statbuf);
#endif
    return (0);
}
#endif


#if !defined(linux) || defined(_LARGEFILE64_SOURCE)
    int
gfal_lstat64 (const char *filename, struct stat64 *statbuf)
    /*
#elif defined(__USE_FILE_OFFSET64)
gfal_stat64 (const char *filename, struct stat *statbuf)
#endif
#if !defined(linux) || defined(_LARGEFILE64_SOURCE) || defined(__USE_FILE_OFFSET64)
     */
{
    return (gfal_stat_generic (filename, 1, statbuf));
}
#endif

    int
gfal_unlink (const char *filename)
{
    int i, rc = 0;
    int sav_errno = 0;
    gfal_file gfile;

    if ((gfile = gfal_file_new (filename, "file", 0, NULL, 0)) == NULL) {
        sav_errno = errno;
        return (-1);
    }
    if (gfile->errcode != 0) {
        sav_errno = gfile->errcode;
        gfal_file_free (gfile);
        errno = sav_errno;
        return (-1);
    }

    if (gfile->nbreplicas > 0 && gfile->replicas != NULL) {
        int bool_issurlok = 1;
        const char *current_surl;
        gfal_request req;
        gfal_internal gobj;
        gfal_filestatus *filestatuses;

        if ((req = gfal_request_new ()) == NULL) {
            sav_errno = errno;
            gfal_file_free (gfile);
            errno = sav_errno;
            return (-1);
        }

        req->nbfiles = 1;
        req->no_bdii_check = gfal_is_nobdii ();

        for (i = 0; i < gfile->nbreplicas; ++i) {
            if (gfile->replicas[i] == NULL)
                continue;

            current_surl = gfal_file_get_replica (gfile);
            req->surls = (char **) &current_surl;

            if (!(bool_issurlok = gfal_init (req, &gobj, NULL, 0) >= 0))
                gfal_file_set_replica_error (gfile, errno, NULL);

            if (bool_issurlok && !(bool_issurlok = gfal_deletesurls (gobj, NULL, 0) >= 0))
                gfal_file_set_replica_error (gfile, errno, NULL);

            if (bool_issurlok && (!(bool_issurlok = gfal_get_results (gobj, &filestatuses) > 0) ||
                        !(bool_issurlok = filestatuses != NULL))) {
                gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_unlink][] %s: Internal error", current_surl);
                gfal_file_set_replica_error (gfile, errno, NULL);
            }

            if (bool_issurlok && !(bool_issurlok = filestatuses[0].status == 0)) {
                gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_unlink][] %s: %s", current_surl, filestatuses[0].explanation);
                gfal_file_set_replica_error (gfile, filestatuses[0].status, NULL);
            }

            if (bool_issurlok) {
                if (gfile->lfn)
                    gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_INFO, "[INFO] lfn:%s - %s > DELETED",
                            gfile->lfn, current_surl);
                else if (gfile->guid)
                    gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_INFO, "[INFO] guid:%s - %s > DELETED",
                            gfile->guid, current_surl);
                else
                    gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_INFO, "[INFO] %s > DELETED", current_surl);
            }

            gfal_internal_free (gobj);
        }

        free (req);

        if (!bool_issurlok) {
            sav_errno = gfile->errcode;
            gfal_file_free (gfile);
            errno = sav_errno;
            return (-1);
        }
    }

    if (gfile->catalog == GFAL_FILE_CATALOG_LFC) {
        rc = lfc_remove (gfile, NULL, 0);
        sav_errno = gfile->errcode;
        gfal_file_free (gfile);
        errno = sav_errno;
        return (rc);
    }
    if (gfile->catalog == GFAL_FILE_CATALOG_EDG) {
        for (i = 0; i < gfile->nbreplicas; ++i) {
            if (gfile->replicas[i] == NULL ||
                    gfile->replicas[i]->surl == NULL ||
                    gfile->replicas[i]->errcode != 0)
                continue;

            if (lrc_unregister_pfn (gfile->guid, gfile->replicas[i]->surl, NULL, 0) < 0)
                gfal_file_set_replica_error (gfile, errno, NULL);
            else {
                gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_INFO, "[INFO] %s - %s > UNREGISTERED",
                        gfile->guid, gfile->replicas[i]->surl);
            }

        }

        if (gfile->nberrors == 0) {
            char **lfns = rmc_lfnsforguid (gfile->guid, NULL, 0);
            if (lfns != NULL) {
                for (i = 0; lfns[i]; ++i) {
                    if (rmc_unregister_alias (gfile->guid, lfns[i], NULL, 0) < 0)
                        gfile->errcode = errno ? errno : ECOMM;
                    else {
                        gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_INFO, "[INFO] %s - %s > UNREGISTERED",
                                gfile->guid, lfns[i]);
                    }

                    free (lfns[i]);
                }
                free (lfns);
            }
        }

        sav_errno = gfile->errcode;
        gfal_file_free (gfile);
        errno = sav_errno;
        return (sav_errno ? -1 : 0);
    }
    if (gfile->turl != NULL) {
        char pfn[1104];
        struct proto_ops *pops = NULL;
        char protocol[64];

        if (parseturl (gfile->turl, protocol, sizeof(protocol), pfn, sizeof(pfn), NULL, 0) == 0) {
            if ((pops = find_pops (protocol)) != NULL) {
                if (pops->unlink (pfn) < 0)
                    sav_errno = pops->maperror (pops, 0);
            } else {
                sav_errno = errno ? errno : ECOMM;
            }
        } else
            sav_errno = errno ? errno : EINVAL;

        gfal_file_free (gfile);
        errno = sav_errno;
        return (sav_errno ? -1 : 0);
    }

	gfal_file_free (gfile);
    return (0);
}

    ssize_t
gfal_write (int fd, const void *buf, size_t size)
{
    int rc;
    struct xfer_info *xi;

    if ((xi = find_xi (fd)) == NULL)
        return (-1);
    if ((rc = xi->pops->write (fd, buf, size)) < 0) {
        errno = xi->pops->maperror (xi->pops, 1);
        return (rc);
    }
    if (xi->size >= 0) xi->size += rc;
    errno = 0;
    return (rc);
}

    int
gfal_deletesurls (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
		struct srm_rm_input rm_input;
		struct srm_rm_output rm_output;

        if (req->srmv2_statuses)
        {
            free (req->srmv2_statuses);
            req->srmv2_statuses = NULL;
        }
        if (req->srmv2_token)
        {
            free (req->srmv2_token);
            req->srmv2_token = NULL;
        }


        srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);
        rm_input.nbfiles = req->nbfiles;
        rm_input.surls = req->surls;

       ret = srm_rm(&context,&rm_input,&rm_output);

       req->srmv2_statuses = rm_output.statuses;

        /*TODO ret = srmv2_deletesurls (req->nbfiles, (const char **) req->surls, req->endpoint,
                &(req->srmv2_statuses), errbuf, errbufsz, req->timeout);*/
    } else if (req->setype == TYPE_SRM) {
        if (req->srm_statuses) {
            free (req->srm_statuses);
            req->srm_statuses = NULL;
        }
/*TODO REMOVE       ret = srm_deletesurls (req->nbfiles, (const char **) req->surls, req->endpoint,
                &(req->srm_statuses), errbuf, errbufsz, req->timeout);*/
    } else { // req->setype == TYPE_SE
        if (req->sfn_statuses) {
            free (req->sfn_statuses);
            req->sfn_statuses = NULL;
        }
        ret = sfn_deletesurls (req->nbfiles, (const char **) req->surls,
                &(req->sfn_statuses), errbuf, errbufsz, gfal_get_timeout_sendreceive ());
    }

    req->returncode = ret;
    return (copy_gfal_results (req, DEFAULT_STATUS));
}

    int
gfal_removedir (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->nbfiles != 1 || req->surls == NULL || req->surls[0] == NULL) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_removedir][EINVAL] you have to specify only one directory SURL at a time");
        errno = EINVAL;
        return (-1);;
    }

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_rmdir_input rmdir_input;
    	struct srm_rmdir_output rmdir_output;
    	struct srm_context context;

    	if (req->srmv2_statuses)
        {
            free (req->srmv2_statuses);
            req->srmv2_statuses = NULL;
        }
        if (req->srmv2_token)
        {
            free (req->srmv2_token);
            req->srmv2_token = NULL;
        }

        srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);
        rmdir_input.recursive = 0;
        rmdir_input.surl = req->surls[0];

        ret = srm_rmdir(&context,&rmdir_input,&rmdir_output);

        req->srmv2_statuses = rmdir_output.statuses;


        /* for the moment, there is no field in the gfal struct for recursive removal */
//TODO        ret = srmv2_rmdir (req->surls[0], req->endpoint, 0, &(req->srmv2_statuses), errbuf, errbufsz, req->timeout);
    } else { // req->setype == TYPE_SRM or TYPE_SE
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_removedir][EPROTONOSUPPORT] only SRMv2.2 supports this operation");
        errno = EPROTONOSUPPORT;
        return (-1);;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, DEFAULT_STATUS));
}
int gfal_create_subdirs(gfal_internal req, char *errbuf, int errbufsz)
{
	int i,result = 0 ;
	struct srm_context context;
	struct srm_mkdir_input mkdir_input;

	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);
	/* Create sub-directories of SURLs */
	for (i = 0; i < req->nbfiles; ++i) {
        const char* dir = gfal_strip_string(req->surls[i], '/');
        int res = 0;


        assert (dir);

        if (dir && strlen(dir) > 0)
        {
        	mkdir_input.dir_name = dir;

            res = srm_mkdir(&context,&mkdir_input);
        }

        free(dir);

        if (res < 0) {
        	result = -1;
        }
	}
	return result;
}
    int
gfal_turlsfromsurls (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_pinstatuses)
        {
            free (req->srmv2_pinstatuses);
            req->srmv2_pinstatuses = NULL;
        }
        if (req->srmv2_token)
        {
            free (req->srmv2_token);
            req->srmv2_token = NULL;
        }
        if ((req->oflag & O_ACCMODE) == 0)
        {
        	struct srm_preparetoget_input preparetoget_input;
        	struct srm_preparetoget_output preparetoget_output;

        	preparetoget_input.nbfiles  = req->nbfiles;
        	preparetoget_input.surls  = req->surls;
        	preparetoget_input.desiredpintime  = req->srmv2_desiredpintime;
        	preparetoget_input.spacetokendesc = req->srmv2_spacetokendesc;
        	preparetoget_input.protocols = req->protocols;

        	ret = srm_prepare_to_get(&context,&preparetoget_input,&preparetoget_output);

        	req->srmv2_pinstatuses = preparetoget_output.filestatuses;
        	req->srmv2_token = preparetoget_output.token;

			/*TODO ret = srmv2_turlsfromsurls_get (req->nbfiles, (const char **) req->surls, req->endpoint,
					req->srmv2_desiredpintime, req->srmv2_spacetokendesc, req->protocols,
					&(req->srmv2_token), &(req->srmv2_pinstatuses), errbuf, errbufsz, req->timeout);*/
        }else
        {
        	struct srm_preparetoput_input preparetoput_input;
        	struct srm_preparetoput_output preparetoput_output;

        	preparetoput_input.nbfiles  = req->nbfiles;
        	preparetoput_input.surls  = req->surls;
        	preparetoput_input.desiredpintime  = req->srmv2_desiredpintime;
        	preparetoput_input.spacetokendesc = req->srmv2_spacetokendesc;
        	preparetoput_input.protocols = req->protocols;
        	preparetoput_input.filesizes = req->filesizes;

        	gfal_create_subdirs(req,errbuf,errbufsz);

        	ret = srm_prepare_to_put(&context,&preparetoput_input,&preparetoput_output);

        	req->srmv2_token = preparetoput_output.token;
        	req->srmv2_pinstatuses = preparetoput_output.filestatuses;

        	/*TODO ret = srmv2_turlsfromsurls_put (req->nbfiles, (const char **) req->surls, req->endpoint,
        			req->filesizes, req->srmv2_desiredpintime, req->srmv2_spacetokendesc, req->protocols,
					&(req->srmv2_token), &(req->srmv2_pinstatuses), errbuf, errbufsz, req->timeout);*/
        }
    } else if (req->setype == TYPE_SRM) {
        if (req->srm_statuses) {
            free (req->srm_statuses);
            req->srm_statuses = NULL;
        }
/*TODO REMOVE        ret = srm_turlsfromsurls (req->nbfiles, (const char **) req->surls, req->endpoint,
                req->filesizes, req->protocols, req->oflag, &(req->srm_reqid),
                &(req->srm_statuses), errbuf, errbufsz, req->timeout);*/
    } else { // req->setype == TYPE_SE
        if (req->sfn_statuses) {
            free (req->sfn_statuses);
            req->sfn_statuses = NULL;
        }
        ret = sfn_turlsfromsurls (req->nbfiles, (const char **) req->surls, req->protocols,
                &(req->sfn_statuses), errbuf, errbufsz);
    }

    req->returncode = ret;
    return (copy_gfal_results (req, PIN_STATUS));
}

    int
gfal_ls (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
    	struct srm_ls_input ls_input;
    	struct srm_ls_output ls_output;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_mdstatuses)
        {
            free_srmv2_mdstatuses (req->srmv2_mdstatuses, req->results_size);
            req->srmv2_mdstatuses = NULL;
        }
        if (req->srmv2_token)
        {
            free (req->srmv2_token);
            req->srmv2_token = NULL;
        }
        ls_input.nbfiles = req->nbfiles;
        ls_input.surls = req->surls;
        ls_input.numlevels = req->srmv2_lslevels;
        ls_input.offset = &(req->srmv2_lsoffset);
        ls_input.count = req->srmv2_lscount;

        ret = srm_ls(&context,&ls_input,&ls_output);

        req->srmv2_mdstatuses = ls_output.statuses;
        req->srmv2_token = ls_output.token;


      /*TODO  ret = srmv2_getfilemd (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_lslevels,
                &(req->srmv2_lsoffset), req->srmv2_lscount, &(req->srmv2_mdstatuses), &(req->srmv2_token),
                errbuf, errbufsz, req->timeout);*/
    } else if (req->setype == TYPE_SRM) {
        if (req->srm_mdstatuses) {
            free (req->srm_mdstatuses);
            req->srm_mdstatuses = NULL;
        }
/*TODO        ret = srm_getfilemd (req->nbfiles, (const char **) req->surls, req->endpoint,
                &(req->srm_mdstatuses), errbuf, errbufsz, req->timeout);*/
    } else { // req->setype == TYPE_SE
        /* sfn_getfilemd uses srmv2_mdstatuses field!! */
        if (req->sfn_statuses) {
            free (req->sfn_statuses);
            req->sfn_statuses = NULL;
        }
        if (req->srmv2_mdstatuses) {
            free (req->srmv2_mdstatuses);
            req->srmv2_mdstatuses = NULL;
        }
        ret = sfn_getfilemd (req->nbfiles, (const char **) req->surls,
                &(req->srmv2_mdstatuses), errbuf, errbufsz, gfal_get_timeout_sendreceive ());
    }

    req->returncode = ret;
    return (copy_gfal_results (req, MD_STATUS));
}

    int
gfal_ls_end (gfal_internal req, char *errbuf, int errbufsz)
{
    if (req == NULL) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_ls_end][EINVAL] Invalid argument");
        return (-1);
    }

    return (req->srmv2_lsoffset == 0);
}

    int
gfal_get (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
    	struct srm_preparetoget_input preparetoget_input;
    	struct srm_preparetoget_output preparetoget_output;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_pinstatuses)
        {
            free (req->srmv2_pinstatuses);
            req->srmv2_pinstatuses = NULL;
        }
        if (req->srmv2_token)
        {
            free (req->srmv2_token);
            req->srmv2_token = NULL;
        }

        preparetoget_input.desiredpintime = req->srmv2_desiredpintime;
        preparetoget_input.nbfiles = req->nbfiles;
        preparetoget_input.protocols = req->protocols;
        preparetoget_input.spacetokendesc = req->srmv2_spacetokendesc;
        preparetoget_input.surls = req->surls;

        ret = srm_prepare_to_get_async(&context,&preparetoget_input,&preparetoget_output);

    	req->srmv2_token = preparetoget_output.token;
    	req->srmv2_pinstatuses = preparetoget_output.filestatuses;


        /*TODO ret = srmv2_gete (req->nbfiles, (const char **) req->surls, req->endpoint,
            req->srmv2_desiredpintime, req->srmv2_spacetokendesc, req->protocols,
            &(req->srmv2_token), &(req->srmv2_pinstatuses), errbuf, errbufsz, req->timeout);*/

    } else if (req->setype == TYPE_SRM) {
        if (req->srm_statuses) {
            free (req->srm_statuses);
            req->srm_statuses = NULL;
        }
/*TODO REMOVE        ret = srm_getxe (req->nbfiles, (const char **) req->surls, req->endpoint, req->protocols,
                &(req->srm_reqid), &(req->srm_statuses), errbuf, errbufsz, req->timeout);*/
    } else { // req->setype == TYPE_SE
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_get][EPROTONOSUPPORT] SFNs aren't supported");
        errno = EPROTONOSUPPORT;
        return (-1);;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, PIN_STATUS));
}

    int
gfal_getstatus (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
    	struct srm_preparetoget_input preparetoget_input;
    	struct srm_preparetoget_output preparetoget_output;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_pinstatuses)
        {
            free (req->srmv2_pinstatuses);
            req->srmv2_pinstatuses = NULL;
        }

        preparetoget_output.token = req->srmv2_token;

        ret = srm_status_of_get_request_async(&context,&preparetoget_input,&preparetoget_output);

        req->srmv2_pinstatuses = preparetoget_output.filestatuses;

        /*TODO ret = srmv2_getstatuse (req->srmv2_token, req->endpoint, &(req->srmv2_pinstatuses),
                errbuf, errbufsz, req->timeout);*/
    } else if (req->setype == TYPE_SRM) {
        if (req->srm_statuses) {
            free (req->srm_statuses);
            req->srm_statuses = NULL;
        }
/*TODO        ret = srm_getstatusxe (req->srm_reqid, req->endpoint, &(req->srm_statuses),
                errbuf, errbufsz, req->timeout);*/
    } else { // req->setype == TYPE_SE
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_getstatus][EPROTONOSUPPORT] SFNs aren't supported");
        errno = EPROTONOSUPPORT;
        return (-1);;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, PIN_STATUS));
}

    int
gfal_bringonline (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
    	struct srm_bringonline_input bringonline_input;
    	struct srm_bringonline_output bringonline_output;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_pinstatuses)
        {
            free (req->srmv2_pinstatuses);
            req->srmv2_pinstatuses = NULL;
        }
        if (req->srmv2_token)
        {
            free (req->srmv2_token);
            req->srmv2_token = NULL;
        }
        bringonline_input.desiredpintime = req->srmv2_desiredpintime;
        bringonline_input.nbfiles = req->nbfiles;
        bringonline_input.protocols =  req->protocols;
        bringonline_input.spacetokendesc = req->srmv2_spacetokendesc;
        bringonline_input.surls = req->surls;

		ret = srm_bring_online(&context,&bringonline_input,&bringonline_output);

		req->srmv2_token = bringonline_output.token;
		req->srmv2_pinstatuses = bringonline_output.filestatuses;

        /*TODO ret = srmv2_bringonline (req->nbfiles, (const char **) req->surls, req->endpoint,
            req->srmv2_desiredpintime, req->srmv2_spacetokendesc, req->protocols,
            &(req->srmv2_token), &(req->srmv2_pinstatuses), errbuf, errbufsz, req->timeout);*/
    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_bringonline][EPROTONOSUPPORT] Only SRMv2-compliant SEs are supported");
        errno = EPROTONOSUPPORT;
        return (-1);;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, PIN_STATUS));
}

    int
gfal_prestage (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
    	struct srm_bringonline_input bringonline_input;
    	struct srm_bringonline_output bringonline_output;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_pinstatuses)
        {
            free (req->srmv2_pinstatuses);
            req->srmv2_pinstatuses = NULL;
        }
        if (req->srmv2_token)
        {
            free (req->srmv2_token);
            req->srmv2_token = NULL;
        }
        bringonline_input.desiredpintime = req->srmv2_desiredpintime;
        bringonline_input.nbfiles = req->nbfiles;
        bringonline_input.protocols =  req->protocols;
        bringonline_input.spacetokendesc = req->srmv2_spacetokendesc;
        bringonline_input.surls = req->surls;

		ret = srm_bring_online_async(&context,&bringonline_input,&bringonline_output);

		req->srmv2_token = bringonline_output.token;
		req->srmv2_pinstatuses = bringonline_output.filestatuses;

        /*TODO ret = srmv2_prestagee (req->nbfiles, (const char **) req->surls, req->endpoint,
            req->srmv2_desiredpintime, req->srmv2_spacetokendesc, req->protocols,
            &(req->srmv2_token), &(req->srmv2_pinstatuses), errbuf, errbufsz, req->timeout);*/

    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_prestage][EPROTONOSUPPORT] Only SRMv2-compliant SEs are supported");
        errno = EPROTONOSUPPORT;
        return (-1);;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, PIN_STATUS));
}

    int
gfal_prestagestatus (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
    	struct srm_bringonline_input bringonline_input;
    	struct srm_bringonline_output bringonline_output;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_pinstatuses)
        {
            free (req->srmv2_pinstatuses);
            req->srmv2_pinstatuses = NULL;
        }
        bringonline_output.token = req->srmv2_token;

        ret = srm_status_of_bring_online_async(&context,&bringonline_input,&bringonline_output);

        req->srmv2_pinstatuses = bringonline_output.filestatuses;

        /*TODO ret = srmv2_prestagestatuse (req->srmv2_token, req->endpoint, &(req->srmv2_pinstatuses),
                errbuf, errbufsz, req->timeout);*/
    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_prestagestatus][EPROTONOSUPPORT] Only SRMv2-compliant SEs are supported");
        errno = EPROTONOSUPPORT;
        return (-1);;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, PIN_STATUS));
}

    int
gfal_pin (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
    	struct srm_extendfilelifetime_input extendfilelifetime_input;
    	struct srm_extendfilelifetime_output extendfilelifetime_output;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_pinstatuses)
        {
            free (req->srmv2_pinstatuses);
            req->srmv2_pinstatuses = NULL;
        }
        extendfilelifetime_input.nbfiles = req->nbfiles;
        extendfilelifetime_input.surls = req->surls;
        extendfilelifetime_input.reqtoken = req->srmv2_token;
        extendfilelifetime_input.pintime = req->srmv2_desiredpintime;

        ret = srm_extend_file_lifetime(&context,&extendfilelifetime_input,&extendfilelifetime_output);

        req->srmv2_pinstatuses = extendfilelifetime_output.filestatuses;

        /*TODO ret = srmv2_pin (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
                req->srmv2_desiredpintime, &(req->srmv2_pinstatuses),
                errbuf, errbufsz, req->timeout);*/
    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_pin][EPROTONOSUPPORT] Only SRMv2-compliant SEs are supported");
        errno = EPROTONOSUPPORT;
        return (-1);;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, PIN_STATUS));
}

    int
gfal_release (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
        if (req->srmv2_statuses)
        {
            free (req->srmv2_statuses);
            req->srmv2_statuses = NULL;
        }
    	struct srm_context context;
    	struct srm_releasefiles_input releasefiles_input;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

    	releasefiles_input.nbfiles = req->nbfiles;
    	releasefiles_input.surls = req->surls;
    	releasefiles_input.reqtoken = req->srmv2_token;

    	ret = srm_release_files(&context,&releasefiles_input,&(req->srmv2_statuses)); // TODO be as the template context,input,output structures

        /*TODO ret = srmv2_release (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
                &(req->srmv2_statuses), errbuf, errbufsz, req->timeout);*/
    } else if (req->setype == TYPE_SRM) {
        int i;

        if (req->srm_statuses == NULL) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_release][EINVAL] no SRMv1 file ids");
            errno = EINVAL;
            return (-1);
        }
        for (i = 0; i < req->nbfiles; ++i) {
 /*TODO REMOVE ALL IF SRMV1          if (srm_set_xfer_done (req->endpoint, req->srm_reqid, req->srm_statuses[i].fileid,
                        errbuf, errbufsz, req->timeout) < 0) {
                req->srm_statuses[i].surl = strdup (req->surls[i]);
                req->srm_statuses[i].turl = NULL;
                req->srm_statuses[i].status = errno;
            } else {
                req->srm_statuses[i].surl = strdup (req->surls[i]);
                req->srm_statuses[i].turl = NULL;
                req->srm_statuses[i].status = 0;
            }*/
        }
        ret = req->nbfiles;
    } else { // req->setype == TYPE_SE
        int i;

        if (req->sfn_statuses)
            free (req->sfn_statuses);
        if ((req->sfn_statuses = (struct sfn_filestatus *) calloc (req->nbfiles, sizeof (struct sfn_filestatus))) == NULL) {
            errno = ENOMEM;
            return (-1);
        }
        for (i = 0; i < req->nbfiles; ++i) {
            req->sfn_statuses[i].surl = strdup (req->surls[i]);
            req->sfn_statuses[i].turl = NULL;
            req->sfn_statuses[i].status = 0;
        }
        ret = req->nbfiles;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, DEFAULT_STATUS));
}

    int
gfal_set_xfer_done (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_statuses)
        {
            free (req->srmv2_statuses);
            req->srmv2_statuses = NULL;
        }
        if (req->oflag == 0)
        {
        	struct srm_releasefiles_input releasefiles_input;
        	releasefiles_input.nbfiles = req->nbfiles;
        	releasefiles_input.reqtoken = req->srmv2_token;
        	releasefiles_input.surls = req->surls;

        	ret = srm_release_files(&context,&releasefiles_input,&(req->srmv2_statuses));

		//TODO	ret = srmv2_set_xfer_done_get (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
		//			&(req->srmv2_statuses), errbuf, errbufsz, req->timeout);
        }else
        {
        	struct srm_putdone_input putdone_input;
        	putdone_input.nbfiles = req->nbfiles;
        	putdone_input.reqtoken = req->srmv2_token;
        	putdone_input.surls = req->surls;

        	ret = srm_put_done(&context,&putdone_input,&(req->srmv2_statuses));

		//TODO	ret = srmv2_set_xfer_done_put (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
		//			&(req->srmv2_statuses), errbuf, errbufsz, req->timeout);
        }
    } else if (req->setype == TYPE_SRM) {
        int i;

        if (req->srm_statuses == NULL) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_set_xfer_done][EINVAL] no SRMv1 file ids");
            errno = EINVAL;
            return (-1);
        }
        for (i = 0; i < req->nbfiles; ++i) {
/*TODO            if (srm_set_xfer_done (req->endpoint, req->srm_reqid, req->srm_statuses[i].fileid,
                        errbuf, errbufsz, req->timeout) < 0) {
                req->srm_statuses[i].surl = strdup (req->surls[i]);
                req->srm_statuses[i].turl = NULL;
                req->srm_statuses[i].status = errno;
            } else {
                req->srm_statuses[i].surl = strdup (req->surls[i]);
                req->srm_statuses[i].turl = NULL;
                req->srm_statuses[i].status = 0;
            }*/
        }
        ret = req->nbfiles;
    } else { // req->setype == TYPE_SE
        int i;

        if (req->sfn_statuses)
            free (req->sfn_statuses);
        if ((req->sfn_statuses = (struct sfn_filestatus *) calloc (req->nbfiles, sizeof (struct sfn_filestatus))) == NULL) {
            errno = ENOMEM;
            return (-1);
        }
        for (i = 0; i < req->nbfiles; ++i) {
            req->sfn_statuses[i].surl = strdup (req->surls[i]);
            req->sfn_statuses[i].turl = NULL;
            req->sfn_statuses[i].status = 0;
        }
        ret = req->nbfiles;
    }

    errno = 0;
    req->returncode = ret;
    return (copy_gfal_results (req, DEFAULT_STATUS));
}

    int
gfal_set_xfer_running (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
        if (req->srmv2_statuses)
        {
            free (req->srmv2_statuses);
            req->srmv2_statuses = NULL;
        }

//srmv2_set_xfer_running start
    	if ((req->srmv2_statuses = (struct srmv2_filestatus *) calloc (req->nbfiles, sizeof (struct srmv2_filestatus))) == NULL)
    	{
    		errno = ENOMEM;
    		ret = (-1);
    	}else
    	{
    		int i;
			for (i = 0; i < req->nbfiles; ++i)
			{
				memset (req->srmv2_statuses + i, 0, sizeof (struct srmv2_filestatus));
				req->srmv2_statuses[i].surl = strdup (req->surls[i]);
				req->srmv2_statuses[i].status = 0;
			}
			ret = req->nbfiles;
    	}
//srmv2_set_xfer_running end

   /*TODO     ret = srmv2_set_xfer_running (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
                &(req->srmv2_statuses), errbuf, errbufsz, req->timeout); */
    } else if (req->setype == TYPE_SRM) {
        int i;

        if (req->srm_statuses == NULL) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_release][EINVAL] no SRMv1 file ids");
            errno = EINVAL;
            return (-1);
        }
        for (i = 0; i < req->nbfiles; ++i) {
/*TODO            if (srm_set_xfer_running (req->endpoint, req->srm_reqid, req->srm_statuses[i].fileid,
                        errbuf, errbufsz, req->timeout) < 0) {
                req->srm_statuses[i].surl = strdup (req->surls[i]);
                req->srm_statuses[i].turl = NULL;
                req->srm_statuses[i].status = errno;
            } else {
                req->srm_statuses[i].surl = strdup (req->surls[i]);
                req->srm_statuses[i].turl = NULL;
                req->srm_statuses[i].status = 0;
            }*/
        }
        ret = req->nbfiles;
    } else { // req->setype == TYPE_SE
        int i;

        if (req->sfn_statuses)
            free (req->sfn_statuses);
        if ((req->sfn_statuses = (struct sfn_filestatus *) calloc (req->nbfiles, sizeof (struct sfn_filestatus))) == NULL) {
            errno = ENOMEM;
            return (-1);
        }
        for (i = 0; i < req->nbfiles; ++i) {
            req->sfn_statuses[i].surl = strdup (req->surls[i]);
            req->sfn_statuses[i].turl = NULL;
            req->sfn_statuses[i].status = 0;
        }
        ret = req->nbfiles;
    }

    errno = 0;
    req->returncode = ret;
    return (copy_gfal_results (req, DEFAULT_STATUS));
}

    int
gfal_abortrequest (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 1, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

        if (req->srmv2_statuses)
        {
            free (req->srmv2_statuses);
            req->srmv2_statuses = NULL;
        }
        if (req->srmv2_pinstatuses)
        {
            free (req->srmv2_pinstatuses);
            req->srmv2_pinstatuses = NULL;
        }
        if (req->srmv2_mdstatuses)
        {
            free (req->srmv2_mdstatuses);
            req->srmv2_mdstatuses = NULL;
        }
        if (req->results)
        {
            free_gfal_results (req->results, req->results_size);
            req->results = NULL;
            req->results_size = -1;
        }
        ret = srm_abort_request(&context,req->srmv2_token);

     //TODO   ret = srmv2_abortrequest (req->endpoint, req->srmv2_token, errbuf, errbufsz, req->timeout);
    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_abortrequest][EPROTONOSUPPORT] Only SRMv2-compliant SEs are supported");
        errno = EPROTONOSUPPORT;
        return (-1);
    }

    req->returncode = ret;
    return (ret);
}

    int
gfal_abortfiles (gfal_internal req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_internal (req, 0, errbuf, errbufsz) < 0)
        return (-1);

    if (req->setype == TYPE_SRMv2)
    {
    	struct srm_context context;
    	struct srm_abort_files_input abortfiles_input;

    	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_verbose);

    	if (req->srmv2_statuses)
        {
            free (req->srmv2_statuses);
            req->srmv2_statuses = NULL;
        }
    	abortfiles_input.nbfiles = req->nbfiles;
    	abortfiles_input.surls = req->surls;
    	abortfiles_input.reqtoken = req->srmv2_token;

    	ret = srm_abort_files(&context,&abortfiles_input,&(req->srmv2_statuses));

        /*TODO ret = srmv2_abortfiles (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
                &(req->srmv2_statuses), errbuf, errbufsz, req->timeout);*/
    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_abortfiles][EPROTONOSUPPORT] Only SRMv2-compliant SEs are supported");
        errno = EPROTONOSUPPORT;
        return (-1);;
    }

    req->returncode = ret;
    return (copy_gfal_results (req, DEFAULT_STATUS));
}

    static int
mdtomd32 (struct stat64 *statb64, struct stat *statbuf)
{
    if (statb64->st_size > OFF_MAX && sizeof(off_t) == 4) {
#if defined(_WIN32)
        errno = EINVAL
#else
            errno = EOVERFLOW;
#endif
        return (-1);
    }
    memset (statbuf, 0, sizeof(struct stat));
    statbuf->st_mode = statb64->st_mode;
    statbuf->st_nlink = statb64->st_nlink;
    statbuf->st_uid = statb64->st_uid;
    statbuf->st_gid = statb64->st_gid;
    statbuf->st_size = (off_t) statb64->st_size;
    return (0);
}
#define ENDPOINT_DEFAULT_PREFIX "httpg://"
#define ENDPOINT_DEFAULT_PREFIX_LEN strlen("httpg://")

    static char *
endpointfromsurl (const char *surl, char *errbuf, int errbufsz, int _prefixing_on)
{
    int len;
    char *p, *endpoint = NULL;
    int endpoint_offset=0;

    if (strncmp (surl, "srm://", 6) && strncmp (surl, "sfn://", 6)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][endpointfromsurl][EINVAL] %s: Invalid SURL (must start with either 'srm://' or 'sfn://')", surl);
        errno = EINVAL;
        return (NULL);
    }

    p = strstr (surl + 6, "?SFN=");
    if (p == NULL) {
        p = strchr (surl + 6, '/');
        if (p == NULL) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][endpointfromsurl][EINVAL] %s: Invalid SURL", surl);
            errno = EINVAL;
            return (NULL);
        }
    }

    len = p - surl - 6;
    if ((endpoint = (char *) calloc (len + 1 + strlen (ENDPOINT_DEFAULT_PREFIX), sizeof (char*))) == NULL) {
        errno = ENOMEM;
        return (NULL);
    }//hack to ensure proper endpoint prefixing (httpg://)

    if(_prefixing_on && len > 0) {
        strcpy (endpoint, ENDPOINT_DEFAULT_PREFIX);
        endpoint_offset = ENDPOINT_DEFAULT_PREFIX_LEN;
    }

    strncpy (endpoint + endpoint_offset, surl + 6, len);
    endpoint[len + endpoint_offset] = 0;
    return (endpoint);
}

int
canonical_url (const char *url, const char *defproto, char *newurl, int newurlsz, char *errbuf, int errbufsz) {
    char *pwd, *lfc_home, *p_url, *p_newurl;
    int len;
    char *cat_type;
    int islfc = 0;

    if (url == NULL || newurl == NULL || newurlsz < 10) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][canonical_url][EFAULT] bad arguments");
        errno = EFAULT;
        return (-1);
    }
    if (strlen (url) > newurlsz - 1) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][canonical_url][ENAMETOOLONG] %s: URL too long", url);
        errno = ENAMETOOLONG;
        return (-1);
    }

    if(get_cat_type (&cat_type) < 0)
        return (-1);
    islfc = strcmp (cat_type, "lfc") == 0;
    free (cat_type);

    pwd = lfc_home = NULL;

    /* get protocol */
    if ((p_url = strstr (url, ":")) == NULL) {
        /* use 'defproto' as default URL protocol if defined */
        if (defproto == NULL) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][canonical_url][EINVAL] %s: No protocol specified", url);
            errno = EINVAL;
            return (-1);
        }
        if (strlen (url) + strlen (defproto) + 1 > newurlsz - 1) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][canonical_url][ENAMETOOLONG] %s: URL too long", url);
            errno = ENAMETOOLONG;
            return (-1);
        }
        sprintf (newurl, "%s:", defproto);
        len = strlen (defproto) + 1;
        p_newurl = newurl + len;
        p_url = (char *) url;
    } else {
        /* include ':' */
        len = (p_url - url) + 1;

        /* include '\0' at the end */
        snprintf (newurl, len + 1, "%s", url);

        p_newurl = newurl + len;
        ++p_url;
    }

    if (strcmp (newurl, "lfn:") == 0) {
        if (*p_url != '/') {
            if (islfc && lfc_home == NULL && (lfc_home = getenv ("LFC_HOME")) == NULL) {
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                        "[GFAL][canonical_url][EINVAL] %s: Relative path, but LFC_HOME not defined", url);
                errno = EINVAL;
                return (-1);
            }

            snprintf (p_newurl, newurlsz - len, "%s/%s", lfc_home, p_url);
        } else {
            while (*(p_url + 1) == '/') ++p_url;
            snprintf (p_newurl, newurlsz - len, "%s", p_url);
        }
    } else if (strcmp (newurl, "file:") == 0) {
        if (*p_url != '/') {
            if (pwd == NULL && (pwd = getenv ("PWD")) == NULL) {
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                        "[GFAL][canonical_url][EINVAL] %s: Can't determine current directory", url);
                errno = EINVAL;
                return (-1);
            }

            snprintf (p_newurl, newurlsz - len, "%s/%s", pwd, p_url);
        } else {
            while (*(p_url + 1) == '/') ++p_url;
            snprintf (p_newurl, newurlsz - len, "%s", p_url);
        }
    } else {
        snprintf (p_newurl, newurlsz - len, "%s", p_url);
    }

    return (0);
}

    int
parseturl (const char *turl, char *protocol, int protocolsz, char *pfn, int pfnsz, char* errbuf, int errbufsz)
{
    int len;
    char *p;

    if (!turl) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][parseturl][EFAULT]: Invalid TURL (NULL)");
        errno = EFAULT;
        return (-1);
    }

    if (turl && strlen (turl) > pfnsz - 1) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][parseturl][ENAMETOOLONG] %s: TURL too long", turl);
        errno = ENAMETOOLONG;
        return (-1);
    }
    strcpy (pfn, turl);

    /* get protocol */
    if ((p = strstr (pfn, ":/")) == NULL) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][parseturl][EINVAL] %s: Invalid TURL", turl);
        errno = EINVAL;
        return (-1);
    } else if ((len = p - pfn) > (protocolsz - 1)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][parseturl][ENAMETOOLONG] %s: TURL protocol too long", turl);
        errno = ENAMETOOLONG;
        return (-1);
    } else {
        strncpy (protocol, pfn, len);
        *(protocol + len) = '\0';
    }

    if (strcmp (protocol, "file") == 0) {
        ++p;
        memmove (pfn, p, strlen (p) + 1);
    } else if (strcmp (protocol, "rfio") == 0) {
        p += 2;
        if (*p != '/' || (*(p + 1) == '/' && *(p + 2) != '/')) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][parseturl][EINVAL] %s: Invalid TURL", turl);
            errno = EINVAL;
            return (-1);
        }
        p++;
        if (*p == '/') {	// no hostname ; *(p + 1) = '/' due to the previous test
            memmove (pfn, p + 1, strlen (p + 1) + 1);
        } else if (strchr (p, '?') == NULL && strchr (p, ':') == NULL) {
            // For Castor2-like RFIO TURL (eg. with "?svcClass=..."), pfn is the TURL, nothing to do
            // For other case, we want hostname:/filename
            memmove (pfn, p, strlen (p) + 1);
            if ((p = strchr (pfn, '/')) == NULL) {
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][parseturl][EINVAL] %s: Invalid TURL", turl);
                errno = EINVAL;
                return (-1);
            }
            // p is pointing on the slash just after the hostname
            if (*(p + 1) != '/')
                memmove (p + 1, p, strlen (p) + 1);

            *p = ':';
        }
        // For other cases (Castor2-like RFIO TURL), the entire turl is returned as pfn
    }

    return (0);
}

    int
setypesandendpoints (const char *endpoint, char ***se_types, char ***se_endpoints, char *errbuf, int errbufsz)
{
    int len;
    char *p1, *p2;
    char endpoint_tmp[256];

    if (se_types == NULL) {
        errno = EINVAL;
        return (-1);
    }
    if (strlen (endpoint) + 2 >= sizeof (endpoint_tmp)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][setypesandendpoints][ENAMETOOLONG] %s: Endpoint too long", endpoint);
        errno = ENAMETOOLONG;
        return (-1);
    }

    if ((p1 = strchr (endpoint, '/')) == NULL || (p2 = strchr (endpoint, ':')) != NULL) {
        strncpy (endpoint_tmp, endpoint, sizeof (endpoint_tmp));
    } else {
        len = p1 - endpoint;
        strncpy (endpoint_tmp, endpoint, len);
        strncpy (endpoint_tmp + len, ":*", 2);
        len += 2;
        strncpy (endpoint_tmp + len, p1, sizeof (endpoint_tmp) - len);
        endpoint_tmp[sizeof (endpoint_tmp) - 1] = 0;
    }

    return (get_se_types_and_endpoints (endpoint_tmp, se_types, se_endpoints, errbuf, errbufsz));
}

    int
setypesandendpointsfromsurl (const char *surl, char ***se_types, char ***se_endpoints, char *errbuf, int errbufsz)
{
    int rc;
    char *endpoint_tmp;

    if ((endpoint_tmp = endpointfromsurl (surl, errbuf, errbufsz, 0)) == NULL)
        return (-1);

    rc = setypesandendpoints (endpoint_tmp, se_types, se_endpoints, errbuf, errbufsz);

    free (endpoint_tmp);
    return (rc);
}

int
get_cat_type (char **cat_type) {
    char *cat_env;
    char *default_cat = GFAL_DEFAULT_CATALOG_TYPE;

    if((cat_env = getenv ("LCG_CATALOG_TYPE")) == NULL) {
        /* default catalogs if no environment variable specified */
        cat_env = default_cat;
    }
    if((*cat_type = strdup(cat_env)) == NULL) {
        return (-1);
    }
    return 0;
}

    char *
get_catalog_endpoint (char *errbuf, int errbufsz)
{
    char *cat_type;
    if (get_cat_type (&cat_type) < 0) {
        return (NULL);
    }
    if (strcmp (cat_type, "edg") == 0) {
        free (cat_type);
        return (lrc_get_catalog_endpoint (errbuf, errbufsz));
    } else if (strcmp (cat_type, "lfc") == 0) {
        free (cat_type);
        return (lfc_get_catalog_endpoint (errbuf, errbufsz));
    } else {
        free (cat_type);
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_catalog_endpoint][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        errno = EINVAL;
        return (NULL);
    }
}

    char *
gfal_guidforpfn (const char *pfn, char *errbuf, int errbufsz)
{
    char *cat_type;
    char actual_pfn[GFAL_PATH_MAXLEN];

    if (purify_surl (pfn, actual_pfn, GFAL_PATH_MAXLEN) < 0)
        return (NULL);
    if (get_cat_type (&cat_type) < 0) {
        return (NULL);
    }
    if (strcmp (cat_type, "edg") == 0) {
        free (cat_type);
        return (lrc_guidforpfn (actual_pfn, errbuf, errbufsz));
    } else if (strcmp (cat_type, "lfc") == 0) {
        free (cat_type);
        return (lfc_guidforpfn (actual_pfn, errbuf, errbufsz));
    } else {
        free (cat_type);
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_guidforpfn][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        errno = EINVAL;
        return (NULL);
    }
}

    int
gfal_guidsforpfns (int nbfiles, const char **pfns, int amode, char ***guids, int **statuses, char *errbuf, int errbufsz)
{
    char *cat_type;
    char actual_pfn[GFAL_PATH_MAXLEN];

    if (get_cat_type (&cat_type) < 0)
        return (-1);

    if (strcmp (cat_type, "edg") == 0) {
        int i;
        char errmsg[GFAL_ERRMSG_LEN];

        free (cat_type);

        if ((*guids = (char **) calloc (nbfiles + 1, sizeof (char *))) == NULL ||
                (*statuses = (int *) calloc (nbfiles, sizeof (int))) == NULL) {
            return (-1);
        }

        for (i = 0; i < nbfiles; ++i) {
            if (purify_surl (pfns[i], actual_pfn, GFAL_PATH_MAXLEN) < 0) {
                (*statuses)[i] = errno;
                continue;
            }
            (*guids)[i] = lrc_guidforpfn (pfns[i], errmsg, GFAL_ERRMSG_LEN);
            (*statuses)[i] = errno;
        }
        errno = 0;
        return (0);
    } else if (strcmp (cat_type, "lfc") == 0) {
        free (cat_type);
        return (lfc_guidsforpfns (nbfiles, pfns, amode, guids, statuses, errbuf, errbufsz));
    } else {
        free (cat_type);
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_guidsforpfns][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        errno = EINVAL;
        return (-1);
    }
}

    int
guid_exists (const char *guid, char *errbuf, int errbufsz)
{
    char *cat_type;
    if (get_cat_type (&cat_type) < 0) {
        return (-1);
    }
    if (strcmp (cat_type, "edg") == 0) {
        free (cat_type);
        return (lrc_guid_exists (guid, errbuf, errbufsz));
    } else if (strcmp (cat_type, "lfc") == 0) {
        free (cat_type);
        return (lfc_guid_exists (guid, errbuf, errbufsz));
    } else {
        free (cat_type);
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][guid_exists][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        errno = EINVAL;
        return (-1);
    }
}

    int
setfilesize (const char *pfn, GFAL_LONG64 filesize, char *errbuf, int errbufsz)
{
    char *cat_type;
    if (get_cat_type (&cat_type) < 0) {
        return (-1);
    }
    if (strcmp (cat_type, "edg") == 0) {
        free (cat_type);
        return (lrc_setfilesize (pfn, filesize, errbuf, errbufsz));
    } else if (strcmp (cat_type, "lfc") == 0) {
        // in this case, we suppose pfn is actually a LFN
        free (cat_type);
        return (lfc_setsize (pfn, filesize, errbuf, errbufsz));
    } else {
        free (cat_type);
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][setfilesize][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        errno = EINVAL;
        return (-1);
    }
}

    char **
gfal_get_replicas (const char *lfn, const char *guid, char *errbuf, int errbufsz)
{
    int sav_errno = 0;
    char *cat_type = NULL, *actual_lfn = NULL, *actual_guid = NULL;
    char **results = NULL;

    if (lfn == NULL && guid == NULL) {
        errno = EINVAL;
        return (NULL);
    }

    if (lfn != NULL)
        actual_lfn = strdup (strncmp (lfn, "lfn:", 4) == 0 ? lfn + 4 : lfn);
    if (guid != NULL)
        actual_guid = strdup (strncmp (guid, "guid:", 5) == 0 ? guid + 5 : guid);

    if (get_cat_type (&cat_type) < 0)
        return (NULL);

    if (strcmp (cat_type, "lfc") == 0) {
        results = lfc_get_replicas (actual_lfn, actual_guid, errbuf, errbufsz);
        sav_errno = errno;
    } else if (strcmp (cat_type, "edg") == 0) {
        if (actual_guid == NULL) {
            if ((actual_guid = rmc_guidfromlfn (actual_lfn, errbuf, errbufsz)) == NULL)
                sav_errno = errno;
        } else {
            results = lrc_surlsfromguid (actual_guid, errbuf, errbufsz);
            sav_errno = errno;
        }
    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_get_replicas][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        sav_errno = EINVAL;
    }

    if (cat_type) free (cat_type);
    if (actual_lfn) free (actual_lfn);
    if (actual_guid) free (actual_guid);
    errno = sav_errno;
    return (results);
}

    int
gfal_unregister_pfns (int nbguids, const char **guids, const char **pfns, int **results, char *errbuf, int errbufsz)
{
    char *cat_type;
    int rc = 0;

    if (get_cat_type (&cat_type) < 0)
        return (-1);

    if (strcmp (cat_type, "edg") == 0) {
        int i, j;
        char **remaining_pfns, **lfns;
        char surl_cat[GFAL_PATH_MAXLEN];

        free (cat_type);

        if ((*results = (int *) calloc (nbguids, sizeof (int))) == NULL)
            return (-1);

        for (i = 0; i < nbguids; ++i) {
            if (purify_surl (pfns[i], surl_cat, GFAL_PATH_MAXLEN) < 0) {
                (*results)[i] = EINVAL;
                continue;
            }
            rc = lrc_unregister_pfn (guids[i], surl_cat, errbuf, errbufsz);
            if (rc < 0 && errno == ENOENT) {
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                        "[GFAL][gfal_unregister_pfns][] Failed with removing SRM version specific information, trying with full SURL...\n");
                rc = lrc_unregister_pfn (guids[i], pfns[i], errbuf, errbufsz);
            }
            if (rc < 0) {
                (*results)[i] = errno;
            } else {
                (*results)[i] = 0;
                gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[guid:%s] %s UNREGISTERED", guids[i], pfns[i]);
            }

            remaining_pfns = lrc_surlsfromguid (guids[i], errbuf, errbufsz);
            if (remaining_pfns != NULL) {
                /* still valid replicas */
                for (j = 0; remaining_pfns[j]; ++j) free (remaining_pfns[j]);
                free(remaining_pfns);
                continue;
            }

            /* There are no more replicas */
            lfns = rmc_lfnsforguid (guids[i], errbuf, errbufsz);
            if (lfns != NULL) {
                /* We remove all aliases */
                for (j = 0; lfns[j]; ++j) {
                    gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "[guid:%s] %s UNREGISTERED", guids[i], lfns[j]);
                    rmc_unregister_alias (guids[i], lfns[j], errbuf, errbufsz);
                    free (lfns[j]);
                }
                free (lfns);
            }
        }
        return (0);
    } else if (strcmp (cat_type, "lfc") == 0) {
        free (cat_type);
        return (lfc_unregister_pfns (nbguids, guids, pfns, results, errbuf, errbufsz));
    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_unregister_pfns][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        free (cat_type);
        errno = EINVAL;
        return (-1);
    }
}

    char *
guidfromlfn (const char *lfn, char *errbuf, int errbufsz)
{
    char *cat_type;
    if (get_cat_type (&cat_type) < 0) {
        return (NULL);
    }
    if (strcmp (cat_type, "edg") == 0) {
        free (cat_type);
        return (rmc_guidfromlfn (lfn, errbuf, errbufsz));
    } else if (strcmp (cat_type, "lfc") == 0) {
        free (cat_type);
        return (lfc_guidfromlfn (lfn, errbuf, errbufsz));
    } else {
        free (cat_type);
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][guidfromlfn][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        errno = EINVAL;
        return (NULL);
    }
}

    char **
gfal_get_aliases (const char *lfn, const char *guid, char *errbuf, int errbufsz)
{
    int sav_errno = 0;
    char *cat_type = NULL, *actual_lfn = NULL, *actual_guid = NULL;
    char **results = NULL;

    if (lfn == NULL && guid == NULL) {
        errno = EINVAL;
        return (NULL);
    }

    if (lfn != NULL)
        actual_lfn = strdup (strncmp (lfn, "lfn:", 4) == 0 ? lfn + 4 : lfn);
    if (guid != NULL)
        actual_guid = strdup (strncmp (guid, "guid:", 5) == 0 ? guid + 5 : guid);

    if (get_cat_type (&cat_type) < 0) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_get_aliases][] Unable to determine the catalog type");
        return (NULL);
    }
    if (strcmp (cat_type, "lfc") == 0) {
        results = lfc_get_aliases (actual_lfn, actual_guid, errbuf, errbufsz);
        sav_errno = errno;
    } else if (strcmp (cat_type, "edg") == 0) {
        if (actual_guid == NULL) {
            if ((actual_guid = rmc_guidfromlfn (actual_lfn, errbuf, errbufsz)) == NULL)
                sav_errno = errno;
        } else {
            results = rmc_lfnsforguid (actual_guid, errbuf, errbufsz);
            sav_errno = errno;
        }
    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_get_aliases][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        sav_errno = EINVAL;
    }

    if (cat_type) free (cat_type);
    if (actual_lfn) free (actual_lfn);
    if (actual_guid) free (actual_guid);
    errno = sav_errno;
    return (results);
}

    int
gfal_register_file (const char *lfn, const char *guid, const char *surl, mode_t mode,
        GFAL_LONG64 size, int bool_createonly, char *errbuf, int errbufsz)
{
    int sav_errno = 0;
    char *cat_type;
    char actual_surl[GFAL_PATH_MAXLEN];
    int islfc, isedg, rc = 0;

    if (surl == NULL) {
        errno = EINVAL;
        return (-1);
    }

    if (get_cat_type (&cat_type) < 0) {
        return (-1);
    }
    islfc = strcmp (cat_type, "lfc") == 0;
    isedg = strcmp (cat_type, "edg") == 0;
    free (cat_type);

    if (lfn && !lfn[0]) lfn = NULL;
    if (guid && !guid[0]) guid = NULL;

    if (guid) {
        uuid_t uuid;

        if (strncmp (guid, "guid:", 5) == 0)
            guid += 5;
        if (uuid_parse (guid, uuid) < 0) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_register_file][EINVAL] guid:%s: Invalid GUID", guid);
            errno = EINVAL; /* invalid guid */
            return -1;
        }
    }
    if (lfn && strncmp (lfn, "lfn:", 4) == 0)
        lfn += 4;
    if (purify_surl (surl, actual_surl, GFAL_PATH_MAXLEN) < 0)
        return (-1);
    if (mode == 0)
        mode = 0664;

    if (islfc) {
        rc = lfc_register_file (lfn, guid, actual_surl, mode, size, bool_createonly, errbuf, errbufsz);
        sav_errno = errno;
    } else if (isedg) {
        char *actual_lfn = NULL, *actual_guid = NULL;
        char *generated_lfn = NULL, *generated_guid = NULL;

        if (lfn) {
            actual_guid = rmc_guidfromlfn (lfn, errbuf, errbufsz);
            if (guid == NULL && actual_guid == NULL &&
                    (guid = generated_guid = gfal_generate_guid (errbuf, errbufsz)) == NULL)
                return (-1);
            if (actual_guid != NULL && (bool_createonly ||
                        (guid != NULL && strncmp (guid, actual_guid, GFAL_GUID_LEN) !=0))) {
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                        "[GFAL][gfal_register_file][EEXIST] lfn:%s: file already exists", lfn);
                free (actual_guid);
                errno = EEXIST;
                return (-1);
            }
            if (guid == NULL)   // guid == NULL and actual_guid != NULL
                guid = actual_guid;
            // case guid != NULL and actual_guid == NULL below
        }
        if (guid && actual_guid == NULL && generated_guid == NULL) {
            char **lfns = NULL;

            lfns = rmc_lfnsforguid (guid, errbuf, errbufsz);
            if (lfn == NULL && lfns == NULL &&
                    (lfn = generated_lfn = gfal_generate_lfn (errbuf, errbufsz)) == NULL)
                return (-1);
            if (lfns != NULL) {
                int i, bool_ok = 0;

                actual_lfn = lfns[0];
                for (i = 0; lfns[i]; ++i) {
                    bool_ok = lfn && strcmp (lfn, lfns[i]) == 0;
                    free (lfns[i]);
                }
                free (lfns);

                if (bool_createonly || (lfn && !bool_ok)) {
                    gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                            "[GFAL][gfal_register_file][EEXIST] guid:%s: file already exists", guid);
                    free (actual_guid);
                    errno = EEXIST;
                    return (-1);
                }
            }
        }

        rc = 0;
        if (!actual_guid && !actual_lfn && lfn) {
            gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "Registering LFN: lfn:%s (guid:%s)", lfn, guid);
            if ((rc = rmc_register_alias (guid, lfn, errbuf, errbufsz)) < 0)
                sav_errno = errno;
        }

        if (generated_lfn) free (generated_lfn);

        if (rc == 0) {
            gfal_errmsg (NULL, 0, GFAL_ERRLEVEL_WARN, "Registering SURL: %s (guid:%s)", actual_surl, guid);
            rc = lrc_register_pfn (guid, actual_surl, errbuf, errbufsz);

            if (rc == 0 && size > 0)
                rc = lrc_setfilesize (actual_surl, size, errbuf, errbufsz);
        }

        if (actual_guid) free (actual_guid);
        if (generated_guid) free (generated_guid);
    } else {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_register_file][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        sav_errno = EINVAL;
        rc = -1;
    }

    errno = sav_errno;
    return (rc);
}

    int
register_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
    char *cat_type;
    if (get_cat_type (&cat_type) < 0) {
        return (-1);
    }
    if (strcmp (cat_type, "edg") == 0) {
        free (cat_type);
        return (rmc_register_alias (guid, lfn, errbuf, errbufsz));
    } else if (strcmp (cat_type, "lfc") == 0) {
        free (cat_type);
        return (lfc_register_alias (guid, lfn, errbuf, errbufsz));
    } else {
        free (cat_type);
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][register_alias][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        errno = EINVAL;
        return (-1);
    }
}

    int
unregister_alias (const char *guid, const char *lfn, char *errbuf, int errbufsz)
{
    char *cat_type;
    if (get_cat_type (&cat_type) < 0) {
        return (-1);
    }
    if (strcmp (cat_type, "edg") == 0) {
        free (cat_type);
        return (rmc_unregister_alias (guid, lfn, errbuf, errbufsz));
    } else if (strcmp (cat_type, "lfc") == 0) {
        free (cat_type);
        return (lfc_unregister_alias (guid, lfn, errbuf, errbufsz));
    } else {
        free (cat_type);
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][unregister_alias][EINVAL] The catalog type is neither 'edg' nor 'lfc'.");
        errno = EINVAL;
        return (-1);
    }
}

    int
getdomainnm (char *name, int namelen)
{
    FILE *fd;
    char line[300];
    char *p, *q;

    if ((fd = fopen ("/etc/resolv.conf", "r")) != NULL) {
        while (fgets (line, sizeof(line), fd) != NULL) {
            if (strncmp (line, "domain", 6) == 0 ||
                    strncmp (line, "search", 6) == 0) {
                p = line + 6;
                while (*p == ' ' || *p == '\t')
                    p++;
                if (*p == '\0' || *p == '\n')
                    continue;
                fclose (fd);
                q = p + strlen (p) - 1;
                if (*q == '\n')
                    *q = '\0';
                q = p;
                while (*q != '\0' && *q != ' ' && *q != '\t')
                    q++;
                if (*q)
                    *q = '\0';
                if (strlen (p) > namelen) {
                    return (-1);
                }
                strcpy (name, p);
                return (0);
            }
        }
        fclose (fd);
    }
    return (-1);
}


    char *
get_default_se(char *errbuf, int errbufsz)
{
    char *vo;
    char *default_se;
    int i;
    char se_env[15 + GFAL_VO_MAXLEN];

    if((vo = gfal_get_vo (errbuf, errbufsz)) == NULL) {
        errno = EINVAL;
        return (NULL);
    }
    if(strlen(vo) >= GFAL_VO_MAXLEN) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][get_default_se][EINVAL] %s: VO name too long", vo);
        errno = EINVAL;
        return (NULL);
    }
    snprintf(se_env, 15 + GFAL_VO_MAXLEN, "VO_%s_DEFAULT_SE", vo);
    for(i = 3; i < 3 + strlen(vo); ++i) {
        if (se_env[i] == '.' || se_env[i] == '-')
            se_env[i] = '_';
        else
            se_env[i] = toupper(se_env[i]);
    }
    default_se = getenv(se_env);
    errno = 0;
    return default_se;
}

int
purify_surl (const char *surl, char *surl_cat, const int surl_cat_sz) {
    char *p,*q,*r;
    char tmp[1104];

    if (surl == NULL || surl_cat == NULL || surl_cat_sz < strlen (surl)) {
        errno = EINVAL;
        return (-1);
    }

    if (gfal_is_purifydisabled () || strncmp (surl, "srm://", 6)) {
        /* Only SRM SURL need to be purify */
        strncpy (surl_cat, surl, surl_cat_sz);
        return (0);
    }

    strncpy (tmp, surl, 1104);
    p = index (tmp+6, ':'); /* is port number specified ? */
    if ((q = index (tmp+6, '/')) == NULL) {
        errno = EINVAL;
        return (-1);
    }

    if ((r = strstr (tmp+6, "?SFN=")) == NULL) {
        /* no full SURL */
        if (p == NULL)
            /* no full SURL and no port number : nothing to remove */
            strncpy (surl_cat, tmp, surl_cat_sz);
        else {
            /* port number must be removed */
            *p = 0;
            snprintf (surl_cat, surl_cat_sz, "%s%s", tmp, q);
        }
    } else {
        /* full SURL */
        if (p == NULL) {
            /* no port number */
            if (q < r)
                *q = 0;
            else
                *r = 0;
        } else
            *p = 0;
        snprintf (surl_cat, surl_cat_sz, "%s%s", tmp, r+5);
    }

    return (0);
}

    static int
generate_surls (gfal_internal gfal, char *errbuf, int errbufsz)
{
    int i;
    uuid_t uuid;
    char guid[GFAL_GUID_LEN];
    char *sa_path, *sa_root;
    char *vo, *ce_ap, *p, *q;
    char dir_path[1104];
    char simple_ep[256];

    if ((gfal->surls = (char **) calloc (gfal->nbfiles, sizeof (char *))) == NULL) {
        errno = ENOMEM;
        return (-1);
    }

    if ((vo = gfal_get_vo (errbuf, errbufsz)) == NULL) {
        errno = EINVAL;
        return (-1);
    }

    /* now create dir path which is either sa_path, sa_root or combination of ce_ap & sa_root
     * First we need the hostname (only) of the endpoint */
    p = strchr (gfal->endpoint, ':');
    if (p) {
        /* At that point we need to determine if it protocol or port number... */
        if (*(p + 1) == '/') {
            /* endpoint begins with eg. http://..., so it must be removed */
            q = strchr (p + 3, ':');
            if (q) {
                /* a port number is specified, but not wanted here */
                strncpy (simple_ep, p + 3, q - p - 3);
                simple_ep[q - p - 3] = 0;
            } else {
                /* no port number, only protocol must be removed */
                strncpy (simple_ep, p + 3, sizeof(simple_ep));
            }
        } else {
            /* no protocol, only port number must be removed */
            strncpy (simple_ep, gfal->endpoint, p - gfal->endpoint);
            simple_ep[p - gfal->endpoint] = 0;
        }
    } else {
        /* nothing must be removed (neither protocol, nor port number) */
        strncpy (simple_ep, gfal->endpoint, sizeof(simple_ep));
    }
    if (get_storage_path (simple_ep, gfal->srmv2_spacetokendesc, &sa_path, &sa_root, errbuf, errbufsz) < 0)
        return (-1);
    if (sa_path != NULL) {
        if (gfal->setype == TYPE_SE) {
            snprintf (dir_path, 1103, "sfn://%s%s%s/", simple_ep, *sa_path=='/'?"":"/", sa_path);
        } else {
            snprintf (dir_path, 1103, "srm://%s%s%s/", simple_ep, *sa_path=='/'?"":"/", sa_path);
        }
    } else {  /* sa_root != NULL */
        if (gfal->setype == TYPE_SE) {
            if (get_ce_ap (simple_ep, &ce_ap, errbuf, errbufsz) < 0)
                return (-1);
            snprintf (dir_path, 1103, "sfn://%s%s%s%s%s/", simple_ep,
                    *ce_ap=='/'?"":"/", ce_ap,
                    *sa_root=='/'?"":"/", sa_root);
            free (ce_ap);
        } else {
            snprintf (dir_path, 1103, "srm://%s%s%s/", simple_ep, *sa_root=='/'?"":"/", sa_root);
        }
    }
    if (sa_path) free (sa_path);
    if (sa_root) free (sa_root);

    if (gfal->relative_path && gfal->nbfiles == 1) {
        asprintf (gfal->surls, "%s%s", dir_path, gfal->relative_path);
    } else if (gfal->relative_path == NULL) {
        time_t current_time;
        struct tm *tm;
        char timestr[11];
        char tmp[25];

        time (&current_time);
        tm = localtime (&current_time);
        strftime (timestr, 11, "%F", tm);
        snprintf (tmp, 24, "generated/%s", timestr);
        strncat (dir_path, tmp, 1103);

        for (i = 0; i < gfal->nbfiles; ++i) {
            uuid_generate (uuid);
            uuid_unparse (uuid, guid);
            asprintf (gfal->surls + i, "%s/file%s", dir_path, guid);
        }
    } else { /* gfal->relative_path && gfal->nbfiles > 1 */
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][generate_surls][EINVAL] 'relative_path' is not compatible with multiple files");
        errno = EINVAL;
        return (-1);
    }

    return (0);
}

    gfal_request
gfal_request_new ()
{
    gfal_request req;

    if ((req = (gfal_request) malloc (sizeof (struct gfal_request_))) == NULL) {
        errno = ENOMEM;
        return (NULL);
    }

    memset (req, 0, sizeof (struct gfal_request_));
    return (req);
}

    int
gfal_init (gfal_request req, gfal_internal *gfal, char *errbuf, int errbufsz)
{
    int i = 0;
    char **se_endpoints;
    char **se_types;
    char *srmv1_endpoint = NULL;
    char *srmv2_endpoint = NULL;
    int isclassicse = 0;
    int endpoint_offset=0;

    if (req == NULL || req->nbfiles < 0 || (req->endpoint == NULL && req->surls == NULL)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] Invalid request: Endpoint or SURLs must be specified");
        errno = EINVAL;
        return (-1);
    }
    if (req->oflag != 0 && req->filesizes == NULL) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] Invalid request: File sizes must be specified for put requests");
        errno = EINVAL;
        return (-1);
    }
    if (req->srmv2_lslevels > 1) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] Invalid request: srmv2_lslevels must be 0 or 1");
        errno = EINVAL;
        return (-1);
    }

    if ((*gfal = (gfal_internal) malloc (sizeof (struct gfal_internal_))) == NULL) {
        errno = ENOMEM;
        return (-1);
    }

    memset (*gfal, 0, sizeof (struct gfal_internal_));
    memcpy (*gfal, req, sizeof (struct gfal_request_));
    /* Use default SRM timeout if not specified in request */
    if (!(*gfal)->timeout)
        (*gfal)->timeout = gfal_get_timeout_srm ();

    if ((*gfal)->no_bdii_check) {
        if ((*gfal)->surls != NULL && ((*gfal)->setype != TYPE_NONE ||
                    ((*gfal)->setype = (*gfal)->defaultsetype) != TYPE_NONE)) {
            if ((*gfal)->setype == TYPE_SE) {
                gfal_internal_free (*gfal);
                *gfal = NULL;
                gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                        "[GFAL][gfal_init][EINVAL] Invalid request: Disabling BDII checks is not compatible with Classic SEs");
                errno = EINVAL;
                return (-1);
            }
            else if ((*gfal)->setype != TYPE_SE && (*gfal)->endpoint == NULL && ((*gfal)->free_endpoint = 1) &&
                    ((*gfal)->endpoint = endpointfromsurl ((*gfal)->surls[0], errbuf, errbufsz, 1)) == NULL) {
                gfal_internal_free (*gfal);
                *gfal = NULL;
                return (-1);
            }
            else {
                /* Check if the endpoint is full or not */
                if(strncmp ((*gfal)->endpoint, ENDPOINT_DEFAULT_PREFIX, ENDPOINT_DEFAULT_PREFIX_LEN) == 0)
                    endpoint_offset = ENDPOINT_DEFAULT_PREFIX_LEN;
                else
                    endpoint_offset = 0;
                const char *s = strchr ((*gfal)->endpoint + endpoint_offset, '/');
                const char *p = strchr ((*gfal)->endpoint + endpoint_offset, ':');

                if (((*gfal)->setype == TYPE_SRMv2 && s == NULL) || p == NULL || (s != NULL && s < p)) {
                    gfal_internal_free (*gfal);
                    *gfal = NULL;
                    gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                            "[GFAL][gfal_init][EINVAL] Invalid request: When BDII checks are disabled, you must provide full endpoint");
                    errno = EINVAL;
                    return (-1);
                }

                return (0);
            }

        } else {
            gfal_internal_free (*gfal);
            *gfal = NULL;
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                    "[GFAL][gfal_init][EINVAL] Invalid request: When BDII checks are disabled, you must provide SURLs and endpoint types");
            errno = EINVAL;
            return (-1);
        }
    }

    if ((*gfal)->endpoint == NULL) {
        /* (*gfal)->surls != NULL */
        if ((*gfal)->surls[0] != NULL) {
            if (((*gfal)->endpoint = endpointfromsurl ((*gfal)->surls[0], errbuf, errbufsz, 0)) == NULL)
                return (-1);
            (*gfal)->free_endpoint = 1;
        } else {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                    "[GFAL][gfal_init][EINVAL] Invalid request: You have to specify either an endpoint or at least one SURL");
            gfal_internal_free (*gfal);
            *gfal = NULL;
            errno = EINVAL;
            return (-1);
        }
    }
    if ((strchr ((*gfal)->endpoint, '.') == NULL)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] No domain name specified for storage element endpoint");
        gfal_internal_free (*gfal);
        *gfal = NULL;
        errno = EINVAL;
        return (-1);
    }
    if (setypesandendpoints ((*gfal)->endpoint, &se_types, &se_endpoints, errbuf, errbufsz) < 0) {
        gfal_internal_free (*gfal);
        *gfal = NULL;
        return (-1);
    }

    while (se_types[i]) {
        if (srmv1_endpoint == NULL && strcmp (se_types[i], "srm_v1") == 0)
            srmv1_endpoint = se_endpoints[i];
        else if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
            srmv2_endpoint = se_endpoints[i];
        else {
            free (se_endpoints[i]);
            if ((strcmp (se_types[i], "disk")) == 0)
                isclassicse = 1;
        }

        free (se_types[i]);
        ++i;
    }

    free (se_types);
    free (se_endpoints);

    if ((*gfal)->surls != NULL && strncmp ((*gfal)->surls[0], "sfn:", 4) == 0 &&
            (isclassicse || srmv1_endpoint || srmv2_endpoint)) {
        /* if surls start with sfn:, we force the SE type to classic SE */
        (*gfal)->setype = TYPE_SE;
        if (srmv1_endpoint) free (srmv1_endpoint);
        if (srmv2_endpoint) free (srmv2_endpoint);
        return (0);
    }

    /* srmv2 is the default if nothing specified by user! */
    if ((*gfal)->setype == TYPE_NONE) {
        if (srmv2_endpoint &&
                ((*gfal)->defaultsetype == TYPE_NONE || (*gfal)->defaultsetype == TYPE_SRMv2 ||
                 ((*gfal)->defaultsetype == TYPE_SRM && !srmv1_endpoint))) {
            (*gfal)->setype = TYPE_SRMv2;
        } else if (!(*gfal)->srmv2_spacetokendesc && !(*gfal)->srmv2_desiredpintime &&
                !(*gfal)->srmv2_lslevels && !(*gfal)->srmv2_lsoffset &&	!(*gfal)->srmv2_lscount) {
            if (srmv1_endpoint && (*gfal)->defaultsetype != TYPE_SE)
                (*gfal)->setype = TYPE_SRM;
            else if (srmv2_endpoint || srmv1_endpoint || isclassicse)
                (*gfal)->setype = TYPE_SE;
        }
    }
    else if ((*gfal)->setype == TYPE_SRMv2 && !srmv2_endpoint) {
        (*gfal)->setype = TYPE_NONE;
    } else if ((*gfal)->srmv2_spacetokendesc || (*gfal)->srmv2_desiredpintime ||
            (*gfal)->srmv2_lslevels || (*gfal)->srmv2_lsoffset || (*gfal)->srmv2_lscount) {
        (*gfal)->setype = TYPE_NONE;
    } else {
        if ((*gfal)->setype == TYPE_SRM && !srmv1_endpoint)
            (*gfal)->setype = TYPE_NONE;
        else if ((*gfal)->setype == TYPE_SE && !srmv2_endpoint && !srmv1_endpoint && !isclassicse)
            (*gfal)->setype = TYPE_NONE;
    }

    if ((*gfal)->setype == TYPE_SRMv2) {
        if ((*gfal)->free_endpoint) free ((*gfal)->endpoint);
        (*gfal)->endpoint = srmv2_endpoint;
        (*gfal)->free_endpoint = 1;
        if (srmv1_endpoint) free (srmv1_endpoint);
    } else if ((*gfal)->setype == TYPE_SRM) {
        if ((*gfal)->free_endpoint) free ((*gfal)->endpoint);
        (*gfal)->endpoint = srmv1_endpoint;
        (*gfal)->free_endpoint = 1;
        if (srmv2_endpoint) free (srmv2_endpoint);
    } else if ((*gfal)->setype == TYPE_NONE) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                "[GFAL][gfal_init][EINVAL] Invalid request: Desired SE type doesn't match request parameters or SE");
        gfal_internal_free (*gfal);
        *gfal = NULL;
        if (srmv1_endpoint) free (srmv1_endpoint);
        if (srmv2_endpoint) free (srmv2_endpoint);
        errno = EINVAL;
        return (-1);
    }

    if ((*gfal)->generatesurls) {
        if ((*gfal)->surls == NULL) {
            if (generate_surls (*gfal, errbuf, errbufsz) < 0)
                return (-1);
        } else {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR,
                    "[GFAL][gfal_init][EINVAL] Invalid request: No SURLs must be specified with 'generatesurls' activated");
            gfal_internal_free (*gfal);
            *gfal = NULL;
            errno = EINVAL;
            return (-1);
        }
    }

    return (0);
}

    static int
check_gfal_internal (gfal_internal req, int allow_null_surls, char *errbuf, int errbufsz)
{
    if (req == NULL || req->setype == TYPE_NONE || (!allow_null_surls && req->surls == NULL) ||
            (req->setype != TYPE_SE && req->endpoint == NULL)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][check_gfal_internal][EINVAL] Invalid gfal_internal argument");
        errno = EINVAL;
        return (-1);
    }

    return (0);
}

    static void
free_gfal_results (gfal_filestatus *gfal, int n)
{
    int i;

    for (i = 0; i < n; ++i) {
        if (gfal[i].surl) {
            free (gfal[i].surl);
            gfal[i].surl = NULL;
        }
        
        if (gfal[i].turl) {
            free (gfal[i].turl);
            gfal[i].turl = NULL;
        }

        if (gfal[i].explanation) {
            free (gfal[i].explanation);
            gfal[i].explanation = NULL;
        }

        if (gfal[i].checksumtype) {
            free (gfal[i].checksumtype);
            gfal[i].checksumtype = NULL;
        }

        if (gfal[i].checksum) {
            free (gfal[i].checksum);
            gfal[i].checksum = NULL;
        }

        if (gfal[i].subpaths) {
            free_gfal_results (gfal[i].subpaths, gfal[i].nbsubpaths);
            gfal[i].subpaths = NULL;
        }

        if (gfal[i].nbspacetokens > 0 && gfal[i].spacetokens) {
            int j;
            for (j = 0; j < gfal[i].nbspacetokens; ++j) {
                if (gfal[i].spacetokens[j]) {
                    free (gfal[i].spacetokens[j]);
                    gfal[i].spacetokens[j] = NULL;
                }
            }

            free (gfal[i].spacetokens);
            gfal[i].spacetokens = NULL;
        }
    }

    free (gfal);
}

    static void
free_srmv2_mdstatuses (struct srmv2_mdfilestatus *md, int n)
{
    int i;

    for (i = 0; i < n; ++i) {
        if (md[i].subpaths) {
            free_srmv2_mdstatuses (md[i].subpaths, md[i].nbsubpaths);
            md[i].subpaths = NULL;
        }
    }

    free (md);
}

    static int
copy_gfal_mdresults (struct srmv2_mdfilestatus srmv2, gfal_filestatus *gfal)
{
    int i;
    int n = srmv2.nbsubpaths;

    gfal->surl = srmv2.surl;
    gfal->stat = srmv2.stat;
    gfal->status = srmv2.status;
    gfal->explanation = srmv2.explanation;
    gfal->nbsubpaths = srmv2.nbsubpaths;
    gfal->locality = srmv2.locality;
    gfal->checksumtype = srmv2.checksumtype;
    gfal->checksum = srmv2.checksum;
    gfal->nbspacetokens = srmv2.nbspacetokens;
    gfal->spacetokens = srmv2.spacetokens;


    if (gfal->nbsubpaths > 0) {
        if ((gfal->subpaths = (gfal_filestatus *) calloc (gfal->nbsubpaths, sizeof (gfal_filestatus))) == NULL) {
            errno = ENOMEM;
            return (-1);
        }

        for (i = 0; i < n; ++i)
            copy_gfal_mdresults (srmv2.subpaths[i], gfal->subpaths + i);
    }

    return (0);
}

    static int
copy_gfal_results (gfal_internal req, enum status_type stype)
{
    int i;

    if (req->returncode < 0)
        return (req->returncode);

    if (req->results && req->results_size != req->returncode) {
        free_gfal_results (req->results, req->results_size);
        req->results = NULL;
        req->results_size = 0;
    }
    if (!req->results && req->returncode > 0) {
        req->results_size = req->returncode;
        if ((req->results = (gfal_filestatus *) calloc (req->results_size, sizeof (gfal_filestatus))) == NULL) {
            errno = ENOMEM;
            return (-1);
        }
    }

    for (i = 0; i < req->results_size; ++i) {
        if (req->results[i].surl)
            free (req->results[i].surl);
        if (req->results[i].turl)
            free (req->results[i].turl);
        if (req->results[i].explanation)
            free (req->results[i].explanation);
        if (req->results[i].checksumtype)
            free (req->results[i].checksumtype);
        if (req->results[i].checksum)
            free (req->results[i].checksum);
        if (req->results[i].subpaths) {
            free_gfal_results (req->results[i].subpaths, req->results[i].nbsubpaths);
            req->results[i].subpaths = NULL;
        }

        memset (req->results + i, 0, sizeof (gfal_filestatus));

        if (req->setype == TYPE_SRMv2) {
            if (stype == DEFAULT_STATUS) {
                req->results[i].surl = req->srmv2_statuses[i].surl;
                req->results[i].turl = req->srmv2_statuses[i].turl;
                req->results[i].status = req->srmv2_statuses[i].status;
                req->results[i].explanation = req->srmv2_statuses[i].explanation;
            } else if (stype == MD_STATUS) {
                copy_gfal_mdresults (req->srmv2_mdstatuses[i], req->results + i);
            } else { // stype == PIN_STATUS
                req->results[i].surl = req->srmv2_pinstatuses[i].surl;
                req->results[i].turl = req->srmv2_pinstatuses[i].turl;
                req->results[i].status = req->srmv2_pinstatuses[i].status;
                req->results[i].explanation = req->srmv2_pinstatuses[i].explanation;
                req->results[i].pinlifetime = req->srmv2_pinstatuses[i].pinlifetime;
            }
        }
        else if (req->setype == TYPE_SRM) {
            if (stype == MD_STATUS) {
                req->results[i].surl = req->srm_mdstatuses[i].surl;
                req->results[i].stat = req->srm_mdstatuses[i].stat;
                req->results[i].status = req->srm_mdstatuses[i].status;
            } else { // stype == DEFAULT_STATUS or PIN_STATUS
                req->results[i].surl = req->srm_statuses[i].surl;
                req->results[i].turl = req->srm_statuses[i].turl;
                req->results[i].status = req->srm_statuses[i].status;
            }
        }
        else if (req->setype == TYPE_SE) {
            if (stype == MD_STATUS) {
                /* sfn_getfilemd uses srmv2_mdstatuses field!! */
                copy_gfal_mdresults (req->srmv2_mdstatuses[i], req->results + i);
            } else {
                req->results[i].surl = req->sfn_statuses[i].surl;
                req->results[i].turl = req->sfn_statuses[i].turl;
                req->results[i].status = req->sfn_statuses[i].status;
            }
        }
    }

    return (0);
}

    int
gfal_get_results (gfal_internal req, gfal_filestatus **results)
{
    if (req == NULL || req->results == NULL) {
        *results = NULL;
        return (-1);
    }

    *results = req->results;
    return (req->results_size);
}

    int
gfal_get_ids (gfal_internal req, int *srm_reqid, int **srm_fileids, char **srmv2_reqtoken)
{
    return (gfal_get_ids_setype (req, NULL, srm_reqid, srm_fileids, srmv2_reqtoken));
}

    int
gfal_get_ids_setype (gfal_internal req, enum se_type *type, int *srm_reqid, int **srm_fileids, char **srmv2_reqtoken)
{
    if (srm_reqid) *srm_reqid = -1;
    if (srm_fileids) *srm_fileids = NULL;
    if (srmv2_reqtoken) *srmv2_reqtoken = NULL;

    if (req == NULL || req->results_size < 1 || req->setype == TYPE_NONE)
        return (-1);

    if (type != NULL) *type = req->setype;

    if (req->srm_statuses) { // SRMv1
        int i;

        if (srm_reqid) *srm_reqid = req->srm_reqid;

        if (srm_fileids)  {
            if ((*srm_fileids = (int *) calloc (req->results_size, sizeof (int))) == NULL)
                return (-1);

            for (i = 0; i < req->results_size; ++i)
                (*srm_fileids)[i] = req->srm_statuses[i].fileid;
        }
    }
    else if (req->srmv2_token && srmv2_reqtoken) { // SRMv2
        *srmv2_reqtoken = strdup (req->srmv2_token);
    }

    return (req->results_size);
}

    int
gfal_set_ids (gfal_internal req, int nbfileids, const int *srm_fileids, int srm_reqid, const char *srmv2_reqtoken,
        char *errbuf, int errbufsz)
{
    if (req == NULL || req->nbfiles < 0 || (srm_fileids == NULL && srmv2_reqtoken == NULL)) {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_set_ids][EINVAL] Invalid arguments");
        errno = EINVAL;
        return (-1);
    }

    if (nbfileids > 0 && srm_fileids) {
        int i;

        if (nbfileids != req->nbfiles) {
            gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_set_ids][EINVAL] mismatch between number of fileids and number of files");
            errno = EINVAL;
            return (-1);
        }

        if (!req->srm_statuses) {
            req->srm_statuses = (struct srm_filestatus *) calloc (req->nbfiles, sizeof (struct srm_filestatus));
            if (!req->srm_statuses)
                return (-1);
        }

        req->srm_reqid = srm_reqid;
        for (i = 0; i < req->nbfiles; ++i)
            req->srm_statuses[i].fileid = srm_fileids[i];
    }

    if (srmv2_reqtoken) {
        req->srmv2_token = strdup (srmv2_reqtoken);
        if (!req->srmv2_token)
            return (-1);
    }

    return (0);
}

    void
gfal_internal_free (gfal_internal req)
{
    int i;

    if (req == NULL)
        return;
    if (req->free_endpoint && req->endpoint) {
        free (req->endpoint);
        req->endpoint = NULL;
    }

    if (req->sfn_statuses) {
        free (req->sfn_statuses);
        req->sfn_statuses = NULL;
    }

    if (req->srm_statuses) {
        free (req->srm_statuses);
        req->srm_statuses = NULL;
    }

    if (req->srm_mdstatuses) {
        free (req->srm_mdstatuses);
        req->srm_mdstatuses = NULL;
    }
    if (req->srmv2_token) {
        free (req->srmv2_token);
        req->srmv2_token = NULL;
    }

    if (req->srmv2_statuses) {
        free (req->srmv2_statuses);
        req->srmv2_statuses = NULL;
    }
    if (req->srmv2_pinstatuses) {
        free (req->srmv2_pinstatuses);
        req->srmv2_pinstatuses = NULL;
    }

    if (req->srmv2_mdstatuses) {
        free_srmv2_mdstatuses (req->srmv2_mdstatuses, req->results_size);
        req->srmv2_mdstatuses = NULL;
    }

    if (req->results) {
        free_gfal_results (req->results, req->results_size);
        req->results = NULL;
    }

    if (req->generatesurls && req->surls) {
        for (i = 0; i < req->nbfiles; ++i) {
            if (req->surls[i]) {
                free (req->surls[i]);
                req->surls[i] = NULL;
            }
        }

        free (req->surls);
        req->surls = NULL;
    }

    free (req);
    return;
}

    void
gfal_spacemd_free (int nbtokens, struct srm_spacemd *smd)
{
    int i;

    if (smd == NULL)
        return;

    for (i = 0; i < nbtokens; ++i) {
        if (smd[i].spacetoken) {
            free (smd[i].spacetoken);
            smd[i].spacetoken = NULL;
        }

        if (smd[i].owner) {
            free (smd[i].owner);
            smd[i].owner = NULL;
        }
    }

    free (smd);
    smd = NULL;
}

    const char *
gfal_version ()
{
    return gfalversion;
}

    void
gfal_set_nobdii (int value)
{
    nobdii = value;
}

    int
gfal_is_nobdii ()
{
    return (nobdii);
}

char *
gfal_generate_lfn (char *errbuf, int errbufsz) {
    char str_uuid[GFAL_GUID_LEN];
    uuid_t uuid;
    time_t current_time;
    struct tm *tm;
    char timestr[11];
    char *lfn = NULL, *vo = NULL;

    time (&current_time);
    tm = localtime (&current_time);
    strftime (timestr, 11, "%F", tm);
    uuid_generate (uuid);
    uuid_unparse (uuid, str_uuid);

    if ((vo = gfal_get_vo (errbuf, errbufsz)) == NULL)
        return (NULL);

    asprintf (&lfn, "/grid/%s/generated/%s/file-%s", vo, timestr, str_uuid);
    return (lfn);
}

char *
gfal_generate_guid (char *errbuf, int errbufsz) {
    char str_uuid[GFAL_GUID_LEN];
    uuid_t uuid;

    uuid_generate (uuid);
    uuid_unparse (uuid, str_uuid);
    return (strdup (str_uuid));
}

/** extract a hostname from a SURL.  We search for "://" to get the start of
  the hostname.  Then we keep going to the next slash, colon or end of the
  SURL. */
char *
gfal_get_hostname (const char *path, char *errbuf, int errbufsz) {
    char *start;
    char *cp;
    char *result;
    char c;
    char sav_path[GFAL_PATH_MAXLEN];

    strcpy (sav_path, path);

    start = strchr (sav_path, ':');
    if ( start == NULL || *(start+1) != '/' || *(start+2) != '/') {
        gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gfal_get_hostname][EINVAL] %s: Invalid syntax", path);
        errno = EINVAL;
        return (NULL);
    }
    /* point start to beginning of hostname */
    start += 3;
    for (cp = start; *cp != '\0' && *cp != ':' && *cp != '/'; cp++) ;

    c = *cp;
    *cp = '\0';
    if ((result = strdup (start)) == NULL) {
        errno = ENOMEM;
        return (NULL);
    }
    *cp = c;
    return result;
}

/* to allow users to access errno value with python API */
int
gfal_get_errno () {
    return (errno);
}
