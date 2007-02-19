/*
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: gfal.c,v $ $Revision: 1.37 $ $Date: 2007/02/19 16:18:40 $ CERN Jean-Philippe Baud
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <uuid/uuid.h>
#include "gfal.h"
#include "gfal_api.h"
#if !defined(OFF_MAX)
#define OFF_MAX 2147483647
#endif

/* size in argument to turlfromsurl2 to create a new file
 * (used to check wether there is enough space on the SE) */
#define DEFAULT_NEWFILE_SIZE 1024

static struct dir_info *di_array[GFAL_OPEN_MAX];
static struct xfer_info *xi_array[GFAL_OPEN_MAX];

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
		free (xi_array[fd]);
		xi_array[fd] = NULL;
	}
	return (0);
}

gfal_access (const char *path, int amode)
{
	char errbuf[256];
	char pathbuf[1024];
	char *guid = NULL, *surl = NULL, *turl = NULL, *pfn = NULL;
	struct proto_ops *pops;
	char protocol[64];
	char **supported_protocols;
	int reqid, fileid;
	char *token = NULL;

	strncpy (pathbuf, path, 1024);

	if (strncmp (path, "lfn:", 4) == 0) {
		char *cat_type;
		int islfc;
		if (get_cat_type (&cat_type) < 0)
			return (-1);
		islfc = strcmp (cat_type, "lfc") == 0;
		free (cat_type);

		if (islfc)
			return lfc_accessl (path + 4, amode, errbuf, sizeof(errbuf));

		if ((guid = guidfromlfn (path + 4, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
	}
	if (guid || (strncmp (path, "guid:", 5) == 0 && (guid = pathbuf + 5))) {
		if ((surl = surlfromguid (guid, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		if (guid != pathbuf) free (guid);
	}
	if ((surl || (surl = pathbuf)) && strncmp (surl, "srm:", 4) == 0) {
		supported_protocols = get_sup_proto ();

		if ((turl = turlfromsurl (surl, supported_protocols, R_OK,
		    &reqid, &fileid, &token, errbuf, sizeof(errbuf), 0)) == NULL)
			return (-1);
		if (surl != path) free (surl);
	} else if (strncmp (surl, "sfn:", 4) == 0) {
		supported_protocols = get_sup_proto ();

		if ((turl = turlfromsfn (surl, supported_protocols, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		if (surl != path) free (surl);
	}
	if (turl == NULL && (turl = strdup (path)) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	if (parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf)) < 0) {
		free (turl);
		return (-1);
	}
	if ((pops = find_pops (protocol)) == NULL) {
		free (turl);
		return (-1);
	}
	if (pops->access (pfn, amode) < 0) {
		free (turl);
		errno = pops->maperror (pops, 0);
		return (-1);
	}

	free (turl);
	errno = 0;
	return (0);
}

gfal_chmod (const char *path, mode_t mode)
{
	char errbuf[256];
	char pathbuf[1024];
	char *guid = NULL, *surl = NULL, *turl = NULL, *pfn = NULL;
	struct proto_ops *pops;
	char protocol[64];
	char **supported_protocols;
	int reqid, fileid;
	char *token = NULL;

	strncpy (pathbuf, path, 1024);

	if (strncmp (path, "lfn:", 4) == 0) {
		char *cat_type;
		int islfc;
		if (get_cat_type (&cat_type) < 0)
			return (-1);
		islfc = strcmp (cat_type, "lfc") == 0;
		free (cat_type);
		if (islfc)
			return (lfc_chmodl (path + 4, mode, errbuf, sizeof(errbuf)) < 0);
		if ((guid = guidfromlfn (path + 4, errbuf, sizeof(errbuf))) == NULL)
			return (-1);

	}
	if (guid || (strncmp (path, "guid:", 5) == 0 && (guid = pathbuf + 5))) {
		if ((surl = surlfromguid (guid, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		if (guid != pathbuf + 5) free (guid);
	}
	if ((surl || (surl = pathbuf)) && strncmp (surl, "srm:", 4) == 0) {
		supported_protocols = get_sup_proto ();

		if ((turl = turlfromsurl (surl, supported_protocols, R_OK,
		    &reqid, &fileid, &token, errbuf, sizeof(errbuf), 0)) == NULL)
			return (-1);
		if (surl != pathbuf) free (surl);
	} else if (strncmp (surl, "sfn:", 4) == 0) {
		supported_protocols = get_sup_proto ();

		if ((turl = turlfromsfn (surl, supported_protocols, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		if (surl != pathbuf) free (surl);
	}
	if (turl == NULL && (turl = strdup (path)) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	if (parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf)) < 0) {
		free (turl);
		return (-1);
	}
	if ((pops = find_pops (protocol)) == NULL) {
		free (turl);
		return (-1);
	}
	if (pops->chmod (pfn, mode) < 0) {
		free (turl);
		errno = pops->maperror (pops, 0);
		return (-1);
	}

	free (turl);
	errno = 0;
	return (0);
}

gfal_close (int fd)
{
	char errbuf[256];
	int rc;
	int rc1 = 0;
	int sav_errno;
	struct xfer_info *xi;

	if ((xi = find_xi (fd)) == NULL)
		return (-1);
	if ((rc = xi->pops->close (fd)) < 0)
		sav_errno = xi->pops->maperror (xi->pops, 1);

	/* set status "done" */

	if (xi->surl) {
		rc1 = set_xfer_done (xi->surl, xi->reqid, xi->fileid,
		    xi->token, xi->oflag, errbuf, sizeof(errbuf), 0);
		free (xi->surl);
		if (xi->token) free (xi->token);
	}
	(void) free_xi (fd);
	if (rc) {
		errno = sav_errno;
		return (-1);
	}
	return (rc1);
}

gfal_closedir (DIR *dir)
{
	struct dir_info *di;
	int rc;

	if ((di = find_di (dir))) {
		if ((rc = di->pops->closedir (dir)) < 0)
			errno = di->pops->maperror (di->pops, 0);
		if (strcmp (di->pops->proto_name, "lfc") == 0)
			free (di->pops);
		free_di (di);
		return (rc);
	} else
		return (-1);
}

gfal_creat (const char *filename, mode_t mode)
{
	return (gfal_open (filename, O_WRONLY|O_CREAT|O_TRUNC, mode));
}

gfal_creat64 (const char *filename, mode_t mode)
{
	return (gfal_open64 (filename, O_WRONLY|O_CREAT|O_TRUNC, mode));
}

void
gfal_errmsg (char *errbuf, int errbufsz, const char *errmsg)
{
	if (errbuf == NULL)
		fprintf (stderr, "%s\n", errmsg);
	else {
		strncpy (errbuf, errmsg, errbufsz - 1);
		*(errbuf+errbufsz-1) = '\0';
	}
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

gfal_lstat (const char *filename, struct stat *statbuf)
{
	char errbuf[256];
	char *fn;
	char *guid;
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];
	int rc;
	struct stat64 statb64;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid, errbuf, sizeof(errbuf))) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		if ((rc = getfilemd (fn, &statb64, errbuf, sizeof(errbuf), 0)) == 0)
			rc = mdtomd32 (&statb64, statbuf);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL, errbuf, sizeof(errbuf))) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf))) == 0) {
		if ((pops = find_pops (protocol)) != NULL) {
			if ((rc = pops->lstat (pfn, statbuf)) < 0)
				errno = pops->maperror (pops, 0);
		}
	}
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	if (rc < 0 || pops == NULL)
		return (-1);
	errno = 0;
	return (0);
}

gfal_lstat64 (const char *filename, struct stat64 *statbuf)
{
	char errbuf[256];
	char *fn;
	char *guid;
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];
	int rc;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid, errbuf, sizeof(errbuf))) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		rc = getfilemd (fn, statbuf, errbuf, sizeof(errbuf), 0);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL, errbuf, sizeof(errbuf))) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf))) == 0) {
		if ((pops = find_pops (protocol)) != NULL) {
			if ((rc = pops->lstat64 (pfn, statbuf)) < 0)
				errno = pops->maperror (pops, 0);
		}
	}
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	if (rc < 0 || pops == NULL)
	   	return (-1);
	errno = 0;
	return (0);
}

