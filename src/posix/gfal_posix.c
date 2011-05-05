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

/**
 * @file gfal_posix.c
 * @brief main file of the posix lib
 * @author Devresse Adrien
 * @version 2.0
 * @date 11/04/2011
 * */


#include "../common/gfal_common.h"
#include "../common/gfal_constants.h" 
#include "../common/gfal_file.h"
#include "../common/gfal_common_storage.h"
#include "gfal_posix_api.h"
#include <dlfcn.h>



static __thread int nobdii = 0;

int gfal_access (const char *path, int amode){
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


                    srm_context_init(&context,gfile->gobj->endpoint,errbuf,GFAL_ERRMSG_LEN,gfal_get_verbose());

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
                    /* fix it : need to be replaced by a correct call to get if (!(bool_issurlok = gfal_turlsfromsurls (gfile->gobj, errbuf, GFAL_ERRMSG_LEN) >= 0))
                        gfal_file_set_replica_error (gfile, errno, errbuf);

					*/
					g_error("transition code, not implemented fix it");
					/* */
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

int gfal_chmod (const char *path, mode_t mode){
    char pathbuf[1104];
    char *cat_type = NULL;
    int islfc;
    GError* err=NULL;

    if (canonical_url (path, "file", pathbuf, 1104, NULL, 0) < 0)
        return (-1);

    if (( cat_type = gfal_get_cat_type(&err) ) == NULL){
		gfal_release_GError(&err);
		free (cat_type);   
        return (-1);
     }
    islfc = strcmp (cat_type, "lfc") == 0;
    free (cat_type);

    if (islfc && strncmp (pathbuf, "lfn:", 4) == 0)
        return (lfc_chmodl (pathbuf + 4, mode, NULL, 0));

    /* gfal_chmod is only supported with LFC and LFNs */
    errno = EPROTONOSUPPORT;
    return (-1);
}



int gfal_close (int fd){
    int rc;
    int sav_errno = 0;
    struct xfer_info *xi;
    GError* err=NULL;

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

        if (( cat_type = gfal_get_cat_type(&err) ) == NULL){
			gfal_release_GError(err);
			free (cat_type);
            return (-1);
		}
        islfc = strcmp (cat_type, "lfc") == 0;
        free (cat_type);

        if (islfc)
            lfc_setsize (xi->gfile->lfn, xi->size, NULL, 0);
    }

    free_xi (fd);
    errno = sav_errno;
    return (rc);
}




