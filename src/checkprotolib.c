/*
 * Copyright (C) 2003-2004 by CERN
 */

/*
 * @(#)$RCSfile: checkprotolib.c,v $ $Revision: 1.5 $ $Date: 2005/05/31 08:54:36 $ CERN Jean-Philippe Baud
 */

#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#if GFAL_ENABLE_DCAP
#include "dcap.h"
#endif
#if GFAL_ENABLE_RFIO
#include "rfio_api.h"
#include "serrno.h"
#endif
#include "gfal.h"

#if GFAL_ENABLE_DCAP
off_t
dc_lseek32 (int fildes, off_t offset, int whence)
{
	off64_t off64in;
	off64_t off64out;
	struct proto_ops *pops;

	if ((pops = find_pops ("dcap")) == NULL)
		return (-1);
	off64in = offset;
	off64out = pops->lseek64 (fildes, off64in, whence);
	if (off64out > (off64_t)2147483647 ) {
		errno = EFBIG;
		return (-1);
	}
	return ((off_t) off64out);
}

off64_t
dc_lseek64 (int fildes, off64_t offset, int whence)
{
	off_t off32in;
	off_t off32out;
	struct proto_ops *pops;

	if (offset > (off64_t)2147483647 ) {
		errno = EFBIG;
		return ((off64_t) -1);
	}
	if ((pops = find_pops ("dcap")) == NULL)
		return ((off64_t) -1);
	off32in = offset;
	off32out = pops->lseek (fildes, off32in, whence);
	return ((off64_t) off32out);
}

checkdcaplib (struct proto_ops *pops)
{
	void *dlhandle;

	if ((dlhandle = dlopen ("libdcap.so", RTLD_LAZY)) == NULL) {
		pops->libok == -1;
		return (-1);
	}
	pops->libok = 1;
	pops->geterror = (int (*) ()) dlsym (dlhandle, "__dc_errno");
	pops->access = (int (*) (const char *, int)) dlsym (dlhandle, "dc_access");
	pops->chmod = (int (*) (const char *, mode_t)) dlsym (dlhandle, "dc_chmod");
	pops->close = (int (*) (int)) dlsym (dlhandle, "dc_close");
	pops->closedir = (int (*) (DIR *)) dlsym (dlhandle, "dc_closedir");
	pops->lseek = (off_t (*) (int, off_t, int)) &dc_lseek32;
	pops->lseek64 = (off64_t (*) (int, off64_t, int)) dlsym (dlhandle, "dc_lseek");
	pops->lstat = (int (*) (const char *, struct stat *)) dlsym (dlhandle, "dc_lstat");
	pops->lstat64 = (int (*) (const char *, struct stat64 *)) dlsym (dlhandle, "dc_lstat64");
	pops->mkdir = (int (*) (const char *, mode_t)) dlsym (dlhandle, "dc_mkdir");
	pops->open = (int (*) (const char *, int, ...)) dlsym (dlhandle, "dc_open");
	pops->opendir = (DIR * (*) (const char *)) dlsym (dlhandle, "dc_opendir");
	pops->read = (ssize_t (*) (int, void *, size_t)) dlsym (dlhandle, "dc_read");
	pops->readdir = (struct dirent * (*) (DIR *)) dlsym (dlhandle, "dc_readdir");
	pops->readdir64 = (struct dirent64 * (*) (DIR *)) dlsym (dlhandle, "dc_readdir64");
	pops->rename = (int (*) (const char *, const char *)) &rename;
	pops->rmdir = (int (*) (const char *)) dlsym (dlhandle, "dc_rmdir");
	pops->setfilchg = (ssize_t (*) (int, const void *, size_t)) &dummysetfilchg;
	pops->stat = (int (*) (const char *, struct stat *)) dlsym (dlhandle, "dc_stat");
	pops->stat64 = (int (*) (const char *, struct stat64 *)) dlsym (dlhandle, "dc_stat64");
	pops->unlink = (int (*) (const char *)) dlsym (dlhandle, "dc_unlink");
	pops->write = (ssize_t (*) (int, const void *, size_t)) dlsym (dlhandle, "dc_write");
	return (0);
}

mapdcaperror (struct proto_ops *pops, int ioflag)
{
	int *errp;

	if (! ioflag)
		return (errno);
	errp = (int *) pops->geterror();
	switch (*errp) {
	case DEAD:
		return (EACCES);
	case DENE:
		return (ENOENT);
	case DEEVAL:
	case DEFLAGS:
		return (EINVAL);
	case DEMALLOC:
		return (ENOMEM);
	default:
		return (ECOMM);
	}
}
#endif