gfal_mkdir (const char *dirname, mode_t mode)
{
	char errbuf[256];
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];

	if (strncmp (dirname, "guid:", 5) == 0 ||
	    strncmp (dirname, "srm:", 4) == 0 ||
	    strncmp (dirname, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}

	if (strncmp (dirname, "lfn:", 4) == 0) {
		int islfc, isedg;
		char *cat_type;
		if (get_cat_type (&cat_type) < 0)
			return (-1);

		islfc = strcmp (cat_type, "lfc") == 0;
		isedg = strcmp (cat_type, "edg") == 0;
		free (cat_type);

		if (islfc)
			return lfc_mkdirp (dirname + 4, mode, errbuf, sizeof(errbuf));
 		else if (isedg)
			return se_mkdir (dirname, errbuf, sizeof(errbuf), 60);
			
		errno = EPROTONOSUPPORT;
		return (-1);
	}

	if (parseturl (dirname, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf)) < 0)
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

gfal_open (const char *filename, int flags, mode_t mode)
{
	char errbuf[256];
	int fd = -1;
	int i;
	int fileid;
	int newfile = 0;
	char *fn = NULL;
	char *lfn = NULL, *guid = NULL;
	char guid_file[37], guid_lfn[37], guid_surl[37];
	char dir_path[1104];
	uuid_t uuid;
	char pathbuf[1024];
	char *pfn;
	char *vo;
	char *default_se;
	char *se_type = NULL;
	int se_isdisk;
	char **ap;
	int *pn;
	int port;
	char *sa_path;
	char *sa_root;
	time_t current_time;
	struct tm *tm;
	char timestr[11];
	char *ce_ap;
	struct proto_ops *pops;
	char protocol[64];
	int reqid;
	char **supported_protocols;
	char *token = NULL;
	char *turl = NULL;
	struct xfer_info *xi;

	supported_protocols = get_sup_proto ();

	if ((flags & (O_WRONLY | O_CREAT)) == (O_WRONLY | O_CREAT)) {
		/* writing in a file, so a new file */
		newfile = 1;

		/* we get the VO name from the corresponding environment variable */
		if ((vo = getenv ("LCG_GFAL_VO")) == NULL) {
			errno = EINVAL;
			goto err;
		}
	}

	if (strncmp (filename, "lfn:", 4) == 0) {
		if (!newfile) {
			if ((guid = guidfromlfn (filename + 4, errbuf, sizeof(errbuf))) == NULL)
				goto err;
			if ((fn = surlfromguid (guid, errbuf, sizeof(errbuf))) == NULL)
				goto err;
		} else { // newfile
			/* We check whether LFN exists, and if no, we create it */
			if ((guid = guidfromlfn (filename + 4, errbuf, sizeof(errbuf))) != NULL) {
				/* LFN already exists, and we don't modify existing files, so ... error */
				errno = EEXIST;
				goto err;
			}

			lfn = strdup (filename + 4);
			if (lfn == NULL) {
				errno = ENOMEM;
				goto err;
			}

			/* We generate a new GUId for the file */
			uuid_generate (uuid);
			uuid_unparse (uuid, guid_file);
		}
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if (!newfile) {
			if ((fn = surlfromguid (filename + 5, errbuf, sizeof(errbuf))) == NULL)
				goto err;
		} else { // newfile
			/* we check the format of the given GUID */
			if (uuid_parse (filename + 5, uuid) < 0) {
				errno = EINVAL; /* invalid guid */
				goto err;
			}

			strncpy (guid_file, filename + 5, 37);

			/* We generate a new LFN for the file */
			uuid_generate (uuid);
			uuid_unparse (uuid, guid_lfn);

			(void) time (&current_time);
			tm = localtime (&current_time);
			strftime (timestr, 11, "%F", tm);

			asprintf (&lfn, "/grid/%s/generated/%s/file%s", vo, timestr, guid_lfn);

			if (lfn == NULL) {
				errno = ENOMEM;
				goto err;
			}
		}
	} else
		fn = (char *)filename;

	if (newfile && !fn) {
		/* We get the default se (for the VO), its type and its info */
		if ((default_se = get_default_se(vo, errbuf, sizeof(errbuf))) == NULL)
			goto err;
		if (get_se_typex (default_se, &se_type, errbuf, sizeof(errbuf)) < 0)
			goto err;

		se_isdisk = strcmp (se_type, "disk") == 0;
		if (!se_isdisk && strcmp (se_type, "srm_v1") &&
		    strcmp (se_type, "edg-se")) {
			free (se_type);
			gfal_errmsg(errbuf, sizeof(errbuf), "The Storage Element type is neither 'disk', 'srm_v1' nor 'edg-se'.");
			errno = EINVAL;
			goto err;
		}
		free (se_type);

		if (get_seap_infox (default_se, &ap, &pn, errbuf, sizeof(errbuf)) < 0)
			goto err;

		for (i=0; ap[i]; ++i) {
			if (strcmp (ap[i], "gsiftp") == 0) port = pn[i];
			free (ap[i]);
		}

		free (ap);
		free (pn);

		if (!port) {
			gfal_errmsg(errbuf, sizeof(errbuf), "protocol not supported by Storage Element.");
			errno = EPROTONOSUPPORT;
			goto err;
		}

		/* now create dir path which is either sa_path, sa_root or combination of ce_ap & sa_root */
		if(get_sa_path (default_se, vo, &sa_path, &sa_root, errbuf, sizeof(errbuf)) < 0) 
			goto err;
		if(sa_path != NULL) {
			if (se_isdisk)
				sprintf (dir_path, "sfn://%s%s%s", default_se, *sa_path=='/'?"":"/", sa_path);
			else
				sprintf (dir_path, "srm://%s%s%s", default_se, *sa_path=='/'?"":"/", sa_path);
		} else {  /* sa_root != NULL */
			if (se_isdisk) {
				if (get_ce_apx (default_se, &ce_ap, errbuf, sizeof(errbuf)) < 0)
					goto err;

				sprintf (dir_path, "sfn://%s%s%s%s%s", default_se, 
					 *ce_ap=='/'?"":"/", ce_ap, 
					 *sa_root=='/'?"":"/", sa_root);
				free (ce_ap);
			} else {
				sprintf (dir_path, "srm://%s%s%s", default_se, *sa_root=='/'?"":"/", sa_root);
			}
		}
		free (sa_path);
		free (sa_root);

		(void) time (&current_time);
		tm = localtime (&current_time);
		strftime (timestr, 11, "%F", tm);
		uuid_generate (uuid);
		uuid_unparse (uuid, guid_surl);
		asprintf (&fn, "%s/generated/%s/file%s", dir_path, timestr, guid_surl);

		if (fn == NULL) {
			errno = ENOMEM;
			goto err;
		}
	}

	if (strncmp (fn, "srm:", 4) == 0) {
		if ((turl = turlfromsurl2 (fn, DEFAULT_NEWFILE_SIZE, NULL, supported_protocols,
		    flags, &reqid, &fileid, &token, errbuf, sizeof(errbuf), 0)) == NULL)
			goto err;
	} else if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, supported_protocols, errbuf, sizeof(errbuf))) == NULL)
			goto err;
	} else		/* assume that is a pfn */
		turl = fn;
	if (parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf)) < 0)
		goto err;
	if ((pops = find_pops (protocol)) == NULL)
		goto err;
	if ((fd = pops->open (pfn, flags, mode)) < 0) {
		errno = pops->maperror (pops, 1);
		goto err;
	}
	if ((xi = alloc_xi (fd)) == NULL)
		goto err;
	xi->fd = fd;
	if (newfile && lfn) {
		xi->size = 0;
		xi->lfn = lfn;
	} else	xi->size = -1;
	xi->oflag = flags;
	xi->pops = pops;
	if (strncmp (fn, "srm:", 4) == 0) {
		xi->surl = strdup (fn);
		xi->reqid = reqid;
		xi->fileid = fileid;
		if (token) {
			xi->token = strdup (token);
			free (token);
		}
		(void) set_xfer_running (xi->surl, xi->reqid, xi->fileid,
		    xi->token, errbuf, sizeof(errbuf), 0);
	}

	if (newfile && lfn) {
		if (create_alias_m (guid_file, lfn, mode, 0, errbuf, sizeof(errbuf)) < 0)
			goto err;
		if (register_pfn (guid_file, fn, errbuf, sizeof(errbuf)) < 0) {
			unregister_alias (guid_file, filename, errbuf, sizeof(errbuf));
			goto err;
		}
	}

	if (guid) free (guid);
	//if (lfn) free (lfn);
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	errno = 0;
	return (fd);