int gfal_closedir (DIR *dir){
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

int gfal_creat (const char *filename, mode_t mode){
    return (gfal_open (filename, O_WRONLY|O_CREAT|O_TRUNC, mode));
}

int gfal_creat64 (const char *filename, mode_t mode){
    return (gfal_open64 (filename, O_WRONLY|O_CREAT|O_TRUNC, mode));
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
    GError* err = NULL;

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
        if ((cat_type = gfal_get_cat_type(&err)) ==NULL){
			gfal_release_GError(&err);
			free(cat_type);
            return (-1);
		}
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
        srm_context_init(&context,srmv2_endpoint,NULL,0,gfal_get_verbose());

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
        gfal_print_verbose(GFAL_VERBOSE_NORMAL, "[gfal_open] new file requested \n");
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

/* fix it : need to be replaced by a correctcall to get        
 *    if (bool_issurlok && !(bool_issurlok = gfal_turlsfromsurls (gfile->gobj, errbuf, GFAL_ERRMSG_LEN) >= 0))
                gfal_file_set_replica_error (gfile, errno, errbuf);
*/
			g_error("transition code, not implemented fix it");
			/* */
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
    GError* err = NULL;

    if (canonical_url (dirname, "file", path, 1104, NULL, 0) < 0)
        return (NULL);

    if ((strncmp (path, "lfn:", 4) == 0 && (islfn = 1)) ||
            strncmp (path, "guid:", 5) == 0) {
        void *dlhandle;
        char *cat_type;

        if (( cat_type = gfal_get_cat_type(&err) ) == NULL){
			gfal_release_GError(&err);
			free(cat_type);
            return (NULL);
		}
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
    GError* err=NULL;

    if (canonical_url (old_name, "file", path1, 1104, NULL, 0) < 0 ||
            canonical_url (new_name, "file", path2, 1104, NULL, 0) < 0)
        return (-1);

    if (strncmp (path1, "lfn:", 4) == 0 && strncmp (path2, "lfn:", 4) == 0) {
        int islfc,isedg;
        char *cat_type;
        if (( cat_type = gfal_get_cat_type(&err) ) == NULL){
			gfal_release_GError(&err);
			free(cat_type);
            return (-1);
		}

        islfc = strcmp (cat_type, "lfc") == 0;
        isedg = 0;
        free (cat_type);

        if (islfc)
            return lfc_renamel (path1 + 4, path2 + 4, NULL, 0);
        else if (isedg) {
			/* rmc remvoed fix it
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
            *  */
            g_error("rmc removed, temporarly disabled");
            /* */
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
    GError* err=NULL;

    if (canonical_url (dirname, "file", path, 1104, NULL, 0) < 0)
        return (-1);

    if (strncmp (path, "lfn:", 4) == 0) {
        int islfc;
        char *cat_type;
        if (( cat_type = gfal_get_cat_type(&err) ) == NULL){
			gfal_release_GError(&err);
			free(cat_type);
            return (-1);
		}
		
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
        srm_context_init(&context,srmv2_endpoint,NULL,0,gfal_get_verbose());

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

    /* gfal_ls removed : fix it        if (bool_issurlok && !(bool_issurlok = gfal_ls (gfile->gobj, errbuf, GFAL_ERRMSG_LEN) >= 0))
                gfal_file_set_replica_error (gfile, errno, errbuf);
	*/
	g_error("gfal_ls removed, function disabled temporarly");
	/* */
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
        gfal_handle gobj;
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

            gfal_handle_free (gobj);
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
	  /* lrc removed : fix it
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
        */
     g_error("lrc removed : not avaialable temporarly");
     /* */
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
gfal_deletesurls (gfal_handle req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_handle (req, 0, errbuf, errbufsz) < 0)
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


        srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_get_verbose());
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
        perror("FEATURE DISABLED in 1.X, need fix");	 // fix this
        errno = EINVAL;
        return -1;
        /*ret = sfn_deletesurls (req->nbfiles, (const char **) req->surls,
                &(req->sfn_statuses), errbuf, errbufsz, gfal_get_timeout_sendreceive ());*/
    }

    req->returncode = ret;
    return (copy_gfal_results (req, DEFAULT_STATUS));
}

    int
gfal_removedir (gfal_handle req, char *errbuf, int errbufsz)
{
    int ret;

    if (check_gfal_handle (req, 0, errbuf, errbufsz) < 0)
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

        srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_get_verbose());
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
int gfal_create_subdirs(gfal_handle req, char *errbuf, int errbufsz)
{
	int i,result = 0 ;
	struct srm_context context;
	struct srm_mkdir_input mkdir_input;

	srm_context_init(&context,req->endpoint,errbuf,errbufsz,gfal_get_verbose());
	/* Create sub-directories of SURLs */
	for (i = 0; i < req->nbfiles; ++i) {
        const gchar* dir = g_strconcat((gchar*)req->surls[i], "/", NULL);	// concat the two string
        g_assert(dir);	
        int res = 0;

        if(strlen(dir) > 0){
        	mkdir_input.dir_name = (char*) dir;
            res = srm_mkdir(&context,&mkdir_input);
        }
		g_free((gpointer) dir);
        if (res < 0) 
        	result = -1;
	}
	return result;
}


void gfal_set_nobdii (int value){
    nobdii = value;
}

int gfal_is_nobdii (){
    return (nobdii);
}


