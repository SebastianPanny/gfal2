/*
 * Copyright (C) 2003-2004 by CERN
 */

/*
 * @(#)$RCSfile: gfal.c,v $ $Revision: 1.6 $ $Date: 2004/04/19 09:10:11 $ CERN Jean-Philippe Baud
 */

#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#include "gfal.h"
#include "gfal_api.h"
#if !defined(OFF_MAX)
#define OFF_MAX 2147483647
#endif

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

static int
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
	char *pfn;
	struct proto_ops *pops;
	char *protocol;

	if (strncmp (path, "lfn:", 4) == 0 ||
	    strncmp (path, "guid:", 5) == 0 ||
	    strncmp (path, "srm:", 4) == 0 ||
	    strncmp (path, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	if (parseturl (path, &protocol, &pfn) < 0)
		return (-1);
	if ((pops = find_pops (protocol)) == NULL)
		return (-1);
	if (pops->access (pfn, amode) < 0) {
		errno = pops->maperror (pops, 0);
		return (-1);
	}
	return (0);
}

gfal_chmod (const char *path, mode_t mode)
{
	char *pfn;
	struct proto_ops *pops;
	char *protocol;

	if (strncmp (path, "lfn:", 4) == 0 ||
	    strncmp (path, "guid:", 5) == 0 ||
	    strncmp (path, "srm:", 4) == 0 ||
	    strncmp (path, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	if (parseturl (path, &protocol, &pfn) < 0)
		return (-1);
	if ((pops = find_pops (protocol)) == NULL)
		return (-1);
	if (pops->chmod (pfn, mode) < 0) {
		errno = pops->maperror (pops, 0);
		return (-1);
	}
	return (0);
}

gfal_close (int fd)
{
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
		    xi->oflag);
		free (xi->surl);
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

	if (di = find_di (dir)) {
		if ((rc = di->pops->closedir (dir)) < 0)
			errno = di->pops->maperror (di->pops, 0);
		(void) free_di (di);
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

off_t
gfal_lseek (int fd, off_t offset, int whence)
{
	off_t offset_out;
	struct xfer_info *xi;

	if ((xi = find_xi (fd)) == NULL)
		return (-1);
	if ((offset_out = xi->pops->lseek (fd, offset, whence)) < 0)
		errno = xi->pops->maperror (xi->pops, 1);
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
	return (offset_out);
}

gfal_lstat (const char *filename, struct stat *statbuf)
{
	char *fn;
	char *guid;
	char *pfn;
	struct proto_ops *pops;
	char *protocol;
	int rc;
	struct stat64 statb64;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4)) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid)) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5)) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		if ((rc = getfilemd (fn, &statb64)) == 0)
			rc = mdtomd32 (&statb64, statbuf);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL)) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, &protocol, &pfn)) == 0) {
		if ((pops = find_pops (protocol)) != NULL) {
			if ((rc = pops->lstat (pfn, statbuf)) < 0)
				errno = pops->maperror (pops, 0);
		}
	}
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	if (rc < 0 || pops == NULL)
		return (-1);
	return (0);
}

gfal_lstat64 (const char *filename, struct stat64 *statbuf)
{
	char *fn;
	char *guid;
	char *pfn;
	struct proto_ops *pops;
	char *protocol;
	int rc;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4)) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid)) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5)) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		rc = getfilemd (fn, statbuf);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL)) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, &protocol, &pfn)) == 0) {
		if ((pops = find_pops (protocol)) != NULL) {
			if ((rc = pops->lstat64 (pfn, statbuf)) < 0)
				errno = pops->maperror (pops, 0);
		}
	}
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	if (rc < 0 || pops == NULL)
		return (-1);
	return (0);
}