err:
	if (fd >= 0) {
		int sav_errno = errno;
		deletepfn (pfn, NULL, errbuf, sizeof(errbuf));
		errno = sav_errno;
	}
	if (guid) free (guid);
	if (lfn) free (lfn);
	if (fn != filename) free (fn);
	if (turl && turl != fn) free (turl);
	return (-1);
}

gfal_open64 (const char *filename, int flags, mode_t mode)
{
	return (gfal_open (filename, flags | O_LARGEFILE, mode));
}

DIR *
gfal_opendir (const char *dirname)
{
	struct dir_info *di;
	DIR *dir;
	char errbuf[256];
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];
	int islfn = 0;

	if ((strncmp (dirname, "lfn:", 4) == 0 && (islfn = 1)) ||
	    strncmp (dirname, "guid:", 5) == 0) {
		void *dlhandle;
		struct proto_ops *pops;
		char *cat_type;
		if (get_cat_type (&cat_type) < 0)
			return (NULL);
		if (strcmp (cat_type, "lfc") != 0) {
			errno = EPROTONOSUPPORT;
			return (NULL);
		}
		free (cat_type);

		if (islfn) dir = (DIR *) lfc_opendirlg (dirname + 4, NULL, errbuf, sizeof(errbuf));
		else	   dir = (DIR *) lfc_opendirlg (NULL, dirname, errbuf, sizeof(errbuf));

		if ((di = alloc_di (dir)) == NULL)
			return (NULL);
		if ((pops = (struct proto_ops *) malloc (sizeof(struct proto_ops))) == NULL) {
			errno = ENOMEM;
			return (NULL);
		}
		bzero (pops, sizeof(struct proto_ops));

		if ((dlhandle = dlopen ("liblfc.so", RTLD_LAZY)) == NULL)
			return (NULL);

		pops->proto_name = "lfc";
		pops->maperror = lfc_maperror;
		pops->readdir = (struct dirent * (*) (DIR *)) dlsym (dlhandle, "lfc_readdir");
		pops->closedir = (int (*) (DIR *)) dlsym (dlhandle, "lfc_closedir");

		di->pops = pops;
		errno = 0;
		return dir;
	}

	if (strncmp (dirname, "srm:", 4) == 0 ||
	    strncmp (dirname, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (NULL);
	}
	if (parseturl (dirname, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf)) < 0)
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

	if ((di = find_di (dir)) == NULL)
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

gfal_rename (const char *old_name, const char *new_name)
{
	char *new_pfn;
	char *old_pfn;
	char errbuf[256];
	char pathbuf1[1024];
	char pathbuf2[1024];
	struct proto_ops *pops;
	char protocol1[64];
	char protocol2[64];

	if (strncmp (old_name, "lfn:", 4) == 0 && strncmp (new_name, "lfn:", 4) == 0) {
		int islfc,isedg;
		char *cat_type;
		if (get_cat_type (&cat_type) < 0)
			return (-1);

		islfc = strcmp (cat_type, "lfc") == 0;
		isedg = strcmp (cat_type, "edg") == 0;
		free (cat_type);

		if (islfc)
			return lfc_renamel (old_name + 4, new_name + 4, errbuf, sizeof(errbuf));
		else if (isedg) {
			char *guid;
			int rc;

			if ((guid = guidfromlfn (old_name + 4, errbuf, sizeof(errbuf))) == NULL)
				return (-1);
			if (rmc_register_alias (guid, new_name, errbuf, sizeof(errbuf)) < 0) {
				free (guid);
				return (-1);
			}
			rc = rmc_unregister_alias (guid, old_name, errbuf, sizeof(errbuf));
			free (guid);
			return (rc);
		}
 
		errno = EPROTONOSUPPORT;
		return (-1);
	}

	if (strncmp (old_name, "lfn:", 4) == 0 ||
	    strncmp (old_name, "guid:", 5) == 0 ||
	    strncmp (old_name, "srm:", 4) == 0 ||
	    strncmp (old_name, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	if (strncmp (new_name, "lfn:", 4) == 0 ||
	    strncmp (new_name, "guid:", 5) == 0 ||
	    strncmp (new_name, "srm:", 4) == 0 ||
	    strncmp (new_name, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	if (parseturl (old_name, protocol1, sizeof(protocol1), pathbuf1, sizeof(pathbuf1), &old_pfn, errbuf, sizeof(errbuf)) < 0)
		return (-1);
	if (parseturl (new_name, protocol2, sizeof(protocol2), pathbuf2, sizeof(pathbuf2), &new_pfn, errbuf, sizeof(errbuf)) < 0)
		return (-1);
	if (strcmp (protocol1, protocol2)) {
		errno = EINVAL;
		return (-1);
	}
	if ((pops = find_pops (protocol1)) == NULL)
		return (-1);
	if (pops->rename (old_pfn, new_pfn) < 0) {
		errno = pops->maperror (pops, 0);
		return (-1);
	}
	errno = 0;
	return (0);
}

gfal_rmdir (const char *dirname)
{
	char errbuf[256];
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];

	if (strncmp (dirname, "lfn:", 4) == 0) {
		int islfc;
		char *cat_type;
		if (get_cat_type (&cat_type) < 0)
			return (-1);

		islfc = strcmp (cat_type, "lfc") == 0;
		free (cat_type);

		if (islfc)
			return lfc_rmdirl (dirname + 4, errbuf, sizeof(errbuf));
 
		errno = EPROTONOSUPPORT;
		return (-1);
	}

	if (strncmp (dirname, "lfn:", 4) == 0 ||
	    strncmp (dirname, "guid:", 5) == 0 ||
	    strncmp (dirname, "srm:", 4) == 0 ||
	    strncmp (dirname, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	if (parseturl (dirname, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf)) < 0)
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

gfal_stat (const char *filename, struct stat *statbuf)
{
	char errbuf[256];
	char *fn;
	char *guid;
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];
	int rc;
	struct stat64 statb64;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid, errbuf, sizeof(errbuf))) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		if ((rc = getfilemd (fn, &statb64, errbuf, sizeof(errbuf), 0)) == 0)
			rc = mdtomd32 (&statb64, statbuf);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL, errbuf, sizeof(errbuf))) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf))) == 0) {
		if ((pops = find_pops (protocol)) != NULL) {
			if ((rc = pops->stat (pfn, statbuf)) < 0)
				errno = pops->maperror (pops, 0);
		}
	}
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	if (rc < 0 || pops == NULL)
		return (-1);
	errno = 0;
	return (0);
}

