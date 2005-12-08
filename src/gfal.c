/*
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: gfal.c,v $ $Revision: 1.24 $ $Date: 2005/12/08 16:10:13 $ CERN Jean-Philippe Baud
 */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
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
	char errbuf[256];
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];

	if (strncmp (path, "lfn:", 4) == 0 ||
	    strncmp (path, "guid:", 5) == 0 ||
	    strncmp (path, "srm:", 4) == 0 ||
	    strncmp (path, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	if (parseturl (path, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf)) < 0)
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
        char errbuf[256];
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];

	if (strncmp (path, "lfn:", 4) == 0 ||
	    strncmp (path, "guid:", 5) == 0 ||
	    strncmp (path, "srm:", 4) == 0 ||
	    strncmp (path, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}
	if (parseturl (path, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, sizeof(errbuf)) < 0)
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
	return (0);
}

gfal_mkdir (const char *dirname, mode_t mode)
{
	char errbuf[256];
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];

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
	if (pops->mkdir (pfn, mode) < 0) {
		errno = pops->maperror (pops, 0);
		return (-1);
	}
	return (0);
}

gfal_open (const char *filename, int flags, mode_t mode)
{
	char errbuf[256];
	int fd;
	int fileid;
	char *fn;
	char *guid = NULL;
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];
	int reqid;
	char **supported_protocols;
	char *token = NULL;
	char *turl = NULL;
	struct xfer_info *xi;

	supported_protocols = get_sup_proto ();

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid, errbuf, sizeof(errbuf))) == NULL) {
			free (guid);
			return (-1);
		}
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		if ((turl = turlfromsurl (fn, supported_protocols, flags,
		    &reqid, &fileid, &token, errbuf, sizeof(errbuf), 0)) == NULL)
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
	char errbuf[256];
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];

	if (strncmp (dirname, "lfn:", 4) == 0 ||
	    strncmp (dirname, "guid:", 5) == 0 ||
	    strncmp (dirname, "srm:", 4) == 0 ||
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
	char errbuf[256];
	char pathbuf1[1024];
	char pathbuf2[1024];
	struct proto_ops *pops;
	char protocol1[64];
	char protocol2[64];

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
	return (0);
}

gfal_rmdir (const char *dirname)
{
	char errbuf[256];
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];

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
	return (0);
}