gfal_mkdir (const char *dirname, mode_t mode)
{
	char *pfn;
	struct proto_ops *pops;
	char *protocol;

	if (strncmp (dirname, "lfn:", 4) == 0 ||
	    strncmp (dirname, "guid:", 5) == 0 ||
	    strncmp (dirname, "srm:", 4) == 0 ||
	    strncmp (dirname, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	if (parseturl (dirname, &protocol, &pfn) < 0)
		return (-1);
	if ((pops = find_pops (protocol)) == NULL)
		return (-1);
	if (pops->mkdir (pfn, mode) < 0) {
		errno = pops->maperror (pops, 0);
		return (-1);
	}
	return (0);
}

gfal_open (const char *filename, int flags, mode_t mode)
{
	int fd;
	int fileid;
	char *fn;
	char *guid = NULL;
	char *pfn;
	struct proto_ops *pops;
	char *protocol;
	int reqid;
	char **supported_protocols;
	char *turl = NULL;
	struct xfer_info *xi;

	supported_protocols = get_sup_proto ();

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4)) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid)) == NULL) {
			free (guid);
			return (-1);
		}
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5)) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		if ((turl = turlfromsurl (fn, supported_protocols, flags,
		    &reqid, &fileid)) == NULL)
			goto err;
	} else if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, supported_protocols)) == NULL)
			goto err;
	} else		/* assume that is a pfn */
		turl = fn;
	if (parseturl (turl, &protocol, &pfn) < 0)
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
	xi->oflag = flags;
	xi->pops = pops;
	if (strncmp (fn, "srm:", 4) == 0) {
		xi->surl = strdup (fn);
		xi->reqid = reqid;
		xi->fileid = fileid;
		(void) set_xfer_running (fn, reqid, fileid);
	}

	if (guid) free (guid);
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	return (fd);
err:
	if (guid) free (guid);
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
	char *pfn;
	struct proto_ops *pops;
	char *protocol;

	if (strncmp (dirname, "lfn:", 4) == 0 ||
	    strncmp (dirname, "guid:", 5) == 0 ||
	    strncmp (dirname, "srm:", 4) == 0 ||
	    strncmp (dirname, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (NULL);
	}
	if (parseturl (dirname, &protocol, &pfn) < 0)
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
	return (de);
}

gfal_rename (const char *old_name, const char *new_name)
{
	char *new_pfn;
	char *old_pfn;
	struct proto_ops *pops;
	char *protocol1;
	char *protocol2;

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
	if (parseturl (old_name, &protocol1, &old_pfn) < 0)
		return (-1);
	if (parseturl (new_name, &protocol2, &new_pfn) < 0)
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
	return (0);
}

gfal_rmdir (const char *dirname)
{
	char *pfn;
	struct proto_ops *pops;
	char *protocol;

	if (strncmp (dirname, "lfn:", 4) == 0 ||
	    strncmp (dirname, "guid:", 5) == 0 ||
	    strncmp (dirname, "srm:", 4) == 0 ||
	    strncmp (dirname, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	if (parseturl (dirname, &protocol, &pfn) < 0)
		return (-1);
	if ((pops = find_pops (protocol)) == NULL)
		return (-1);
	if (pops->rmdir (pfn) < 0) {
		errno = pops->maperror (pops, 0);
		return (-1);
	}
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
	return (rc);
}

gfal_stat (const char *filename, struct stat *statbuf)
{
	char *fn;
	char *guid;
	char *pfn;
	struct proto_ops *pops;
	char *protocol;
	int rc;
	struct stat64 statb64;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4)) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid)) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5)) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		if ((rc = getfilemd (fn, &statb64)) == 0)
			rc = mdtomd32 (&statb64, statbuf);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL)) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, &protocol, &pfn)) == 0) {
		if ((pops = find_pops (protocol)) != NULL) {
			if ((rc = pops->stat (pfn, statbuf)) < 0)
				errno = pops->maperror (pops, 0);
		}
	}
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	if (rc < 0 || pops == NULL)
		return (-1);
	return (0);
}

gfal_stat64 (const char *filename, struct stat64 *statbuf)
{
	char *fn;
	char *guid;
	char *pfn;
	struct proto_ops *pops;
	char *protocol;
	int rc;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4)) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid)) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5)) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		rc = getfilemd (fn, statbuf);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL)) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, &protocol, &pfn)) == 0) {
		if ((pops = find_pops (protocol)) != NULL) {
			if ((rc = pops->stat64 (pfn, statbuf)) < 0)
				errno = pops->maperror (pops, 0);
		}
	}
	if (fn != filename) free (fn);
	if (turl != fn) free (turl);
	if (rc < 0 || pops == NULL)
		return (-1);
	return (0);
}