gfal_stat64 (const char *filename, struct stat64 *statbuf)
{
	char errbuf[256];
	char *fn;
	char *guid;
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];
	int rc;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid, errbuf, sizeof(errbuf))) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		rc = getfilemd (fn, statbuf, errbuf, sizeof(errbuf), 0);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL, errbuf, sizeof(errbuf))) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf),
	    &pfn, errbuf, sizeof(errbuf))) == 0) {
		if ((pops = find_pops (protocol)) != NULL) {
			if ((rc = pops->stat64 (pfn, statbuf)) < 0)
				errno = pops->maperror (pops, 0);
		}
	}
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	if (rc < 0 || pops == NULL)
		return (-1);
	errno = 0;
	return (0);
}

gfal_unlink (const char *filename)
{
	char errbuf[256];
	char *guid, *guid_lfn;
	int islfn = 0, i = 0;
	char **pfns;
	int rc = 0;

	if (strncmp (filename, "lfn:", 4) == 0) {
		islfn = 1;
		if ((guid_lfn = guidfromlfn (filename + 4, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
	}
	if ((islfn && (guid = guid_lfn)) || (strncmp (filename, "guid:", 5) == 0 && (guid = strdup (filename + 5)))) {
		/* must try to delete all PFNs mapped to this guid */
		if ((pfns = surlsfromguid (guid, errbuf, sizeof(errbuf))) == NULL) {
			if (guid != NULL) free (guid);
			return (-1);
		}
		while (pfns[i]) {
			if (deletepfn (pfns[i], guid, errbuf, sizeof(errbuf)) == 0 && islfn)
				unregister_pfn (guid, pfns[i], errbuf, sizeof(errbuf));
			else --rc;
			free (pfns[i++]);
		}
		free (pfns);

		if (rc == 0 && islfn)			
			rc = unregister_alias (guid, filename + 4, errbuf, sizeof(errbuf));

		if (guid != NULL) free (guid);
		return (rc == 0 ? 0 : -1);
	} else
		return (deletepfn (filename, NULL, errbuf, sizeof(errbuf)));
}

ssize_t
gfal_write (int fd, const void *buf, size_t size)
{
	int rc;
	struct xfer_info *xi;

	if ((xi = find_xi (fd)) == NULL)
		return (-1);
	if ((rc = xi->pops->write (fd, buf, size)) < 0)
		errno = xi->pops->maperror (xi->pops, 1);
	if (xi->size >= 0) xi->size += rc;
	errno = 0;
	return (rc);
}

deletepfn (const char *fn, const char *guid, char *errbuf, int errbufsz)
{
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];
	int rc;
	char *turl;

	if (strncmp (fn, "srm:", 4) == 0) {
		if (deletesurl2 (fn, NULL, errbuf, errbufsz, 0) < 0)
			return (-1);
	} else {
		if (strncmp (fn, "sfn:", 4) == 0) {
			if ((turl = turlfromsfn (fn, NULL, errbuf, errbufsz)) == NULL)
				return (-1);
		} else		/* assume that is a pfn */
			turl = (char *)fn;
		if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf))) == 0) {
			if ((pops = find_pops (protocol)) != NULL) {
				if ((rc = pops->unlink (pfn)) < 0)
					errno = pops->maperror (pops, 0);
			}
		}
		if (turl != fn) free (turl);
		if (rc < 0 || pops == NULL)
			return (-1);
	}
	return (0);
}

deletesurl (const char *surl, char *errbuf, int errbufsz, int timeout)
{
	return (deletesurl2 (surl, NULL, errbuf, errbufsz, timeout));
}

deletesurl2 (const char *surl, char *spacetokendesc, char *errbuf, int errbufsz, int timeout)
{
	int rc, i = 0;
	char **se_endpoints;
	char **se_types;
	char *srm_endpoint = NULL;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	int edgse = 0;
	int srm_v1 = 0;
	int srm_v2 = 0;

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "edg-se")) == 0) {
			edgse = 1;
			srm_endpoint = srm_endpoint == NULL ? strdup (se_endpoints[i]) : srm_endpoint;
		} else if ((strcmp (se_types[i], "srm_v1")) == 0) {
			srm_v1 = 1;
			srmv1_endpoint = srmv1_endpoint == NULL ? strdup (se_endpoints[i]) : srmv1_endpoint;
		} else if ((strcmp (se_types[i], "srm_v2")) == 0) {
			srm_v2 = 1;
			srmv2_endpoint = srmv2_endpoint == NULL ? strdup (se_endpoints[i]) : srmv2_endpoint;
		}
		free (se_types[i]);
		free (se_endpoints[i]);
		i++;
	}
	free (se_types);
	free (se_endpoints);

	/* if spacetokendesc specified by user and/or SRM v2.2 supported */
	if (((spacetokendesc != NULL) && srm_v2) || (!srm_v1 && srm_v2))
		rc = srmv2_deletesurl (surl, srmv2_endpoint, errbuf, errbufsz, timeout);
	else if (srm_v1)
		rc = srm_deletesurle (surl, srmv1_endpoint, errbuf, errbufsz, timeout);
	else if (edgse)
		rc = se_deletesurle (surl, srm_endpoint, errbuf, errbufsz, timeout);
	else {
		gfal_errmsg(errbuf, errbufsz, "The Storage Element is neither published as ClassicSE, SRM v1.1 nor v2.2");
		errno = EINVAL;
		rc = -1;
	}

	if (srm_endpoint != NULL) free (srm_endpoint);
	if (srmv1_endpoint != NULL) free (srmv1_endpoint);
	if (srmv2_endpoint != NULL) free (srmv2_endpoint);

	return (rc);
}