#if GFAL_ENABLE_RFIO
checkrfiolib (struct proto_ops *pops)
{
	void *dlhandle;
	char *p;

	p = getenv ("LCG_RFIO_TYPE");
	if (p && strcmp (p, "dpm") == 0) {
		if ((dlhandle = dlopen ("libdpm.so", RTLD_LAZY)) == NULL) {
			pops->libok == -1;
			return (-1);
		}
	} else if (p && strcmp (p, "castor") == 0) {
		if ((dlhandle = dlopen ("libshift.so", RTLD_LAZY)) == NULL) {
			pops->libok == -1;
			return (-1);
		}
	} else if ((dlhandle = dlopen ("libshift.so", RTLD_LAZY)) == NULL &&
	    (dlhandle = dlopen ("libdpm.so", RTLD_LAZY)) == NULL) {
		pops->libok == -1;
		return (-1);
	}
	pops->libok = 1;
	pops->geterror = (int (*) ()) dlsym (dlhandle, "rfio_serrno");
	pops->access = (int (*) (const char *, int)) dlsym (dlhandle, "rfio_access");
	pops->chmod = (int (*) (const char *, mode_t)) dlsym (dlhandle, "rfio_chmod");
	pops->close = (int (*) (int)) dlsym (dlhandle, "rfio_close");
	pops->closedir = (int (*) (DIR *)) dlsym (dlhandle, "rfio_closedir");
	pops->lseek = (off_t (*) (int, off_t, int)) dlsym (dlhandle, "rfio_lseek");
	pops->lseek64 = (off64_t (*) (int, off64_t, int)) dlsym (dlhandle, "rfio_lseek64");
	pops->lstat = (int (*) (const char *, struct stat *)) dlsym (dlhandle, "rfio_lstat");
	pops->lstat64 = (int (*) (const char *, struct stat64 *)) dlsym (dlhandle, "rfio_lstat64");
	pops->mkdir = (int (*) (const char *, mode_t)) dlsym (dlhandle, "rfio_mkdir");
	pops->open = (int (*) (const char *, int, ...)) dlsym (dlhandle, "rfio_open");
	pops->opendir = (DIR * (*) (const char *)) dlsym (dlhandle, "rfio_opendir");
	pops->read = (ssize_t (*) (int, void *, size_t)) dlsym (dlhandle, "rfio_read");
	pops->readdir = (struct dirent * (*) (DIR *)) dlsym (dlhandle, "rfio_readdir");
	pops->readdir64 = (struct dirent64 * (*) (DIR *)) dlsym (dlhandle, "rfio_readdir64");
	pops->rename = (int (*) (const char *, const char *)) dlsym (dlhandle, "rfio_rename");
	pops->rmdir = (int (*) (const char *)) dlsym (dlhandle, "rfio_rmdir");
	pops->setfilchg = (ssize_t (*) (int, const void *, size_t)) dlsym (dlhandle, "rfio_HsmIf_FirstWrite");
	pops->stat = (int (*) (const char *, struct stat *)) dlsym (dlhandle, "rfio_stat");
	pops->stat64 = (int (*) (const char *, struct stat64 *)) dlsym (dlhandle, "rfio_stat64");
	pops->unlink = (int (*) (const char *)) dlsym (dlhandle, "rfio_unlink");
	pops->write = (ssize_t (*) (int, const void *, size_t)) dlsym (dlhandle, "rfio_write");
	return (0);
}

maprfioerror (struct proto_ops *pops, int ioflag)
{
	int err;

	if ((err = pops->geterror()) < SEBASEOFF)
		return (err);
	else
		return (ECOMM);
}
#endif

static struct proto_ops pops_array[] = {
	{
		"file",
		1,
		NULL,
		NULL,
		&mapposixerror,
		&access,
		&chmod,
		&close,
		&closedir,
		&lseek,
		&lseek64,
		&lstat,
		&lstat64,
		&mkdir,
		&open,
		&opendir,
		&read,
		&readdir,
		&readdir64,
		&rename,
		&rmdir,
		&dummysetfilchg,
		&stat,
		&stat64,
		&unlink,
		&write
	},

#if GFAL_ENABLE_RFIO
	{
		"rfio", 0, &checkrfiolib, NULL, &maprfioerror, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	},
#endif

#if GFAL_ENABLE_DCAP
	{
		"dcap", 0, &checkdcaplib, NULL, &mapdcaperror, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	},
	{
		"gsidcap", 0, &checkdcaplib, NULL, &mapdcaperror, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	},
	{
		"kdcap", 0, &checkdcaplib, NULL, &mapdcaperror, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	},
#endif
	{
		"", 1, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	}
};

static ssize_t
dummysetfilchg (int fd, const void *buffer, size_t size)
{
	return (0);
}

struct proto_ops *
find_pops (const char *protocol)
{
	struct proto_ops *pops;
	struct proto_ops *tmp;

	for (pops = pops_array; *pops->proto_name; pops++) {
		if (strcmp (protocol, pops->proto_name) == 0)
			break;
	}
	if (*pops->proto_name) {
		if (pops->libok > 0)
			return (pops);
		if (pops->libok == 0 && pops->checkprotolib (pops) == 0) {
			for (tmp = pops_array; *tmp->proto_name; tmp++) {
				if (tmp != pops &&
				    tmp->checkprotolib == pops->checkprotolib)
					memcpy (&tmp->libok, &pops->libok,
					    sizeof(struct proto_ops) - sizeof(char *));
			}
			return (pops);
		}
	}
	errno = EPROTONOSUPPORT;
	return (NULL);
}

static int
fnotsup ()
{
	errno = ENOTSUP;
	return (-1);
}

char **
get_sup_proto ()
{
	int i = 0;
	struct proto_ops *pops;
	static char *supported_protocols[sizeof(pops_array)/sizeof(struct proto_ops)];
	struct proto_ops *tmp;

	for (pops = pops_array;; pops++) {
		if (pops->libok < 0) continue;		/* library not accessible */
		if (pops->libok == 0) {
			if (pops->checkprotolib (pops))
				continue;
			for (tmp = pops_array; *tmp->proto_name; tmp++) {
				if (tmp != pops &&
				    tmp->checkprotolib == pops->checkprotolib)
					memcpy (&tmp->libok, &pops->libok,
					    sizeof(struct proto_ops) - sizeof(char *));
			}
		}
		supported_protocols[i++] = pops->proto_name;
		if (*pops->proto_name == '\0') break;
	}
	return (supported_protocols);
}

mapposixerror (struct proto_ops *pops, int ioflag)
{
	return (errno);
}
