/*
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: gfal.h,v $ $Revision: 1.6 $ $Date: 2006/11/09 16:38:36 $ CERN Jean-Philippe Baud
 */

#ifndef _GFAL_H
#define _GFAL_H

#include "gfal_constants.h"

struct dir_info {
	DIR	*dir;
	struct proto_ops *pops;
};

static ssize_t dummysetfilchg ();
static int fnotsup ();

struct proto_ops {
	char	*proto_name;
	int	libok;	/* -1 (library is not accessible), 0 (library not checked yet), 1 (library ok) */
	int	(*checkprotolib)(struct proto_ops *);
	int	(*geterror)();
	int	(*maperror)(struct proto_ops *, int);
	int	(*access)(const char *, int);
	int	(*chmod)(const char *, mode_t);
	int	(*close)(int);
	int	(*closedir)(DIR *);
	off_t	(*lseek)(int, off_t, int);
	off64_t	(*lseek64)(int, off64_t, int);
	int	(*lstat)(const char *, struct stat *);
	int	(*lstat64)(const char *, struct stat64 *);
	int	(*mkdir)(const char *, mode_t);
	int	(*open)(const char *, int, ...);
	DIR	*(*opendir)(const char *);
	ssize_t	(*read)(int, void *, size_t);
	struct dirent	*(*readdir)(DIR *);
	struct dirent64	*(*readdir64)(DIR *);
	int	(*rename)(const char *, const char *);
	int	(*rmdir)(const char *);
	ssize_t	(*setfilchg)(int, const void *, size_t);
	int	(*stat)(const char *, struct stat *);
	int	(*stat64)(const char *, struct stat64 *);
	int	(*unlink)(const char *);
	ssize_t	(*write)(int, const void *, size_t);
};

struct xfer_info {
	int	fd;
	int	oflag;
	char	*surl;
	int	reqid;
	int	fileid;
	char 	*token;
	struct proto_ops *pops;
};

static struct dir_info *alloc_di (DIR *);
static struct xfer_info *alloc_xi (int);
static struct dir_info *find_di (DIR *);
static struct xfer_info *find_xi (int);
static int free_di (struct dir_info *);
static int free_xi (int);
static int mdtomd32 (struct stat64 *, struct stat *);
struct proto_ops *find_pops (const char *);
char **get_sup_proto ();
int mapposixerror (struct proto_ops *, int);
#endif