getfilemd (const char *surl, struct stat64 *statbuf, char *errbuf, int errbufsz, int timeout)
{
	int rc, i = 0;
	char **se_endpoints;
	char **se_types;
	char *srm_endpoint = NULL;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	int edgse = 0;
	int srm_v1 = 0;
	int srm_v2 = 0;

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "edg-se")) == 0) {
			edgse = 1;
			srm_endpoint = srm_endpoint == NULL ? strdup (se_endpoints[i]) : srm_endpoint;
		} else if ((strcmp (se_types[i], "srm_v1")) == 0) {
			srm_v1 = 1;
			srmv1_endpoint = srmv1_endpoint == NULL ? strdup (se_endpoints[i]) : srmv1_endpoint;
		} else if ((strcmp (se_types[i], "srm_v2")) == 0) {
                        srm_v2 = 1;
                        srmv2_endpoint = srmv2_endpoint == NULL ? strdup (se_endpoints[i]) : srmv2_endpoint;
                }
		free (se_types[i]);
		free (se_endpoints[i]);
		i++;
	}
	free (se_types);
	free (se_endpoints);

	if (srm_v1)
		rc = srm_getfilemde (surl, srmv1_endpoint, statbuf, errbuf, errbufsz, timeout);
	else if (srm_v2)
		rc = srmv2_getfilemd (surl, srmv2_endpoint, statbuf, NULL, errbuf, errbufsz, timeout);
	else if (edgse)
		rc = se_getfilemde (surl, srm_endpoint, statbuf, errbuf, errbufsz, timeout);
	else {
		gfal_errmsg(errbuf, errbufsz, "The Storage Element type is neither 'srm_v1' nor 'edg-se'.");
		errno = EINVAL;
		rc = -1;
	}

	if (srm_endpoint != NULL) free (srm_endpoint);
	if (srmv1_endpoint != NULL) free (srmv1_endpoint);
	if (srmv2_endpoint != NULL) free (srmv2_endpoint);

	return (rc);
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

#define SRM_EP_PATH "/srm/managerv1"
#define SRM_PORT 8443
parsesurl (const char *surl, char *endpoint, int srm_endpointsz, char **sfn,
	char *errbuf, int errbufsz)
{
	int len;
	int len1;
	int lenp;
	char *p;
	char *p1, *p2;
	char *se_endpoint;
	int se_port;

	if (strncmp (surl, "srm://", 6)) {
		gfal_errmsg(errbuf, errbufsz, "Source URL doesn't start with \"srm://\".");
		errno = EINVAL;
		return (-1);
	}
	if (p = strstr (surl + 6, "?SFN=")) {
		*sfn = p + 5;
		for (p1 = (char *)surl + 6; p1 < p; p1++)
			if (*p1 == '/') break;
	} else if (p = strchr (surl + 6, '/')) {
		*sfn = p;
		p1 = p;
	} else {
		gfal_errmsg(errbuf, errbufsz, "Bad Source URL syntax.");
		errno = EINVAL;
		return (-1);
	}
#ifdef GFAL_SECURE
	strcpy (endpoint, "https://");
	lenp = 8;
#else
	strcpy (endpoint, "http://");
	lenp = 7;
#endif
	/* copy hostname */

	len = p1 - surl - 6;
	if (lenp + len >= srm_endpointsz) {
		gfal_errmsg(errbuf, errbufsz, "Source URL too long.");
		errno = ENAMETOOLONG;
		return (-1);
	}
	strncpy (endpoint + lenp, surl + 6, len);
	*(endpoint + lenp + len) = '\0';

	if (p1 == p) {	/* no user specified endpoint */

		/* try to get endpoint from Information Service */

		if ((p2 = strchr (endpoint + lenp, ':')))
			*p2 = '\0';
		if (get_se_endpoint (endpoint + lenp, &se_endpoint) == 0) {
			strcpy (endpoint, se_endpoint);
			free (se_endpoint);
			return (0);
		} else
			if (p2)
				*p2 = ':';
	}

	/* set port number if not specified by user */

	if ((p2 = strchr (endpoint + lenp, ':')) == NULL) {	/* no port specified */
		if (get_se_port (endpoint + lenp, &se_port) < 0)
			se_port = SRM_PORT;
		if (lenp + len + 6 >= srm_endpointsz) {
			gfal_errmsg(errbuf, errbufsz, "Source URL too long");
			errno = ENAMETOOLONG;
			return (-1);
		}
		len1 = sprintf (endpoint + lenp + len, ":%d", se_port);
	} else
		len1 = 0;
	len1 += lenp + len;

	/* copy endpoint */

	if (p1 != p) {	/* user specified endpoint */
		if (len1 + (p - p1) >= srm_endpointsz) {
			gfal_errmsg(errbuf, errbufsz, "Source URL too long.");
			errno = ENAMETOOLONG;
			return (-1);
		}
		strncpy (endpoint + len1, p1, p - p1);
		*(endpoint + len1 + (p - p1)) = '\0';
	} else {
		if (len1 + strlen (SRM_EP_PATH) >= srm_endpointsz) {
			gfal_errmsg(errbuf, errbufsz, "Source URL too long.");
			errno = ENAMETOOLONG;
			return (-1);
		}
		strcpy (endpoint + len1, SRM_EP_PATH);
	}
	return (0);
}

parseturl (const char *turl, char *protocol, int protocolsz, char *pathbuf, int pathbufsz, char **pfn, char* errbuf, int errbufsz)
{
	int len;
	char *p;

	/* get protocol */

	if ((p = strstr (turl, ":/")) == NULL) {
		/* to enable 'file' protocol by default
		if (4 > (protocolsz - 1)) {
			gfal_errmsg(errbuf, errbufsz, "Destination URL too long.");
			errno = ENAMETOOLONG;
			return (-1);
		}
		sprintf (protocol, "file");
		*/
		gfal_errmsg(errbuf, errbufsz, "Invalid URL.");
		errno = ENAMETOOLONG;
		return (-1);
	} else if ((len = p - turl) > (protocolsz - 1)) {
		gfal_errmsg(errbuf, errbufsz, "Destination URL too long.");
		errno = ENAMETOOLONG;
		return (-1);
	} else {
		strncpy (protocol, turl, len);
		*(protocol + len) = '\0';
	}

	if (strcmp (protocol, "file") == 0) {
		if (p == NULL)
			strcpy (pathbuf, turl);
		else {
			++p;
			while (*(p + 1) == '/') ++p;
			strcpy (pathbuf, p);
		}
		*pfn = pathbuf;
	} else if (strcmp (protocol, "rfio") == 0) {
		p += 2;
		if (*p != '/') {
			gfal_errmsg(errbuf, errbufsz, "Bad destination URL syntax.");
			errno = EINVAL;
			return (-1);
		}
		p++;
		if (*p == '/' && *(p + 1) == '/') {	/* no hostname */
			*pfn = p + 1;
		} else {
			if (strlen (p) > pathbufsz) {
				gfal_errmsg(errbuf, errbufsz, "Destination URL too long.");
				errno = ENAMETOOLONG;
				return (-1);
			}
			strcpy (pathbuf, p);
			if (p = strstr (pathbuf, "//"))
				*p = ':';
			*pfn = pathbuf;
		}
	} else 
		*pfn = (char *) turl;
	return (0);
}