gfal_unlink (const char *filename)
{
	char errbuf[256];
	int i = 0;
	char **pfns;
	int rc = 0;

	if (strncmp (filename, "lfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		/* must try to delete all PFNs mapped to this guid */
		if ((pfns = surlsfromguid (filename + 5, errbuf, sizeof(errbuf))) == NULL)
			return (-1);
		while (pfns[i]) {
			rc += deletepfn (pfns[i], filename + 5, errbuf, sizeof(errbuf));
			free (pfns[i++]);
		}
		free (pfns);
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
		if (deletesurl (fn, errbuf, errbufsz, 0) < 0)
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
	char *se_type;

	if (setypefromsurl (surl, &se_type, errbuf, errbufsz) < 0)
		return (-1);
	if (strcmp (se_type, "srm_v1") == 0) {
		free (se_type);
		return (srm_deletesurl (surl, errbuf, errbufsz, timeout));
	} else if (strcmp (se_type, "edg-se") == 0) {
		free (se_type);
		return (se_deletesurl (surl, errbuf, errbufsz, timeout));
	} else {
		free (se_type);
		gfal_errmsg(errbuf, errbufsz, "The Storage Element type is neither 'srm_v1' nor 'edg-se'.");
		errno = EINVAL;
		return (-1);
	}
}

getfilemd (const char *surl, struct stat64 *statbuf, char *errbuf, int errbufsz, int timeout)
{
	char *se_type;
	int ret;
	ret=setypefromsurl (surl, &se_type, errbuf, errbufsz);
	if (ret < 0)
		return (-1);
	if (strcmp (se_type, "srm_v1") == 0) {
		free (se_type);
		return (srm_getfilemd (surl, statbuf, errbuf, errbufsz, timeout));
	} else if (strcmp (se_type, "edg-se") == 0) {
		free (se_type);
		return (se_getfilemd (surl, statbuf, errbuf, errbufsz, timeout));
	} else {
		free (se_type);
		gfal_errmsg(errbuf, errbufsz, "The Storage Element type is neither 'srm_v1' nor 'edg-se'.");
		errno = EINVAL;
		return (-1);
	}
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
		gfal_errmsg(errbuf, errbufsz, "Bad destination URL syntax.");
		errno = EINVAL;
		return (-1);
	}
	if ((len = p - turl) > (protocolsz - 1)) {
		gfal_errmsg(errbuf, errbufsz, "Destination URL too long.");
		errno = ENAMETOOLONG;
		return (-1);
	}
	strncpy (protocol, turl, len);
	*(protocol + len) = '\0';

	if (strcmp (protocol, "file") == 0) {
		*pfn = p + 1;
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
	char *se_type;

	if (setypefromsurl (surl, &se_type, errbuf, errbufsz) < 0)
		return (-1);
	if (strcmp (se_type, "srm_v1") == 0) {
		free (se_type);
		return (srm_set_xfer_done (surl, reqid, fileid, token, oflag,
		    errbuf, errbufsz, timeout));
	} else if (strcmp (se_type, "edg-se") == 0) {
		free (se_type);
		return (se_set_xfer_done (surl, reqid, fileid, token, oflag,
		    errbuf, errbufsz, timeout));
	} else {
		free (se_type);
		gfal_errmsg(errbuf, errbufsz, "The Storage Element type is neither 'srm_v1' nor 'edg-se'.");
		errno = EINVAL;
		return (-1);
	}
}

set_xfer_running (const char *surl, int reqid, int fileid, char *token,
	char *errbuf, int errbufsz, int timeout)
{
	char *se_type;

	if (setypefromsurl (surl, &se_type, errbuf, errbufsz) < 0)
		return (-1);
	if (strcmp (se_type, "srm_v1") == 0) {
		free (se_type);
		return (srm_set_xfer_running (surl, reqid, fileid, token,
		    errbuf, errbufsz, timeout));
	} else if (strcmp (se_type, "edg-se") == 0) {
		free (se_type);
		return (se_set_xfer_running (surl, reqid, fileid, token,
		    errbuf, errbufsz));
	} else {
		free (se_type);
		gfal_errmsg(errbuf, errbufsz, "The Storage Element type is neither 'srm_v1' nor 'edg-se'.");
		errno = EINVAL;
		return (-1);
	}
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
turlfromsurlx (const char *surl, GFAL_LONG64 filesize, char **protocols,
	int oflag, int *reqid, int *fileid, char **token, char *errbuf,
	int errbufsz, int timeout)
{
	int *fileids;
	char *p;
	char *se_type;
	char **turls;

	if (setypefromsurl (surl, &se_type, errbuf, errbufsz) < 0)
		return (NULL);
	if (strcmp (se_type, "srm_v1") == 0) {
		free (se_type);
		if (srm_turlsfromsurls (1, &surl, &filesize, protocols, oflag,
		    reqid, &fileids, token, &turls, errbuf, errbufsz, timeout) <= 0)
			return (NULL);
		*fileid = fileids[0];
	       p = turls[0];
	       free (fileids);
	       free (turls);
	       return (p);
	} else if (strcmp (se_type, "edg-se") == 0) {
		free (se_type);
		return (se_turlfromsurl (surl, protocols, oflag, reqid, fileid,
		    token, errbuf, errbufsz, timeout));
	} else {
		free (se_type);
		gfal_errmsg(errbuf, errbufsz, "The Storage Element type is neither 'srm_v1' nor 'edg-se'.");
		errno = EINVAL;
		return (NULL);
	}
}

char *
turlfromsurl (const char *surl, char **protocols, int oflag, int *reqid,
	int *fileid, char **token, char *errbuf, int errbufsz, int timeout)
{
	GFAL_LONG64 zero = 0;

	return (turlfromsurlx (surl, zero, protocols, oflag, reqid, fileid,
	    token, errbuf, errbufsz, timeout));
}

get_cat_type (char **cat_type) {
	char *cat_env;
	/* JC FIXME no hardcoded strings */
	char *default_cat = "edg";

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
		free (cat_type);
		gfal_errmsg(errbuf, errbufsz, "The LFC catalog doesn't support the setfilesize() method.");
		errno = EINVAL;
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
	char *cat_type;
	if (get_cat_type (&cat_type) < 0) {
		return (-1);
	}
	if (strcmp (cat_type, "edg") == 0) {
		free (cat_type);
		return (rmc_register_alias (guid, lfn, errbuf, errbufsz));
	} else if (strcmp (cat_type, "lfc") == 0) {
		free (cat_type);
		return (lfc_create_alias (guid, lfn, size, errbuf, errbufsz));
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