gfal_unlink (const char *filename)
{
	int i = 0;
	char **pfns;
	int rc = 0;

	if (strncmp (filename, "lfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		/* must try to delete all PFNs mapped to this guid */
		if ((pfns = surlsfromguid (filename + 5)) == NULL)
			return (-1);
		while (pfns[i]) {
			rc += deletepfn (pfns[i], filename + 5);
			free (pfns[i++]);
		}
		free (pfns);
		return (rc == 0 ? 0 : -1);
	} else
		return (deletepfn (filename, NULL));
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
	return (rc);
}

deletepfn (const char *fn, const char *guid)
{
	char *pfn;
	struct proto_ops *pops;
	char *protocol;
	int rc;
	char *turl;

	if (strncmp (fn, "srm:", 4) == 0) {
		if (deletesurl (fn) < 0)
			return (-1);
	} else {
		if (strncmp (fn, "sfn:", 4) == 0) {
			if ((turl = turlfromsfn (fn, NULL)) == NULL)
				return (-1);
		} else		/* assume that is a pfn */
			turl = (char *)fn;
		if ((rc = parseturl (turl, &protocol, &pfn)) == 0) {
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
parsesurl (const char *surl, char **endpoint, char **sfn)
{
	int len;
	int lenp;
	char *p;
	static char srm_ep[256];

	if (strncmp (surl, "srm://", 6)) {
		errno = EINVAL;
		return (-1);
	}
	if (p = strstr (surl + 6, "?SFN=")) {
		*sfn = p + 5;
	} else if (p = strchr (surl + 6, '/')) {
		*sfn = p;
	} else {
		errno = EINVAL;
		return (-1);
	}
#ifdef GFAL_SECURE
	strcpy (srm_ep, "https://");
	lenp = 8;
#else
	strcpy (srm_ep, "http://");
	lenp = 7;
#endif
	len = p - surl - 6;
	if (lenp + len >= sizeof(srm_ep)) {
		errno = EINVAL;
		return (-1);
	}
	strncpy (srm_ep + lenp, surl + 6, len);
	*(srm_ep + lenp + len) = '\0';
	if (strchr (srm_ep + lenp, '/') == NULL) {
		if (strlen (SRM_EP_PATH) + lenp + len >= sizeof(srm_ep)) {
			errno = EINVAL;
			return (-1);
		}
		strcat (srm_ep, SRM_EP_PATH);
	}
	*endpoint = srm_ep;
	return (0);
}

parseturl (const char *turl, char **protocol, char **pfn)
{
	int len;
	char *p;
	static char path[1024];
	static char proto[64];

	/* get protocol */

	if ((p = strstr (turl, ":/")) == NULL) {
		errno = EINVAL;
		return (-1);
	}
	if ((len = p - turl) > (sizeof(proto) - 1)) {
		errno = EINVAL;
		return (-1);
	}
	strncpy (proto, turl, len);
	*(proto + len) = '\0';

	if (strcmp (proto, "file") == 0) {
		*pfn = p + 1;
	} else if (strcmp (proto, "rfio") == 0) {
		p += 2;
		if (*p != '/') {
			errno = EINVAL;
			return (-1);
		}
		p++;
		if (*p == '/' && *(p + 1) == '/') {	/* no hostname */
			*pfn = p + 1;
		} else {
			if (strlen (p) > sizeof(path)) {
				errno = EINVAL;
				return (-1);
			}
			strcpy (path, p);
			if (p = strstr (path, "//"))
				*p = ':';
			*pfn = path;
		}
	} else 
		*pfn = (char *) turl;
	*protocol = proto;
	return (0);
}

char *
turlfromsfn (const char *sfn, char **protocols)
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
		errno = EINVAL;
		return (NULL);
	}
	if ((p = strchr (sfn + 6, '/')) == NULL ||
	    (len = p - (sfn + 6)) > sizeof(server)) {
		errno = EINVAL;
		return (NULL);
	}

	/* check that RFIO library is available */

	if (protocols == NULL)
		protoarray = get_sup_proto ();
	else
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
	if (get_seap_info (server, &ap, &pn) < 0)
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
		errno = EPROTONOSUPPORT;
		return (NULL);
	}
	if ((turl = malloc (strlen (sfn) + 2)) == NULL)
		return (NULL);
	strcpy (turl, "rfio");
	strcpy (turl + 4, sfn + 3);
	return (turl);
}