set_xfer_done (const char *surl, int reqid, int fileid, char *token, int oflag,
	char *errbuf, int errbufsz, int timeout)
{
	int rc, i = 0;
	char **se_endpoints;
	char **se_types;
	char *srm_endpoint = NULL;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	int edgse = 0;
	int srm_v1 = 0;
	int srm_v2 = 0;

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "edg-se")) == 0) {
			edgse = 1;
			srm_endpoint = srm_endpoint == NULL ? strdup (se_endpoints[i]) : srm_endpoint;
		} else if ((strcmp (se_types[i], "srm_v1")) == 0) {
			srm_v1 = 1;
			srmv1_endpoint = srmv1_endpoint == NULL ? strdup (se_endpoints[i]) : srmv1_endpoint;
		} else if ((strcmp (se_types[i], "srm_v2")) == 0) {
			srm_v2 = 1;
			srmv2_endpoint = srmv2_endpoint == NULL ? strdup (se_endpoints[i]) : srmv2_endpoint;
		}
		free (se_types[i]);
		free (se_endpoints[i]);
		i++;
	}
	free (se_types);
	free (se_endpoints);

	/* if token specified  or SRM v2,2 supported only */
	if (((token != NULL) && srm_v2) || (!srm_v1 && srm_v2)) {
		if ((oflag & O_ACCMODE) == 0)
			rc = srmv2_set_xfer_done_get (surl, token, srmv2_endpoint, errbuf, errbufsz, timeout);
		else
			rc = srmv2_set_xfer_done_put (surl, token, srmv2_endpoint, errbuf, errbufsz, timeout);
	} else if (srm_v1) {
		rc = srm_set_xfer_donee (surl, srmv1_endpoint, reqid, fileid, token, oflag,
			errbuf, errbufsz, timeout);
	} else if (edgse) {
		rc = se_set_xfer_donee (surl, srm_endpoint, reqid, fileid, token, oflag,
		    errbuf, errbufsz, timeout);
	} else {
		gfal_errmsg(errbuf, errbufsz, "The Storage Element type is neither published as Classic SE nor SRM (v1.1 or v2.2).");
		errno = EINVAL;
		rc = -1;
	}

	if (srm_endpoint != NULL) free (srm_endpoint);
	if (srmv1_endpoint != NULL) free (srmv1_endpoint);
	if (srmv2_endpoint != NULL) free (srmv2_endpoint);

	return (rc);
}

set_xfer_running (const char *surl, int reqid, int fileid, char *token,
	char *errbuf, int errbufsz, int timeout)
{
	int rc, i = 0;
	char **se_endpoints;
	char **se_types;
	char *srm_endpoint = NULL;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	int edgse = 0;
	int srm_v1 = 0;
	int srm_v2 = 0;

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "edg-se")) == 0) {
			edgse = 1;
			srm_endpoint = srm_endpoint == NULL ? strdup (se_endpoints[i]) : srm_endpoint;
		} else if ((strcmp (se_types[i], "srm_v1")) == 0) {
			srm_v1 = 1;
			srmv1_endpoint = srmv1_endpoint == NULL ? strdup (se_endpoints[i]) : srmv1_endpoint;
		} else if ((strcmp (se_types[i], "srm_v2")) == 0) {
			srm_v2 = 1;
			srmv2_endpoint = srmv2_endpoint == NULL ? strdup (se_endpoints[i]) : srmv2_endpoint;
		}
		free (se_types[i]);
		free (se_endpoints[i]);
		i++;
	}
	free (se_types);
	free (se_endpoints);

	/* if token specified  or SRM v2,2 supported only */
	if (((token != NULL) && (srm_v1 && srm_v2)) || (!srm_v1 && srm_v2)) {
		rc = srmv2_set_xfer_running (surl, srmv2_endpoint, token, errbuf, errbufsz, timeout);
	} else if (srm_v1) {
		rc = srm_set_xfer_runninge (surl, srmv1_endpoint, reqid, fileid, token, errbuf, errbufsz, timeout);
	} else if (edgse) {
		rc = se_set_xfer_running (surl, reqid, fileid, token, errbuf, errbufsz);
	} else {
		gfal_errmsg(errbuf, errbufsz, "The Storage Element type is neither published as Classic SE nor SRM (v1.1 or v2.2).");
		errno = EINVAL;
		rc = -1;
	}

	if (srm_endpoint != NULL) free (srm_endpoint);
	if (srmv1_endpoint != NULL) free (srmv1_endpoint);
	if (srmv2_endpoint != NULL) free (srmv2_endpoint);

	return (rc);
}

setypefromsurl (const char *surl, char **se_type,
	char *errbuf, int errbufsz)
{
	int len;
	char *p;
	char server[256];

	if ((p = strchr (surl + 6, '/')) == NULL) {
		gfal_errmsg(errbuf, errbufsz, "Bad source URL syntax.");
		errno = EINVAL;
		return (-1);
	}
	if ((len = p - surl - 6) >= sizeof(server)) {
		gfal_errmsg(errbuf, errbufsz, "Host name too long.");
		errno = ENAMETOOLONG;
		return (-1);
	}
	strncpy (server, surl + 6, len);
	server[len] = '\0';
	if ((p = strchr (server, ':'))) *p = '\0';
	return (get_se_typex (server, se_type, errbuf, errbufsz));
}

setypesandendpoints (const char *endpoint, char ***se_types, char ***se_endpoints, char *errbuf, int errbufsz)
{
	int rc;
	int len;
	char *p1, *p2;
	char endpoint_tmp[256];

	if (se_types == NULL) {
       		errno = EINVAL;
	       	return (-1);
	}
	if (strlen (endpoint) + 2 >= sizeof (endpoint_tmp)) {
		gfal_errmsg(errbuf, errbufsz, "Source URL too long.");
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

	if (get_srm_types_and_endpoints (endpoint_tmp, se_types, se_endpoints, errbuf, errbufsz) == 0)
		return (0);

	/* Maybe BDII uses old Glue Schema... */
	if ((p1 = strchr (endpoint_tmp, '/')) != NULL)
		*p1 = 0;

	if ((*se_types = (char**) calloc (2, sizeof (char*))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	(*se_types)[1] = NULL;
	if (se_endpoints) {
	        if ((*se_endpoints = (char**) calloc (2, sizeof (char*))) == NULL) {
	                errno = ENOMEM;
        	        return (-1);
	        }
		(*se_endpoints)[1] = NULL;
		return (get_se_typeandendpoint (endpoint_tmp, *se_types, *se_endpoints, errbuf, errbufsz));
	} else {
		return (get_se_typeandendpoint (endpoint_tmp, *se_types, NULL, errbuf, errbufsz));
	}
}

setypesandendpointsfromsurl (const char *surl, char ***se_types, char ***se_endpoints, char *errbuf, int errbufsz)
{
	int len;
	char *p;
	char endpoint_tmp[256];

	if (strncmp (surl, "srm://", 6) && strncmp (surl, "sfn://", 6)) {
		gfal_errmsg(errbuf, errbufsz, "Source URL starts neither with \"srm://\" nor \"sfn://\".");
		errno = EINVAL;
		return (-1);
	}

	p = strstr (surl + 6, "?SFN=");
	p = p == NULL ? strchr (surl + 6, '/') : p;

	if (p == NULL) {
		gfal_errmsg(errbuf, errbufsz, "Bad Source URL syntax.");
		errno = EINVAL;
		return (-1);
	}

	if ((len = p - surl - 6) >= sizeof (endpoint_tmp)) {
		gfal_errmsg(errbuf, errbufsz, "Source URL too long.");
		errno = ENAMETOOLONG;
		return (-1);
	}
	strncpy (endpoint_tmp, surl + 6, len);
	endpoint_tmp[len] = 0;

	return (setypesandendpoints (endpoint_tmp, se_types, se_endpoints, errbuf, errbufsz));
}

char *
turlfromsfn (const char *sfn, char **protocols, char *errbuf, int errbufsz)
{
	char **ap;
	int i;
	int len;
	char *p;
	int *pn;
	int port = 0;
	char **protoarray;
	char server[64];
	char *turl;

	if (strncmp (sfn, "sfn://", 6)) {
		gfal_errmsg(errbuf, errbufsz, "File doesn't start with \"sfn://\".");
		errno = EINVAL;
		return (NULL);
	}
	if ((p = strchr (sfn + 6, '/')) == NULL ||
	    (len = p - (sfn + 6)) > sizeof(server)) {
		gfal_errmsg(errbuf, errbufsz, "Host name too long.");
		errno = ENAMETOOLONG;
		return (NULL);
	}

	/* check that RFIO library is available */

	if (protocols == NULL) {
		protoarray = get_sup_proto ();
	} else
		protoarray = protocols;
	for (i = 0; *protoarray[i]; i++)
		if (strcmp (protoarray[i], "rfio") == 0) break;
	if (*protoarray[i] == '\0') {
		errno = EPROTONOSUPPORT;
		return (NULL);
	}

	/* check that the SE supports RFIO */

	strncpy (server, sfn + 6, len);
	*(server + len) = '\0';
	if (get_seap_infox (server, &ap, &pn, errbuf, errbufsz) < 0)
		return (NULL);
	i = 0;
	while (ap[i]) {
		if (strcmp (ap[i], "rfio") == 0) port = pn[i];
		free (ap[i]);
		i++;
	}
	free (ap);
	free (pn);
	if (! port) {
		gfal_errmsg(errbuf, errbufsz, "rfio protocol not supported by Storage Element.");
		errno = EPROTONOSUPPORT;
		return (NULL);
	}
	if ((turl = malloc (strlen (sfn) + 2)) == NULL)
		return (NULL);
	strcpy (turl, "rfio");
	strcpy (turl + 4, sfn + 3);
	return (turl);
}

char *
turlfromsurl2 (const char *surl, GFAL_LONG64 filesize, const char *spacetokendesc, char **protocols,
	int oflag, int *reqid, int *fileid, char **token, char *errbuf, int errbufsz, int timeout)
{
	char *p;
	char **explanations;
	int *fileids;
	int i = 0;
	char **se_endpoints;
	char **se_types;
	char *srm_endpoint = NULL;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	int edgse = 0;
	int srm_v1 = 0;
	int srm_v2 = 0;
	int *statuses;
	char **turls;
	char **sourcesurls;

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (NULL);

	while (se_types[i]) {
		if ((strcmp (se_types[i], "edg-se")) == 0) {
			edgse = 1;
			srm_endpoint = srm_endpoint == NULL ? strdup (se_endpoints[i]) : srm_endpoint;
		} else if ((strcmp (se_types[i], "srm_v1")) == 0) {
			srm_v1 = 1;
			srmv1_endpoint = srmv1_endpoint == NULL ? strdup (se_endpoints[i]) : srmv1_endpoint;
		} else if ((strcmp (se_types[i], "srm_v2")) == 0) {
			srm_v2 = 1;
			srmv2_endpoint = srmv2_endpoint == NULL ? strdup (se_endpoints[i]) : srmv2_endpoint;
		}
		free (se_types[i]);
		free (se_endpoints[i]);
		i++;
	}
	free (se_types);
	free (se_endpoints);

	/* if spacetokendesc specified by user and/or SRM v2.2 supported */
	if (((spacetokendesc != NULL) && srm_v2) || (!srm_v1 && srm_v2)) {
		if ((oflag & O_ACCMODE) == 0) {
			if (srmv2_turlsfromsurls_get (1, &surl, srmv2_endpoint, &filesize, spacetokendesc, protocols,
			    token, &sourcesurls, &turls, &statuses, &explanations, errbuf, errbufsz, timeout) <= 0) {
				free (srm_endpoint);
				return NULL;
			}
		} else {
			if ((srmv2_turlsfromsurls_put (1, &surl, srmv2_endpoint, &filesize, spacetokendesc, protocols,
			     token, &sourcesurls, &turls, &statuses, &explanations, errbuf, errbufsz, timeout)) <=0) {
				free (srm_endpoint);
				return NULL;
			}
		}
		p = turls[0];
		if (explanations[0]) {
			gfal_errmsg (errbuf, errbufsz, explanations[0]);
		}
		free (srm_endpoint);
		free (sourcesurls); 
		free (turls);
		free (statuses);
		free (explanations);
	}  else	if (srm_v1) { /* if SRM v1.1 supported */
		if (srm_turlsfromsurlse (1, &surl, srmv1_endpoint, &filesize, protocols, oflag,
		    reqid, &fileids, token, &turls, errbuf, errbufsz, timeout) <= 0)
			return (NULL);
		*fileid = fileids[0];
		p = turls[0];
		free (fileids);
		free (turls);
	} else if (edgse) {
		p = se_turlfromsurle (surl, srm_endpoint, protocols, oflag, reqid, fileid,
		    			token, errbuf, errbufsz, timeout);
	} else {
		gfal_errmsg(errbuf, errbufsz, "The Storage Element type is neither published as Classic SE nor SRM (v1.1 or v2.2).");
		errno = EINVAL;
		p = NULL;
	}

	if (srm_endpoint != NULL) free (srm_endpoint);
	if (srmv1_endpoint != NULL) free (srmv1_endpoint);
	if (srmv2_endpoint != NULL) free (srmv2_endpoint);
	return (p);
}

char *
turlfromsurlx (const char *surl, GFAL_LONG64 filesize, char **protocols, int oflag, int *reqid,
	int *fileid, char **token, char *errbuf, int errbufsz, int timeout)
{
	GFAL_LONG64 zero = 0;

	return (turlfromsurl2 (surl, zero, NULL, protocols, oflag, reqid, fileid, 
		token, errbuf, errbufsz, timeout));
}

char *
turlfromsurl (const char *surl, char **protocols, int oflag, int *reqid,
	int *fileid, char **token, char *errbuf, int errbufsz, int timeout)
{
	GFAL_LONG64 zero = 0;

	return (turlfromsurl2 (surl, zero, NULL, protocols, oflag, reqid, fileid,
	    token, errbuf, errbufsz, timeout));
}

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

getfilesizeg (const char *guid, GFAL_LONG64 *filesize, char *errbuf, int errbufsz)
{
	char *cat_type;
	if (get_cat_type (&cat_type) < 0) {
		return (-1);
	}
	if (strcmp (cat_type, "edg") == 0) {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The EDG catalog doesn't support the getfilesizeg() method.");
		errno = EINVAL;
	} else if (strcmp (cat_type, "lfc") == 0) {
		free (cat_type);
		return (lfc_getfilesizeg (guid, filesize, errbuf, errbufsz));
	} else {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (-1);
	}
}

char *get_catalog_endpoint (char *errbuf, int errbufsz)
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
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (NULL);
	}
}

char *
guidforpfn (const char *pfn, char *errbuf, int errbufsz)
{
	char *cat_type;
	if (get_cat_type (&cat_type) < 0) {
		return (NULL);
	}
	if (strcmp (cat_type, "edg") == 0) {
		free (cat_type);
		return (lrc_guidforpfn (pfn, errbuf, errbufsz));
	} else if (strcmp (cat_type, "lfc") == 0) {
		free (cat_type);
		return (lfc_guidforpfn (pfn, errbuf, errbufsz));
	} else {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (NULL);
	}
}

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
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (-1);
	}
}

register_pfn (const char *guid, const char *pfn, char *errbuf, int errbufsz)
{
	char *cat_type;
	if (get_cat_type (&cat_type) < 0) {
		return (-1);
	}
	if (strcmp (cat_type, "edg") == 0) {
		free (cat_type);
		return (lrc_register_pfn (guid, pfn, errbuf, errbufsz));
	} else if (strcmp (cat_type, "lfc") == 0) {
		free (cat_type);
		return (lfc_register_pfn (guid, pfn, errbuf, errbufsz));
	} else {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (-1);
	}
}

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
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (-1);
	}
}

char *
surlfromguid (const char *guid, char *errbuf, int errbufsz)
{
	char *cat_type;
	if (get_cat_type (&cat_type) < 0) {
		return (NULL);
	}
	if (strcmp (cat_type, "edg") == 0) {
		free (cat_type);
		return (lrc_surlfromguid (guid, errbuf, errbufsz));
	} else if (strcmp (cat_type, "lfc") == 0) {
		free (cat_type);
		return (lfc_surlfromguid (guid, errbuf, errbufsz));
	} else {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (NULL);
	}
}

char **
surlsfromguid (const char *guid, char *errbuf, int errbufsz)
{
	char *cat_type;
	if (get_cat_type (&cat_type) < 0) {
		return (NULL);
	}
	if (strcmp (cat_type, "edg") == 0) {
		free (cat_type);
		return (lrc_surlsfromguid (guid, errbuf, errbufsz));
	} else if (strcmp (cat_type, "lfc") == 0) {
		free (cat_type);
		return (lfc_surlsfromguid (guid, errbuf, errbufsz));
	} else {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (NULL);
	}
}

unregister_pfn (const char *guid, const char *pfn, char *errbuf, int errbufsz)
{
	char *cat_type;
	if (get_cat_type (&cat_type) < 0) {
		return (-1);
	}
	if (strcmp (cat_type, "edg") == 0) {
		free (cat_type);
		return (lrc_unregister_pfn (guid, pfn, errbuf, errbufsz));
	} else if (strcmp (cat_type, "lfc") == 0) {
		free (cat_type);
		return (lfc_unregister_pfn (guid, pfn, errbuf, errbufsz));
	} else {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
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
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (NULL);
	}
}

char **
lfnsforguid (const char *guid, char *errbuf, int errbufsz)
{
	char *cat_type;
	if (get_cat_type (&cat_type) < 0) {
		return (NULL);
	}
	if (strcmp (cat_type, "edg") == 0) {
		free (cat_type);
		return (rmc_lfnsforguid (guid, errbuf, errbufsz));
	} else if (strcmp (cat_type, "lfc") == 0) {
		free (cat_type);
		return (lfc_lfnsforguid (guid, errbuf, errbufsz));
	} else {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (NULL);
	}
}

int
replica_exists(const char* guid, char *errbuf, int errbufsz) 
{
	char *cat_type;
	if (get_cat_type (&cat_type) < 0) {
		return (-1);
	}
	if (strcmp (cat_type, "edg") == 0) {
		free (cat_type);
		return (lrc_replica_exists (guid, errbuf, errbufsz));
	} else if (strcmp (cat_type, "lfc") == 0) {
		free (cat_type);
		return (lfc_replica_exists (guid, errbuf, errbufsz));
	} else {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (-1);
	}
}

create_alias (const char *guid, const char *lfn, GFAL_LONG64 size, char *errbuf,
	      int errbufsz)
{
	return create_alias_m (guid, lfn, 0666, size, errbuf, errbufsz);
}

create_alias_m (const char *guid, const char *lfn, mode_t mode, GFAL_LONG64 size,
		char *errbuf, int errbufsz)
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
		return (lfc_create_alias (guid, lfn, mode, size, errbuf, errbufsz));
	} else {
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (-1);
	}
}

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
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
		errno = EINVAL;
		return (-1);
	}
}

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
		gfal_errmsg(errbuf, errbufsz, "The catalog type is neither 'edg' nor 'lfc'.");
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
getbestfile(char **surls, int size, char *errbuf, int errbufsz)
{
  char dname[64];
  int first;
  int i;
  char  *p1, *p2;
  char *p;
  int ret;
  char *default_se;
  int  localsurl, default_match;

  /* skip entries not in the form srm: or sfn:
   * take entry on same domain if it exists else
   * take the first supported entry
   */
  first = -1;
  localsurl = -1;
  *dname = '\0';
  (void) getdomainnm (dname, sizeof(dname));

  /* and get the default SE, it there is one */
  if((default_se = get_default_se(NULL, errbuf, errbufsz)) == NULL) 
	  return (NULL);

  for (i = 0; i < size; i++) {
    p = surls[i];
    if (strncmp (p, "srm://", 6) && strncmp (p, "sfn://", 6))
      continue;
    if ((p1 = strchr (p + 6, '/')) == NULL) continue;
    *p1 = '\0';
    if ((p2 = strchr (p + 6, ':')))
      *p2 = '\0';
    if (first < 0) first = i;
    default_match = -1;
    if(default_se != NULL) {
      default_match = strcmp(p + 6, default_se);
    }
    if ((p = strchr (p + 6, '.')) == NULL) continue;
    ret = strcmp (p + 1, dname);
    *p1 = '/';
    if (p2) *p2 = ':';
    if (default_match == 0) break; /* default se match => replica on default SE */
    if (ret == 0) localsurl = i;	/* domains match ==> local replica */
  }
  if (i == size) {	/* no default SE entry */
    if (first < 0) {	/* only non suported entries */
      gfal_errmsg(errbuf, errbufsz, "Only non supported entries. No replica entry starting with \"srm://\" or \"sfn://\".");
      errno = EINVAL;
      return (NULL);
    } else if(localsurl >= 0) {
      i = localsurl;
    } else {
      /* seed with current time */
      srand( (unsigned)time( NULL ) );
      i = (int)((double)size * rand()/(RAND_MAX));
    }
  }
  return surls[i];
}


char *
get_default_se(char *vo, char *errbuf, int errbufsz) 
{
	char *default_se;
	int i;
	char se_env[64];
	char *vo_env;
	char error_str[128];

	if(vo == NULL) {
		if ((vo_env = getenv ("LCG_GFAL_VO")) == NULL) {
			gfal_errmsg (errbuf, errbufsz, "LCG_GFAL_VO not set");
			errno = EINVAL;
			return (NULL);
		}
		vo = vo_env;
	}
	if(strlen(vo) + 15 >= 64) {
		errno = ENAMETOOLONG;
		gfal_errmsg(errbuf, errbufsz, "VO Name too long");
		return (NULL);
	}
	sprintf(se_env, "VO_%s_DEFAULT_SE", vo);
	for(i = 3; i < 3 + strlen(vo); ++i) {
		if (se_env[i] == '.' || se_env[i] == '-') 
			se_env[i] = '_';
		else
			se_env[i] = toupper(se_env[i]);
	} 
	default_se = getenv(se_env);
	if(default_se == NULL) {
		snprintf(error_str, 128, "No Default SE: %s not set", se_env);
		gfal_errmsg(errbuf, errbufsz, error_str);
		errno = EINVAL;
		return (NULL);
	}
	return default_se;
}

int
purify_surl (const char *surl, char *surl_cat, const int surl_cat_sz) {
	char *p,*q,*r;
	char tmp[1104];
	int l;

	if (surl == NULL || surl_cat == NULL || surl_cat_sz < strlen (surl)) {
		errno = EINVAL;
		return (-1);
	}
	if (strncmp (surl, "srm://", 6)) {
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
