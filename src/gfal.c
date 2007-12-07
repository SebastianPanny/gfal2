/*
 * Copyright (C) 2003-2005 by CERN
 */

/*
 * @(#)$RCSfile: gfal.c,v $ $Revision: 1.65 $ $Date: 2007/12/07 14:38:21 $ CERN Jean-Philippe Baud
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
#include <voms_apic.h>
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
static char *gfal_vo = NULL;

enum status_type {DEFAULT_STATUS = 0, MD_STATUS, PIN_STATUS};

static int copy_gfal_results (gfal_internal, enum status_type);
static int check_gfal_internal (gfal_internal, char *, int);

/* the version should be set by a "define" at the makefile level */
static const char gfalversion[] = VERSION;

int
gfal_set_vo (const char *vo)
{
	if ((gfal_vo = strdup (vo)) == NULL)
		return (-1);
	
	return (0);
}

char *
gfal_get_vo (char *errbuf, int errbufsz)
{
	if (gfal_vo == NULL && (gfal_vo = getenv ("LCG_GFAL_VO")) == NULL) {
		struct vomsdata *vd;
		int error;
		char errmsg[ERRMSG_LEN];

		if ((vd = VOMS_Init ("", "")) == NULL) {
			VOMS_ErrorMessage (vd, error, errmsg, ERRMSG_LEN);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		}
		else if (!VOMS_SetVerificationType (VERIFY_NONE, vd, &error)) {
			VOMS_ErrorMessage (vd, error, errmsg, ERRMSG_LEN);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		}
		else if (!VOMS_RetrieveFromProxy (RECURSE_CHAIN, vd, &error)) {
			VOMS_ErrorMessage (vd, error, errmsg, ERRMSG_LEN);
			gfal_errmsg (errbuf, errbufsz, errmsg);
		}
		else if (vd->data && vd->data[0]) {
			if ((gfal_vo = strdup (vd->data[0]->voname)) == NULL)
				return (NULL);
		}
	}

	if (gfal_vo == NULL)
		gfal_errmsg (errbuf, errbufsz, "Unable to get the VO name neither from environment (LCG_GFAL_VO) nor from the proxy");

	return (gfal_vo);
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
		free (xi_array[fd]);
		xi_array[fd] = NULL;
	}
	return (0);
}

gfal_access (const char *path, int amode)
{
	char errbuf[ERRMSG_LEN];
	char pathbuf[1024];
	char *guid = NULL, *surl = NULL, *turl = NULL, *pfn = NULL;
	struct proto_ops *pops;
	char protocol[64];
	char **supported_protocols;
	int reqid, fileid;
	char *token = NULL;

	strncpy (pathbuf, path, 1024);

	if (strncmp (pathbuf, "lfn:", 4) == 0) {
		char *cat_type;
		int islfc;
		if (get_cat_type (&cat_type) < 0)
			return (-1);
		islfc = strcmp (cat_type, "lfc") == 0;
		free (cat_type);

		if (islfc)
			return lfc_accessl (pathbuf + 4, amode, errbuf, ERRMSG_LEN);

		if ((guid = guidfromlfn (pathbuf + 4, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
	}
	if (guid || (strncmp (pathbuf, "guid:", 5) == 0 && (guid = pathbuf + 5))) {
		if ((surl = surlfromguid (guid, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
		if (guid != pathbuf) free (guid);
	}
	if ((surl || (surl = pathbuf)) && strncmp (surl, "srm:", 4) == 0) {
		int rc, i = 0;
		char **se_endpoints;
		char **se_types;
		char *srmv2_endpoint = NULL;
		struct srmv2_filestatus *filestatuses;

		if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, NULL, 0) < 0)
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

		if (srmv2_access (1, (const char **) &surl, srmv2_endpoint, amode,
					&filestatuses, errbuf, ERRMSG_LEN, 0) < 1 || !filestatuses) {
			free (srmv2_endpoint);
			return (-1);
		}
		if (filestatuses[0].surl) free (filestatuses[0].surl);
		errno = filestatuses[0].status;
		rc = filestatuses[0].status == 0 ? 0 : -1;
		free (filestatuses);
		free (srmv2_endpoint);
		return (rc);
	} else if (strncmp (surl, "sfn:", 4) == 0) {
		supported_protocols = get_sup_proto ();

		if ((turl = turlfromsfn (surl, supported_protocols, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
		if (surl != path) free (surl);
	}
	if (turl == NULL && (turl = strdup (path)) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	if (parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, ERRMSG_LEN) < 0) {
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
	char errbuf[ERRMSG_LEN];
	char *cat_type = NULL;
	int islfc;

	if (get_cat_type (&cat_type) < 0)
		return (-1);
	islfc = strcmp (cat_type, "lfc") == 0;
	free (cat_type);

	if (islfc && strncmp (path, "lfn:", 4) == 0) {
		return (lfc_chmodl (path + 4, mode, errbuf, ERRMSG_LEN) < 0);
	} else if (islfc && strncmp (path, "guid:", 5) == 0) {
		int i, rc = 0;
		char **lfns;

		if ((lfns = lfnsforguid (path + 5, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);

		if (lfns[0] != NULL)
			rc = lfc_chmodl (lfns[0] + 4, mode, errbuf, ERRMSG_LEN);

		for (i = 0; lfns[i] != NULL; ++i)
			free (lfns[i]);
		free (lfns);
		return (rc);
	}

	/* gfal_chmod is only supported with LFC */
	errno = EPROTONOSUPPORT;
	return (-1);
}

gfal_close (int fd)
{
	char errbuf[ERRMSG_LEN];
	int rc;
	int rc1 = 0;
	int sav_errno = 0;
	struct xfer_info *xi;

	if ((xi = find_xi (fd)) == NULL)
		return (-1);
	if ((rc = xi->pops->close (fd)) < 0)
		sav_errno = xi->pops->maperror (xi->pops, 1);

	/* set status "done" */

	if (xi->surl) {
		rc1 = set_xfer_done (xi->surl, xi->reqid, xi->fileid,
				xi->token, xi->oflag, errbuf, ERRMSG_LEN, 0);
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
	char errbuf[ERRMSG_LEN];
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
		if ((guid = guidfromlfn (filename + 4, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid, errbuf, ERRMSG_LEN)) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		if ((rc = getfilemd (fn, &statb64, errbuf, ERRMSG_LEN, 0)) == 0)
			rc = mdtomd32 (&statb64, statbuf);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL, errbuf, ERRMSG_LEN)) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, ERRMSG_LEN)) == 0) {
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
	char errbuf[ERRMSG_LEN];
	char *fn;
	char *guid;
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];
	int rc;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid, errbuf, ERRMSG_LEN)) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		rc = getfilemd (fn, statbuf, errbuf, ERRMSG_LEN, 0);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL, errbuf, ERRMSG_LEN)) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, ERRMSG_LEN)) == 0) {
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
	char errbuf[ERRMSG_LEN];
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];

	if (strncmp (dirname, "guid:", 5) == 0 ||
			strncmp (dirname, "sfn:", 4) == 0) {
		errno = EPROTONOSUPPORT;
		return (-1);
	}

	if (strncmp (dirname, "lfn:", 4) == 0) {
		int islfc;
		char *cat_type;
		if (get_cat_type (&cat_type) < 0)
			return (-1);

		islfc = strcmp (cat_type, "lfc") == 0;
		free (cat_type);

		/* Only LFC has a tree-like structure */
		if (islfc)
			return lfc_mkdirp (dirname + 4, mode, errbuf, ERRMSG_LEN);

		/* So, mkdir is not supported for non-LFC file catalogs */
		errno = EPROTONOSUPPORT;
		return (-1);
	}

	if (strncmp (dirname, "srm:", 4) == 0) {
		// only with SRMv2 !
		int rc, i = 0;
		char **se_endpoints;
		char **se_types;
		char *srmv2_endpoint = NULL;

		if (setypesandendpointsfromsurl (dirname, &se_types, &se_endpoints, NULL, 0) < 0)
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

		rc = srmv2_makedirp (dirname, srmv2_endpoint, NULL, 0, 0);
		free (srmv2_endpoint);

		return (rc);
	}

	/* It is a TURL */
	if (parseturl (dirname, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, ERRMSG_LEN) < 0)
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
	char errbuf[ERRMSG_LEN];
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

	if ((flags & (O_WRONLY | O_CREAT)) == (O_WRONLY | O_CREAT) ||
			(flags & (O_RDWR | O_CREAT)) == (O_RDWR | O_CREAT)) {
		/* writing in a file, so a new file */
		newfile = 1;
	}

	if (strncmp (filename, "lfn:", 4) == 0) {
		if (!newfile) {
			if ((guid = guidfromlfn (filename + 4, errbuf, ERRMSG_LEN)) == NULL)
				goto err;
			if ((fn = surlfromguid (guid, errbuf, ERRMSG_LEN)) == NULL)
				goto err;
		} else { // newfile
			/* We check whether LFN exists, and if no, we create it */
			if ((guid = guidfromlfn (filename + 4, errbuf, ERRMSG_LEN)) != NULL) {
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
			if ((fn = surlfromguid (filename + 5, errbuf, ERRMSG_LEN)) == NULL)
				goto err;
		} else { // newfile
			/* we check the format of the given GUID */
			if (uuid_parse (filename + 5, uuid) < 0) {
				errno = EINVAL; /* invalid guid */
				goto err;
			}

			strncpy (guid_file, filename + 5, 37);

			if ((vo = gfal_get_vo (errbuf, ERRMSG_LEN)) == NULL) {
				errno = EINVAL;
				goto err;
			}

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
		char **se_types;
		char **se_endpoints;
		int isdisk = 0, issrmv1 = 0, issrmv2 = 0;
		int j = 0;

		/* we need a vo name to generate a valid SURL */
		if ((vo = gfal_get_vo (errbuf, ERRMSG_LEN)) == NULL) {
			errno = EINVAL;
			goto err;
		}

		/* We get the default se (for the VO), its type and its info */
		if ((default_se = get_default_se(errbuf, ERRMSG_LEN)) == NULL)
			goto err;
		if (setypesandendpoints (default_se, &se_types, &se_endpoints, errbuf, ERRMSG_LEN) < 0)
			return (-1);

		while (se_types[j]) {
			if (issrmv1 == 0 && strcmp (se_types[j], "srm_v1") == 0)
				issrmv1 = 1;
			else if (issrmv2 == 0 && strcmp (se_types[j], "srm_v2") == 0)
				issrmv2 = 1;
			else if (isdisk == 0 && strcmp (se_types[j], "disk") == 0)
				isdisk = 1;

			free (se_endpoints[j]);
			free (se_types[j]);
			++j;
		}
		free (se_types);
		free (se_endpoints);

		if (!isdisk && !issrmv1 && !issrmv2) {
			free (se_type);
			gfal_errmsg(errbuf, ERRMSG_LEN, "The Storage Element type is neither 'disk', 'srm_v1' nor 'srm_v2'.");
			errno = EINVAL;
			goto err;
		}
		free (se_type);

		if (get_seap_info (default_se, &ap, &pn, errbuf, ERRMSG_LEN) < 0)
			goto err;

		for (i=0; ap[i]; ++i) {
			if (strcmp (ap[i], "gsiftp") == 0) port = pn[i];
			free (ap[i]);
		}

		free (ap);
		free (pn);

		if (!port) {
			gfal_errmsg(errbuf, ERRMSG_LEN, "protocol not supported by Storage Element.");
			errno = EPROTONOSUPPORT;
			goto err;
		}

		/* now create dir path which is either sa_path, sa_root or combination of ce_ap & sa_root */
		if(get_sa_path (default_se, vo, &sa_path, &sa_root, errbuf, ERRMSG_LEN) < 0) 
			goto err;
		if(sa_path != NULL) {
			if (isdisk)
				sprintf (dir_path, "sfn://%s%s%s", default_se, *sa_path=='/'?"":"/", sa_path);
			else
				sprintf (dir_path, "srm://%s%s%s", default_se, *sa_path=='/'?"":"/", sa_path);
		} else {  /* sa_root != NULL */
			if (isdisk) {
				if (get_ce_ap (default_se, &ce_ap, errbuf, ERRMSG_LEN) < 0)
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
						flags, &reqid, &fileid, &token, errbuf, ERRMSG_LEN, 0)) == NULL)
			goto err;
	} else if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, supported_protocols, errbuf, ERRMSG_LEN)) == NULL)
			goto err;
	} else		/* assume that is a pfn */
		turl = fn;
	if (parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, ERRMSG_LEN) < 0)
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
				xi->token, errbuf, ERRMSG_LEN, 0);
	}

	if (newfile && lfn) {
		if (create_alias_m (guid_file, lfn, mode, 0, errbuf, ERRMSG_LEN) < 0)
			goto err;
		if (register_pfn (guid_file, fn, errbuf, ERRMSG_LEN) < 0) {
			unregister_alias (guid_file, filename, errbuf, ERRMSG_LEN);
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
		deletepfn (pfn, NULL, errbuf, ERRMSG_LEN);
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
	char errbuf[ERRMSG_LEN];
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

		if (islfn) dir = (DIR *) lfc_opendirlg (dirname + 4, NULL, errbuf, ERRMSG_LEN);
		else	   dir = (DIR *) lfc_opendirlg (NULL, dirname, errbuf, ERRMSG_LEN);

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
	if (parseturl (dirname, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, ERRMSG_LEN) < 0)
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
	char errbuf[ERRMSG_LEN];
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
			return lfc_renamel (old_name + 4, new_name + 4, errbuf, ERRMSG_LEN);
		else if (isedg) {
			char *guid;
			int rc;

			if ((guid = guidfromlfn (old_name + 4, errbuf, ERRMSG_LEN)) == NULL)
				return (-1);
			if (rmc_register_alias (guid, new_name, errbuf, ERRMSG_LEN) < 0) {
				free (guid);
				return (-1);
			}
			rc = rmc_unregister_alias (guid, old_name, errbuf, ERRMSG_LEN);
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
	if (parseturl (old_name, protocol1, sizeof(protocol1), pathbuf1, sizeof(pathbuf1), &old_pfn, errbuf, ERRMSG_LEN) < 0)
		return (-1);
	if (parseturl (new_name, protocol2, sizeof(protocol2), pathbuf2, sizeof(pathbuf2), &new_pfn, errbuf, ERRMSG_LEN) < 0)
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
	char errbuf[ERRMSG_LEN];
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
			return lfc_rmdirl (dirname + 4, errbuf, ERRMSG_LEN);

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
	if (parseturl (dirname, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, ERRMSG_LEN) < 0)
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
	char errbuf[ERRMSG_LEN];
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
		int islfc;
		char *cat_type;
		if (get_cat_type (&cat_type) < 0)
			return (-1);

		islfc = strcmp (cat_type, "lfc") == 0;
		free (cat_type);

		if (islfc)
			return lfc_statl (filename + 4, NULL, statbuf, errbuf, ERRMSG_LEN);

		if ((guid = guidfromlfn (filename + 4, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid, errbuf, ERRMSG_LEN)) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		if ((rc = getfilemd (fn, &statb64, errbuf, ERRMSG_LEN, 0)) == 0)
			rc = mdtomd32 (&statb64, statbuf);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL, errbuf, ERRMSG_LEN)) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, ERRMSG_LEN)) == 0) {
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
	char errbuf[ERRMSG_LEN];
	char *fn;
	char *guid;
	char pathbuf[1024];
	char *pfn;
	struct proto_ops *pops;
	char protocol[64];
	int rc;
	char *turl;

	if (strncmp (filename, "lfn:", 4) == 0) {
		if ((guid = guidfromlfn (filename + 4, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
		if ((fn = surlfromguid (guid, errbuf, ERRMSG_LEN)) == NULL) {
			free (guid);
			return (-1);
		}
		free (guid);
	} else if (strncmp (filename, "guid:", 5) == 0) {
		if ((fn = surlfromguid (filename + 5, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
	} else
		fn = (char *)filename;
	if (strncmp (fn, "srm:", 4) == 0) {
		rc = getfilemd (fn, statbuf, errbuf, ERRMSG_LEN, 0);
		if (fn != filename) free (fn);
		return (rc);
	}
	if (strncmp (fn, "sfn:", 4) == 0) {
		if ((turl = turlfromsfn (fn, NULL, errbuf, ERRMSG_LEN)) == NULL) {
			if (fn != filename) free (fn);
			return (-1);
		}
	} else		/* assume that is a pfn */
		turl = fn;
	if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf),
					&pfn, errbuf, ERRMSG_LEN)) == 0) {
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
	char errbuf[ERRMSG_LEN];
	char *guid, *guid_lfn;
	int islfn = 0, i = 0;
	char **pfns;
	int rc = 0;

	if (strncmp (filename, "lfn:", 4) == 0) {
		islfn = 1;
		if ((guid_lfn = guidfromlfn (filename + 4, errbuf, ERRMSG_LEN)) == NULL)
			return (-1);
	}
	if ((islfn && (guid = guid_lfn)) || (strncmp (filename, "guid:", 5) == 0 && (guid = strdup (filename + 5)))) {
		/* must try to delete all PFNs mapped to this guid */
		if ((pfns = surlsfromguid (guid, errbuf, ERRMSG_LEN)) == NULL) {
			if (guid != NULL) free (guid);
			return (-1);
		}
		while (pfns[i]) {
			if (deletepfn (pfns[i], guid, errbuf, ERRMSG_LEN) == 0 && islfn)
				unregister_pfn (guid, pfns[i], errbuf, ERRMSG_LEN);
			else --rc;
			free (pfns[i++]);
		}
		free (pfns);

		if (rc == 0 && islfn)			
			rc = unregister_alias (guid, filename + 4, errbuf, ERRMSG_LEN);

		if (guid != NULL) free (guid);
		return (rc == 0 ? 0 : -1);
	} else
		return (deletepfn (filename, NULL, errbuf, ERRMSG_LEN));
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

gfal_deletesurls (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_statuses) {
			free (req->srmv2_statuses);
			req->srmv2_statuses = NULL;
		}
		if (req->srmv2_token) {
			free (req->srmv2_token);
			req->srmv2_token = NULL;
		}
		ret = srmv2_deletesurls (req->nbfiles, (const char **) req->surls, req->endpoint,
				&(req->srmv2_statuses), errbuf, errbufsz, req->timeout);
	} else if (req->setype == TYPE_SRM) {
		if (req->srm_statuses) {
			free (req->srm_statuses);
			req->srm_statuses = NULL;
		}
		ret = srm_deletesurls (req->nbfiles, (const char **) req->surls, req->endpoint,
				&(req->srm_statuses), errbuf, errbufsz, req->timeout);
	} else { // req->setype == TYPE_SE
		if (req->sfn_statuses) {
			free (req->sfn_statuses);
			req->sfn_statuses = NULL;
		}
		ret = sfn_deletesurls (req->nbfiles, (const char **) req->surls,
				&(req->sfn_statuses), errbuf, errbufsz);
	}

	req->returncode = ret;
	return (copy_gfal_results (req, DEFAULT_STATUS));
}

gfal_turlsfromsurls (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_pinstatuses) {
			free (req->srmv2_pinstatuses);
			req->srmv2_pinstatuses = NULL;
		}
		if (req->srmv2_token) {
			free (req->srmv2_token);
			req->srmv2_token = NULL;
		}
		if ((req->oflag & O_ACCMODE) == 0)
			ret = srmv2_turlsfromsurls_get (req->nbfiles, (const char **) req->surls, req->endpoint,
					req->srmv2_desiredpintime, req->srmv2_spacetokendesc, req->protocols,
					&(req->srmv2_token), &(req->srmv2_pinstatuses), errbuf, errbufsz, req->timeout);
		else
			ret = srmv2_turlsfromsurls_put (req->nbfiles, (const char **) req->surls, req->endpoint,
					req->filesizes, req->srmv2_desiredpintime, req->srmv2_spacetokendesc, req->protocols,
					&(req->srmv2_token), &(req->srmv2_pinstatuses), errbuf, errbufsz, req->timeout);
	} else if (req->setype == TYPE_SRM) {
		if (req->srm_statuses) {
			free (req->srm_statuses);
			req->srm_statuses = NULL;
		}
		ret = srm_turlsfromsurls (req->nbfiles, (const char **) req->surls, req->endpoint,
				req->filesizes, req->protocols, req->oflag, &(req->srm_reqid),
				&(req->srm_statuses), errbuf, errbufsz, req->timeout);
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

gfal_ls (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_mdstatuses) {
			free (req->srmv2_mdstatuses);
			req->srmv2_mdstatuses = NULL;
		}
		if (req->srmv2_token) {
			free (req->srmv2_token);
			req->srmv2_token = NULL;
		}
		ret = srmv2_getfilemd (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_lslevels,
				req->srmv2_lsoffset, req->srmv2_lscount, &(req->srmv2_mdstatuses), &(req->srmv2_token),
				errbuf, errbufsz, req->timeout);
	} else if (req->setype == TYPE_SRM) {
		if (req->srm_mdstatuses) {
			free (req->srm_mdstatuses);
			req->srm_mdstatuses = NULL;
		}
		ret = srm_getfilemd (req->nbfiles, (const char **) req->surls, req->endpoint,
				&(req->srm_mdstatuses), errbuf, errbufsz, req->timeout);
	} else { // req->setype == TYPE_SE
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "gfal_ls: SFNs aren't supported");
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EPROTONOSUPPORT;
		return (-1);;
	}

	req->returncode = ret;
	return (copy_gfal_results (req, MD_STATUS));
}

gfal_get (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_pinstatuses) {
			free (req->srmv2_pinstatuses);
			req->srmv2_pinstatuses = NULL;
		}
		if (req->srmv2_token) {
			free (req->srmv2_token);
			req->srmv2_token = NULL;
		}
		ret = srmv2_gete (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_spacetokendesc,
				req->srmv2_desiredpintime, req->protocols, &(req->srmv2_token),
				&(req->srmv2_pinstatuses), errbuf, errbufsz, req->timeout);
	} else if (req->setype == TYPE_SRM) {
		if (req->srm_statuses) {
			free (req->srm_statuses);
			req->srm_statuses = NULL;
		}
		ret = srm_getxe (req->nbfiles, (const char **) req->surls, req->endpoint, req->protocols,
				&(req->srm_reqid), &(req->srm_statuses), errbuf, errbufsz, req->timeout);
	} else { // req->setype == TYPE_SE
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "gfal_get: SFNs aren't supported");
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EPROTONOSUPPORT;
		return (-1);;
	}

	req->returncode = ret;
	return (copy_gfal_results (req, PIN_STATUS));
}

gfal_getstatus (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_pinstatuses) {
			free (req->srmv2_pinstatuses);
			req->srmv2_pinstatuses = NULL;
		}
		ret = srmv2_getstatuse (req->srmv2_token, req->endpoint, &(req->srmv2_pinstatuses),
				errbuf, errbufsz, req->timeout);
	} else if (req->setype == TYPE_SRM) {
		if (req->srm_statuses) {
			free (req->srm_statuses);
			req->srm_statuses = NULL;
		}
		ret = srm_getstatusxe (req->srm_reqid, req->endpoint, &(req->srm_statuses),
				errbuf, errbufsz, req->timeout);
	} else { // req->setype == TYPE_SE
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "gfal_getstatus: SFNs aren't supported");
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EPROTONOSUPPORT;
		return (-1);;
	}

	req->returncode = ret;
	return (copy_gfal_results (req, PIN_STATUS));
}

gfal_prestage (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_statuses) {
			free (req->srmv2_statuses);
			req->srmv2_statuses = NULL;
		}
		if (req->srmv2_token) {
			free (req->srmv2_token);
			req->srmv2_token = NULL;
		}
		ret = srmv2_prestagee (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_spacetokendesc,
				req->protocols, &(req->srmv2_token), &(req->srmv2_statuses),
				errbuf, errbufsz, req->timeout);
	} else {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "gfal_prestage: Only SRMv2-compliant SEs are supported");
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EPROTONOSUPPORT;
		return (-1);;
	}

	req->returncode = ret;
	return (copy_gfal_results (req, DEFAULT_STATUS));
}

gfal_prestagestatus (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_statuses) {
			free (req->srmv2_statuses);
			req->srmv2_statuses = NULL;
		}
		ret = srmv2_prestagestatuse (req->srmv2_token, req->endpoint, &(req->srmv2_statuses),
				errbuf, errbufsz, req->timeout);
	} else {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "gfal_prestagestatus: Only SRMv2-compliant SEs are supported");
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EPROTONOSUPPORT;
		return (-1);;
	}

	req->returncode = ret;
	return (copy_gfal_results (req, DEFAULT_STATUS));
}

gfal_pin (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_pinstatuses) {
			free (req->srmv2_pinstatuses);
			req->srmv2_pinstatuses = NULL;
		}
		ret = srmv2_pin (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
				req->srmv2_desiredpintime, &(req->srmv2_pinstatuses),
				errbuf, errbufsz, req->timeout);
	} else {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "gfal_prestage: Only SRMv2-compliant SEs are supported");
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EPROTONOSUPPORT;
		return (-1);;
	}

	req->returncode = ret;
	return (copy_gfal_results (req, PIN_STATUS));
}

gfal_release (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_statuses) {
			free (req->srmv2_statuses);
			req->srmv2_statuses = NULL;
		}
		ret = srmv2_release (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
				&(req->srmv2_statuses), errbuf, errbufsz, req->timeout);
	} else if (req->setype == TYPE_SRM) {
		int i;

		if (req->srm_statuses == NULL) {
			gfal_errmsg (errbuf, errbufsz, "gfal_release: no SRMv1 file ids");
			errno = EINVAL;
			return (-1);
		}
		for (i = 0; i < req->nbfiles; ++i) {
			if (srm_set_xfer_done (req->endpoint, req->srm_reqid, req->srm_statuses[i].fileid,
						errbuf, errbufsz, req->timeout) < 0)
				req->srm_statuses[i].status = errno;
			else
				req->srm_statuses[i].status = 0;
		}
		ret = 0;
	} else { // req->setype == TYPE_SE
		int i;

		if (req->sfn_statuses)
			free (req->sfn_statuses);
		if ((req->sfn_statuses = (struct sfn_filestatus *) calloc (req->nbfiles, sizeof (struct sfn_filestatus))) == NULL) {
			errno = ENOMEM;
			return (-1);
		}
		memset (req->sfn_statuses + i, 0, sizeof (struct srmv2_filestatus));
		for (i = 0; i < req->nbfiles; ++i) {
			req->sfn_statuses[i].surl = strdup (req->surls[i]);
			req->sfn_statuses[i].status = 0;
		}
		ret = 0;
	}

	req->returncode = ret;
	return (copy_gfal_results (req, DEFAULT_STATUS));
}

gfal_set_xfer_done (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_statuses) {
			free (req->srmv2_statuses);
			req->srmv2_statuses = NULL;
		}
		if (req->oflag == 0)
			ret = srmv2_set_xfer_done_get (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
					&(req->srmv2_statuses), errbuf, errbufsz, req->timeout);
		else
			ret = srmv2_set_xfer_done_put (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
					&(req->srmv2_statuses), errbuf, errbufsz, req->timeout);
	} else if (req->setype == TYPE_SRM) {
		int i;

		if (req->srm_statuses == NULL) {
			gfal_errmsg (errbuf, errbufsz, "gfal_release: no SRMv1 file ids");
			errno = EINVAL;
			return (-1);
		}
		for (i = 0; i < req->nbfiles; ++i) {
			if (srm_set_xfer_done (req->endpoint, req->srm_reqid, req->srm_statuses[i].fileid,
						errbuf, errbufsz, req->timeout) < 0)
				req->srm_statuses[i].status = errno;
			else
				req->srm_statuses[i].status = 0;
		}
		ret = 0;
	} else { // req->setype == TYPE_SE
		int i;

		if (req->sfn_statuses)
			free (req->sfn_statuses);
		if ((req->sfn_statuses = (struct sfn_filestatus *) calloc (req->nbfiles, sizeof (struct sfn_filestatus))) == NULL) {
			errno = ENOMEM;
			return (-1);
		}
		for (i = 0; i < req->nbfiles; ++i) {
			memset (req->sfn_statuses + i, 0, sizeof (struct srmv2_filestatus));
			req->sfn_statuses[i].surl = strdup (req->surls[i]);
			req->sfn_statuses[i].status = 0;
		}
		ret = 0;
	}

	errno = 0;
	req->returncode = ret;
	return (copy_gfal_results (req, DEFAULT_STATUS));
}

gfal_set_xfer_running (gfal_internal req, char *errbuf, int errbufsz)
{
	int ret;

	if (check_gfal_internal (req, errbuf, errbufsz) < 0)
		return (-1);

	if (req->setype == TYPE_SRMv2) {
		if (req->srmv2_statuses) {
			free (req->srmv2_statuses);
			req->srmv2_statuses = NULL;
		}
		ret = srmv2_set_xfer_running (req->nbfiles, (const char **) req->surls, req->endpoint, req->srmv2_token,
				&(req->srmv2_statuses), errbuf, errbufsz, req->timeout);
	} else if (req->setype == TYPE_SRM) {
		int i;

		if (req->srm_statuses == NULL) {
			gfal_errmsg (errbuf, errbufsz, "gfal_release: no SRMv1 file ids");
			errno = EINVAL;
			return (-1);
		}
		for (i = 0; i < req->nbfiles; ++i) {
			if (srm_set_xfer_running (req->endpoint, req->srm_reqid, req->srm_statuses[i].fileid,
						errbuf, errbufsz, req->timeout) < 0)
				req->srm_statuses[i].status = errno;
			else
				req->srm_statuses[i].status = 0;
		}
		ret = 0;
	} else { // req->setype == TYPE_SE
		int i;

		if (req->sfn_statuses)
			free (req->sfn_statuses);
		if ((req->sfn_statuses = (struct sfn_filestatus *) calloc (req->nbfiles, sizeof (struct sfn_filestatus))) == NULL) {
			errno = ENOMEM;
			return (-1);
		}
		for (i = 0; i < req->nbfiles; ++i) {
			memset (req->sfn_statuses + i, 0, sizeof (struct sfn_filestatus));
			req->sfn_statuses[i].surl = strdup (req->surls[i]);
			req->sfn_statuses[i].status = 0;
		}
		ret = 0;
	}

	errno = 0;
	req->returncode = ret;
	return (copy_gfal_results (req, DEFAULT_STATUS));
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
		if ((rc = parseturl (turl, protocol, sizeof(protocol), pathbuf, sizeof(pathbuf), &pfn, errbuf, errbufsz)) == 0) {
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
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	char errmsg[ERRMSG_LEN];

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if (srmv1_endpoint == NULL && strcmp (se_types[i], "srm_v1") == 0)
			srmv1_endpoint = se_endpoints[i];
		else if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
			srmv2_endpoint = se_endpoints[i];
		else
			free (se_endpoints[i]);

		free (se_types[i]);
		++i;
	}
	free (se_types);
	free (se_endpoints);

	/* if spacetokendesc specified by user and/or SRM v2.2 supported */
	if (((spacetokendesc != NULL) || !srmv1_endpoint) && srmv2_endpoint) {
		struct srmv2_filestatus *statuses;

		rc = srmv2_deletesurls (1, &surl, srmv2_endpoint, &statuses, errbuf, errbufsz, timeout);

		if (rc > 0) {
			rc = statuses[0].status == 0 ? 0 : -1;
			if (statuses[0].explanation) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", surl, statuses[0].explanation);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				free (statuses[0].explanation);
			}
			if (statuses[0].surl) free (statuses[0].surl);
			free (statuses);
		} else rc = -1;
	} else if (srmv1_endpoint) {
		struct srm_filestatus *statuses;

		rc = srm_deletesurls (1, &surl, srmv1_endpoint, &statuses, errbuf, errbufsz, timeout);

		if (rc > 0) {
			rc = statuses[0].status == 0 ? 0 : -1;
			if (statuses[0].surl) free (statuses[0].surl);
			free (statuses);
		} else rc = -1;
	} else {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: SE not published as ClassicSE nor SRMv1 nor SRMv2.2", surl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}

	if (srmv1_endpoint != NULL) free (srmv1_endpoint);
	if (srmv2_endpoint != NULL) free (srmv2_endpoint);

	return (rc);
}

getfilemd (const char *surl, struct stat64 *statbuf, char *errbuf, int errbufsz, int timeout)
{
	int rc, i = 0;
	char **se_endpoints;
	char **se_types;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if (srmv1_endpoint == NULL && strcmp (se_types[i], "srm_v1") == 0)
			srmv1_endpoint = se_endpoints[i];
		else if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
			srmv2_endpoint = se_endpoints[i];
		else
			free (se_endpoints[i]);

		free (se_types[i]);
		++i;
	}
	free (se_types);
	free (se_endpoints);

	if (srmv1_endpoint) {
		struct srm_mdfilestatus *mdstatuses = NULL;

		if (srm_getfilemd (1, &surl, srmv1_endpoint, &mdstatuses, errbuf, errbufsz, timeout) < 1 ||
				!mdstatuses) {
			free (srmv1_endpoint);
			if (srmv2_endpoint) free (srmv2_endpoint);
			return (-1);
		}

		if (mdstatuses[0].status) {
			errno = mdstatuses[0].status;
			rc = -1;
		} else {
			*statbuf = mdstatuses[0].stat;
			rc = 0;
		}

		if (mdstatuses[0].surl) free (mdstatuses[0].surl);
		free (mdstatuses);
	} else if (srmv2_endpoint) {
		struct srmv2_mdfilestatus *mdstatuses = NULL;

		if (srmv2_getfilemd (1, &surl, srmv2_endpoint, 0, 0, 0, &mdstatuses, NULL, errbuf, errbufsz, timeout) < 1 ||
				!mdstatuses) {
			free (srmv2_endpoint);
			return (-1);
		}

		if (mdstatuses[0].status) {
			errno = mdstatuses[0].status;
			rc = -1;
		} else {
			*statbuf = mdstatuses[0].stat;
			rc = 0;
		}

		if (mdstatuses[0].surl) free (mdstatuses[0].surl);
		free (mdstatuses);
	} else {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: SE not published as ClassicSE nor SRMv1 nor SRMv2.2", surl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}

	if (srmv1_endpoint)	free (srmv1_endpoint);
	if (srmv2_endpoint)	free (srmv2_endpoint);

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
#define ENDPOINT_DEFAULT_PREFIX "httpg://"
#define ENDPOINT_DEFAULT_PREFIX_LEN strlen("httpg://")

	static char *
endpointfromsurl (const char *surl, char *errbuf, int errbufsz, int _prefixing_on)
{
	int len;
	char *p, *endpoint;
	char errmsg[ERRMSG_LEN];
	int endpoint_offset=0;

	if (strncmp (surl, "srm://", 6) && strncmp (surl, "sfn://", 6)) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid SURL (must start with either 'srm://' or 'sfn://')", surl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (NULL);
	}

	p = strstr (surl + 6, "?SFN=");
	if (p == NULL) {
		p = strchr (surl + 6, '/');
		if (p == NULL) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid SURL", surl);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
			return (NULL);
		}
	}

	len = p - surl - 6;
	if ((endpoint = (char *) calloc (len + 1+ strlen(ENDPOINT_DEFAULT_PREFIX), sizeof (char*))) == NULL) {
		errno = ENOMEM;
		return (NULL);
	}//hack to ensure proper endpoint prefixing (httpg://)

	if(_prefixing_on && strncmp (surl+6, ENDPOINT_DEFAULT_PREFIX, ENDPOINT_DEFAULT_PREFIX_LEN) && (len>0))
	{
		strcpy(endpoint,ENDPOINT_DEFAULT_PREFIX);
		endpoint_offset=ENDPOINT_DEFAULT_PREFIX_LEN;
	}
	strncpy (endpoint + endpoint_offset, surl+6, len);
	endpoint[len + endpoint_offset] = 0;
	return (endpoint);
}

parsesurl (const char *surl, char *endpoint, int srm_endpointsz, char **sfn,
		char *errbuf, int errbufsz)
{
	int i = 0;
	char **se_endpoints;
	char **se_types;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	int len;
	char *p;
	char endpoint_tmp[256];
	char errmsg[ERRMSG_LEN];

	if (strncmp (surl, "srm://", 6) && strncmp (surl, "sfn://", 6)) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid SURL (must start with either 'srm://' or 'sfn://')", surl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}

	p = strstr (surl + 6, "?SFN=");
	if (p == NULL) {
		p = strchr (surl + 6, '/');
		if (p == NULL) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid SURL", surl);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
			return (-1);
		}
		*sfn = p;
	} else	*sfn = p + 5;

	if ((len = p - surl - 6) >= sizeof (endpoint_tmp)) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: SURL too long", surl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	strncpy (endpoint_tmp, surl + 6, len);
	endpoint_tmp[len] = 0;

	if (setypesandendpoints (endpoint_tmp, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if (srmv1_endpoint == NULL && strcmp (se_types[i], "srm_v1") == 0)
			srmv1_endpoint = se_endpoints[i];
		else if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
			srmv2_endpoint = se_endpoints[i];
		else
			free (se_endpoints[i]);

		free (se_types[i]);
		++i;
	}
	free (se_types);
	free (se_endpoints);

	if (srmv1_endpoint)
		strncpy (endpoint, srmv1_endpoint, srm_endpointsz);
	else if (srmv2_endpoint)
		strncpy (endpoint, srmv2_endpoint, srm_endpointsz);
	else {
		snprintf (errmsg, ERRMSG_LEN, "%s: No matching SRM-compliant SE", surl);
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}

	if (srmv1_endpoint) free (srmv1_endpoint);
	if (srmv2_endpoint) free (srmv2_endpoint);

	return (0);
}

parseturl (const char *turl, char *protocol, int protocolsz, char *pathbuf, int pathbufsz, char **pfn, char* errbuf, int errbufsz)
{
	int len;
	char *p,*p2;
	char errmsg[ERRMSG_LEN];

	if (strlen (turl) > pathbufsz - 1) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: TURL too long", turl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	}
	strcpy (pathbuf, turl);

	/* get protocol */

	if ((p = strstr (pathbuf, ":/")) == NULL) {
		/* to enable 'file' protocol by default
		   if (4 > (protocolsz - 1)) {
		   snprintf (errmsg, ERRMSG_LEN - 1, "%s: TURL too long", turl);
		   gfal_errmsg(errbuf, errbufsz, errmsg);
		   gfal_errmsg(errbuf, errbufsz, "TURL too long.");
		   errno = ENAMETOOLONG;
		   return (-1);
		   }
		   sprintf (protocol, "file");
		   */
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid TURL", turl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	} else if ((len = p - pathbuf) > (protocolsz - 1)) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: TURL too long", turl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
		return (-1);
	} else {
		strncpy (protocol, pathbuf, len);
		*(protocol + len) = '\0';
	}

	if (strcmp (protocol, "file") == 0) {
		if (p != NULL) {
			++p;
			if (*(p + 1) == '/' && (*(p + 2) != '/' || *(p + 3) == '/')) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid TURL", turl);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				errno = EINVAL;
				return (-1);
			}
			while (*(p + 1) == '/') ++p;
			memmove (pathbuf, p, strlen (p) + 1);
		}
	} else if (strcmp (protocol, "rfio") == 0) {
		p += 2;
		if (*p != '/' || (*(p + 1) == '/' && *(p + 2) != '/')) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid TURL", turl);
			gfal_errmsg(errbuf, errbufsz, errmsg);
			errno = EINVAL;
			return (-1);
		}
		p++;
		if (*p == '/') {	// no hostname ; *(p + 1) = '/' due to the previous test
			memmove (pathbuf, p + 1, strlen (p + 1) + 1);
		} else if (strchr (p, '?') == NULL && strchr (p, ':') == NULL) {
			// For Castor2-like RFIO TURL (eg. with "?svcClass=..."), pfn is the TURL, nothing to do
			// For other case, we want hostname:/filename
			memmove (pathbuf, p, strlen (p) + 1);
			if ((p = strchr (pathbuf, '/')) == NULL) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: Invalid TURL", turl);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				errno = EINVAL;
				return (-1);
			}
			// p is pointing on the slash just after the hostname
			if (*(p + 1) != '/')
				memmove (p + 1, p, strlen (p) + 1);

			*p = ':';
		}
		// For other cases (Castor2-like RFIO TURL), the entore turl is returned as pfn
	} 

	*pfn = pathbuf;
	return (0);
}

set_xfer_done (const char *surl, int reqid, int fileid, char *token, int oflag,
		char *errbuf, int errbufsz, int timeout)
{
	int rc, i = 0;
	char **se_endpoints;
	char **se_types;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	char errmsg[ERRMSG_LEN];

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if (srmv1_endpoint == NULL && strcmp (se_types[i], "srm_v1") == 0)
			srmv1_endpoint = se_endpoints[i];
		else if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
			srmv2_endpoint = se_endpoints[i];
		else
			free (se_endpoints[i]);

		free (se_types[i]);
		++i;
	}
	free (se_types);
	free (se_endpoints);

	/* if token specified  or SRM v2,2 supported only */
	if (((token != NULL) || !srmv1_endpoint) && srmv2_endpoint) {
		struct srmv2_filestatus *statuses;

		if ((oflag & O_ACCMODE) == 0) {
			rc = srmv2_set_xfer_done_get (1, &surl, srmv2_endpoint, token, &statuses, errbuf, errbufsz, timeout);

			if (rc > 0) {
				rc = statuses[0].status == 1 ? 0 : -1;
				if (statuses[0].explanation) {
					gfal_errmsg(errbuf, errbufsz, statuses[0].explanation);
					free (statuses[0].explanation);
				}
				free (statuses);
			} else rc = -1;
		} else {
			rc = srmv2_set_xfer_done_put (1, &surl, srmv2_endpoint, token, &statuses, errbuf, errbufsz, timeout);

			if (rc > 0) {
				rc = statuses[0].status == 1 ? 0 : -1;
				if (statuses[0].explanation) {
					snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", surl, statuses[0].explanation);
					gfal_errmsg(errbuf, errbufsz, errmsg);
					free (statuses[0].explanation);
				}
				free (statuses);
			}
		}
	} else if (srmv1_endpoint) {
		rc = srm_set_xfer_done (srmv1_endpoint, reqid, fileid, errbuf, errbufsz, timeout);
	} else {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: SE not published as ClassicSE nor SRMv1 nor SRMv2.2", surl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}

	if (srmv1_endpoint)	free (srmv1_endpoint);
	if (srmv2_endpoint)	free (srmv2_endpoint);

	return (rc);
}

set_xfer_running (const char *surl, int reqid, int fileid, char *token,
		char *errbuf, int errbufsz, int timeout)
{
	int rc, i = 0;
	char **se_endpoints;
	char **se_types;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (-1);

	while (se_types[i]) {
		if (srmv1_endpoint == NULL && strcmp (se_types[i], "srm_v1") == 0)
			srmv1_endpoint = se_endpoints[i];
		else if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
			srmv2_endpoint = se_endpoints[i];
		else
			free (se_endpoints[i]);

		free (se_types[i]);
		++i;
	}
	free (se_types);
	free (se_endpoints);

	/* if token specified  or SRM v2,2 supported only */
	if (((token != NULL) || !srmv1_endpoint) && srmv2_endpoint) {
		struct srmv2_filestatus *filestatuses;

		if (srmv2_set_xfer_running (1, &surl, srmv2_endpoint, token, &filestatuses, errbuf, errbufsz, timeout) < 1 ||
				!filestatuses) {
			if (srmv1_endpoint) free (srmv1_endpoint);
			free (srmv2_endpoint);
			return (-1);
		}

		rc = filestatuses[0].status == 0 ? 0 : -1;
		free (filestatuses[0].surl);
		free (filestatuses);
	} else if (srmv1_endpoint) {
		rc = srm_set_xfer_running (srmv1_endpoint, reqid, fileid, errbuf, errbufsz, timeout);
	} else {
		char errmsg[ERRMSG_LEN];
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: SE not published as ClassicSE nor SRMv1 nor SRMv2.2", surl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		rc = -1;
	}

	if (srmv1_endpoint) free (srmv1_endpoint);
	if (srmv2_endpoint) free (srmv2_endpoint);
	return (rc);
}

setypesandendpoints (const char *endpoint, char ***se_types, char ***se_endpoints, char *errbuf, int errbufsz)
{
	int rc;
	int len;
	char *p1, *p2;
	char endpoint_tmp[256];
	char errmsg[ERRMSG_LEN];

	if (se_types == NULL) {
		errno = EINVAL;
		return (-1);
	}
	if (strlen (endpoint) + 2 >= sizeof (endpoint_tmp)) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: Endpoint too long", endpoint);
		gfal_errmsg(errbuf, errbufsz, errmsg);
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
	char *endpoint_tmp;
	char errmsg[ERRMSG_LEN];

	if ((endpoint_tmp = endpointfromsurl (surl, errbuf, errbufsz,0)) == NULL)
		return (-1);

	return (setypesandendpoints (endpoint_tmp, se_types, se_endpoints, errbuf, errbufsz));
}

char *
turlfromsfn (const char *sfn, char **protocols, char *errbuf, int errbufsz) {
	char *turl;
	struct sfn_filestatus *statuses;

	if (sfn_turlsfromsurls (1, &sfn, protocols, &statuses, errbuf, errbufsz) < 0)
		return (NULL);

	if (statuses == NULL) {
		errno = ENOMEM;
		return (NULL);
	}

	turl = statuses[0].turl;
	if (statuses[0].surl) free (statuses[0].surl);
	errno = statuses[0].status;
	free (statuses);
	return (turl);
}

	char *
turlfromsurl2 (const char *surl, GFAL_LONG64 filesize, const char *spacetokendesc, char **protocols,
		int oflag, int *reqid, int *fileid, char **token, char *errbuf, int errbufsz, int timeout)
{
	char *p;
	int *fileids;
	int i = 0;
	char **se_endpoints;
	char **se_types;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	char errmsg[ERRMSG_LEN];

	if (setypesandendpointsfromsurl (surl, &se_types, &se_endpoints, errbuf, errbufsz) < 0)
		return (NULL);

	while (se_types[i]) {
		if (srmv1_endpoint == NULL && strcmp (se_types[i], "srm_v1") == 0)
			srmv1_endpoint = se_endpoints[i];
		else if (srmv2_endpoint == NULL && strcmp (se_types[i], "srm_v2") == 0)
			srmv2_endpoint = se_endpoints[i];
		else
			free (se_endpoints[i]);

		free (se_types[i]);
		++i;
	}
	free (se_types);
	free (se_endpoints);

	/* if spacetokendesc specified by user and/or SRM v2.2 supported */
	if (((spacetokendesc != NULL) || !srmv1_endpoint) && srmv2_endpoint) {
		struct srmv2_pinfilestatus *filestatuses = NULL;

		if ((oflag & O_ACCMODE) == 0) {
			if (srmv2_turlsfromsurls_get (1, &surl, srmv2_endpoint, 0, spacetokendesc, protocols,
						token, &filestatuses, errbuf, errbufsz, timeout) < 1 || !filestatuses) {
				if (srmv1_endpoint != NULL) free (srmv1_endpoint);
				free (srmv2_endpoint);
				return NULL;
			}
		} else {
			if ((srmv2_turlsfromsurls_put (1, &surl, srmv2_endpoint, &filesize, 0, spacetokendesc, protocols,
							token, &filestatuses, errbuf, errbufsz, timeout)) < 1 || !filestatuses) {
				if (srmv1_endpoint != NULL) free (srmv1_endpoint);
				free (srmv2_endpoint);
				return NULL;
			}
		}
		if (filestatuses[0].surl) free (filestatuses[0].surl);
		if (filestatuses[0].status) {
			errno = filestatuses[0].status;
			if (filestatuses[0].explanation) {
				snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", surl, filestatuses[0].explanation);
				gfal_errmsg(errbuf, errbufsz, errmsg);
				free (filestatuses[0].explanation);
			}
			free (filestatuses);
			if (srmv1_endpoint != NULL) free (srmv1_endpoint);
			free (srmv2_endpoint);
			return (NULL);
		} 

		p = filestatuses[0].turl;
		if (filestatuses[0].explanation) {
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: %s", surl, filestatuses[0].explanation);
			gfal_errmsg(errbuf, errbufsz, errmsg);
		}
		free (filestatuses);
	}  else	if (srmv1_endpoint) { /* if SRM v1.1 supported */
		struct srm_filestatus *filestatuses = NULL;

		if (srm_turlsfromsurls (1, &surl, srmv1_endpoint, &filesize, protocols, oflag,
					reqid, &filestatuses, errbuf, errbufsz, timeout) < 1) {
			free (srmv1_endpoint);
			if (srmv2_endpoint != NULL) free (srmv2_endpoint);
			return (NULL);
		}

		if (filestatuses[0].surl) free (filestatuses[0].surl);
		if (filestatuses[0].status) {
			errno = filestatuses[0].status;
			free (filestatuses);
			free (srmv1_endpoint);
			if (srmv2_endpoint != NULL) free (srmv2_endpoint);
			return (NULL);
		} 

		p = filestatuses[0].turl;
		*fileid = filestatuses[0].fileid;
		free (filestatuses);
	} else {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: SE not published as ClassicSE nor SRMv1 nor SRMv2.2", surl);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = EINVAL;
		p = NULL;
	}

	if (srmv1_endpoint != NULL) free (srmv1_endpoint);
	if (srmv2_endpoint != NULL) free (srmv2_endpoint);
	return (p);
}

	char *
turlfromsurlx (const char *surl, GFAL_LONG64 filesize, char **protocols, int oflag, int *reqid,
		int *fileid, char **token, char *errbuf, int errbufsz, int timeout)
{
	GFAL_LONG64 zero = 0;

	return (turlfromsurl2 (surl, filesize, NULL, protocols, oflag, reqid, fileid, 
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
		gfal_errmsg(errbuf, errbufsz, "EDG catalogs don't support the getfilesizeg() method.");
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
		gfal_errmsg (errbuf, errbufsz, "Unable to determine the catalog type");
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
	int i;
	char  *p, *p1, *p2, *p3;
	int ret;
	char *default_se;
	int  localsurl, default_match, selected, nblocalsurl, nbselected;

	srand ((unsigned) time (NULL));

	/* skip entries not in the form srm: or sfn:
	 * take entry on same domain if it exists else
	 * take the first supported entry
	 */
	localsurl = -1;
	selected = -1;
	nblocalsurl = 0;
	nbselected = 0;
	*dname = '\0';
	(void) getdomainnm (dname, sizeof(dname));

	/* and get the default SE, it there is one */
	default_se = get_default_se(errbuf, errbufsz);

	for (i = 0; i < size; i++) {
		p = surls[i];
		if (strncmp (p, "srm://", 6) && strncmp (p, "sfn://", 6))
			continue;
		if ((p1 = strchr (p + 6, '/')) == NULL) continue;
		if ((p2 = strchr (p + 6, '.')) == NULL) continue;
		*p1 = '\0';
		if ((p3 = strchr (p + 6, ':')))
			*p3 = '\0';
		default_match = -1;
		if(default_se != NULL) {
			default_match = strcmp(p + 6, default_se);
		}
		ret = strcmp (p2 + 1, dname);
		*p1 = '/';
		if (p3) *p3 = ':';
		if (default_match == 0) break; /* default se match => replica on default SE */
		if (ret == 0) {
			/* domains match ==> local replica */
			++nblocalsurl;
			localsurl = (rand() % nblocalsurl) == 0 ? i : localsurl;
		} else if (localsurl == -1) {
			++nbselected;
			selected = (rand() % nbselected) == 0 ? i : selected;
		}
	}
	if (i == size) {	/* no default SE entry */
		if (selected == -1 && localsurl == -1) {	/* only non suported entries */
			gfal_errmsg(errbuf, errbufsz, "Only non supported entries. No replica entry starting with \"srm://\" or \"sfn://\".");
			errno = EINVAL;
			return (NULL);
		} else if(localsurl >= 0)
			i = localsurl;
		else
			i = selected;
	}
	return surls[i];
}


	char *
get_default_se(char *errbuf, int errbufsz) 
{
	char *vo;
	char *default_se;
	int i;
	char se_env[64];
	char errmsg[ERRMSG_LEN];

	if((vo = gfal_get_vo (errbuf, errbufsz)) == NULL) {
		errno = EINVAL;
		return (NULL);
	}
	if(strlen(vo) + 15 >= 64) {
		snprintf (errmsg, ERRMSG_LEN - 1, "%s: VO name too long", vo);
		gfal_errmsg(errbuf, errbufsz, errmsg);
		errno = ENAMETOOLONG;
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

	static int
generate_surls (gfal_internal gfal, char *errbuf, int errbufsz)
{
	int i;
	uuid_t uuid;
	char guid[37];
	char *sa_path, *sa_root;
	char *vo, *ce_ap, *p, *q, *simple_ep;
	char dir_path[1104];
	char errmsg[ERRMSG_LEN];

	if ((gfal->surls = (char **) calloc (gfal->nbfiles, sizeof (char *))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	if ((vo = gfal_get_vo (errbuf, errbufsz)) == NULL) {
		errno = EINVAL;
		return (-1);
	}

	/* now create dir path which is either sa_path, sa_root or combination of ce_ap & sa_root */
	p = strchr (gfal->endpoint, ':');
	simple_ep = p == NULL ? gfal->endpoint : p + 3;
	if ((q = strchr (simple_ep, ':')) != NULL)
		*q = 0;
	if(get_sa_path (simple_ep, vo, &sa_path, &sa_root, errbuf, errbufsz) < 0) 
		return (-1);
	if(sa_path != NULL) {
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
	free (sa_path);
	free (sa_root);

	if (q) *q = ':';

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
		gfal_errmsg(errbuf, errbufsz, "'relative_path' is not compatible with multiple files");
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

gfal_init (gfal_request req, gfal_internal *gfal, char *errbuf, int errbufsz)
{
	int rc, i = 0;
	char **se_endpoints;
	char **se_types;
	char *srmv1_endpoint = NULL;
	char *srmv2_endpoint = NULL;
	int isclassicse = 0;
	char errmsg[ERRMSG_LEN];
	int endpoint_offset=0;

	if (req == NULL || req->nbfiles < 1 || (!req->generatesurls && req->surls == NULL)) {
		gfal_errmsg (errbuf, errbufsz, "Invalid request: No SURLs specified");
		errno = EINVAL;
		return (-1);
	}
	if (req->oflag != 0 && req->filesizes == NULL) {
		gfal_errmsg (errbuf, errbufsz, "Invalid request: File sizes must be specified for put requests");
		errno = EINVAL;
		return (-1);
	}
	if (req->srmv2_lslevels > 1) {
		gfal_errmsg (errbuf, errbufsz, "Invalid request: srmv2_lslevels must be 0 or 1");
		errno = EINVAL;
		return (-1);
	}

	if ((*gfal = (gfal_internal) malloc (sizeof (struct gfal_internal_))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	memset (*gfal, 0, sizeof (struct gfal_internal_));
	memcpy (*gfal, req, sizeof (struct gfal_request_));

	/* if no protocols, get the list of supported ones */
	if (!(*gfal)->protocols)
		(*gfal)->protocols = get_sup_proto ();

	if ((*gfal)->no_bdii_check) {
		if ((*gfal)->surls != NULL && ((*gfal)->setype != TYPE_NONE ||
					((*gfal)->setype = (*gfal)->defaultsetype) != TYPE_NONE)) {
			if ((*gfal)->setype == TYPE_SE) {
				gfal_internal_free (*gfal);
				*gfal = NULL;
				gfal_errmsg (errbuf, errbufsz, "Invalid request: Disabling BDII checks is not compatible with Classic SEs");
				errno = EINVAL;
				return (-1);
			}
			else if ((*gfal)->setype != TYPE_SE && (*gfal)->endpoint == NULL && ((*gfal)->free_endpoint = 1) &&
					((*gfal)->endpoint = endpointfromsurl ((*gfal)->surls[0], errbuf, errbufsz,1)) == NULL) {
				gfal_internal_free (*gfal);
				*gfal = NULL;
				return (-1);
			}
			else {
				/* Check if the endpoint is full or not */
				if(strncmp ((*gfal)->endpoint, ENDPOINT_DEFAULT_PREFIX, ENDPOINT_DEFAULT_PREFIX_LEN)==0)
					endpoint_offset=ENDPOINT_DEFAULT_PREFIX_LEN; 
				else
					endpoint_offset=0;
				const char *s = strchr ((*gfal)->endpoint+endpoint_offset, '/');
				const char *p = strchr ((*gfal)->endpoint+endpoint_offset, ':');

				if (((*gfal)->setype == TYPE_SRMv2 && s == NULL) || p == NULL || (s != NULL && s < p)) {
					gfal_internal_free (*gfal);
					*gfal = NULL;
					gfal_errmsg (errbuf, errbufsz, "Invalid request: When BDII checks are disabled, you must provide full endpoint");
					errno = EINVAL;
					return (-1);
				}

				return (0);
			}

		} else {
			gfal_internal_free (*gfal);
			*gfal = NULL;
			gfal_errmsg (errbuf, errbufsz, "Invalid request: When BDII checks are disabled, you must provide SURLs and endpoint type");
			errno = EINVAL;
			return (-1);
		}
	}

	if ((*gfal)->endpoint == NULL) {
		if ((*gfal)->surls != NULL) {
			if (((*gfal)->endpoint = endpointfromsurl ((*gfal)->surls[0], errbuf, errbufsz,0)) == NULL)
				return (-1);
			(*gfal)->free_endpoint = 1;
		} else {
			/* surls == NULL means that generatesurls == 1 */
			gfal_internal_free (*gfal);
			*gfal = NULL;
			gfal_errmsg (errbuf, errbufsz, "Invalid request: endpoint must be specified with 'generatesurls' activated");
			errno = EINVAL;
			return (-1);
		}
	}
	if ((strchr ((*gfal)->endpoint, '.') == NULL)) {
		gfal_errmsg(errbuf, errbufsz, "No domain name specified for storage element endpoint");
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

	/* Set SE type to the default one if possible */
	if ((*gfal)->defaultsetype == TYPE_SRM && !(*gfal)->srmv2_spacetokendesc &&
			!(*gfal)->srmv2_desiredpintime && !(*gfal)->srmv2_lslevels &&
			!(*gfal)->srmv2_lsoffset && !(*gfal)->srmv2_lscount &&
			srmv1_endpoint)
		(*gfal)->setype = TYPE_SRM;
	else if ((*gfal)->defaultsetype == TYPE_SRMv2 && srmv2_endpoint)
		(*gfal)->setype = TYPE_SRMv2;

	if (((*gfal)->setype == TYPE_NONE || (*gfal)->setype == TYPE_SRM) &&
			!(*gfal)->srmv2_spacetokendesc && !(*gfal)->srmv2_desiredpintime &&
			!(*gfal)->srmv2_lsoffset && !(*gfal)->srmv2_lscount &&
			!(*gfal)->srmv2_lslevels && srmv1_endpoint) {
		(*gfal)->setype = TYPE_SRM;
		(*gfal)->endpoint = srmv1_endpoint;
		if (srmv2_endpoint) free (srmv2_endpoint);
	} else if (((*gfal)->setype == TYPE_NONE || (*gfal)->setype == TYPE_SRMv2) && srmv2_endpoint) {
		(*gfal)->setype = TYPE_SRMv2;
		(*gfal)->endpoint = srmv2_endpoint;
		if (srmv1_endpoint) free (srmv1_endpoint);
	} else if (((*gfal)->setype == TYPE_NONE || (*gfal)->setype == TYPE_SE) && isclassicse) {
		(*gfal)->setype = TYPE_SE;
	} else {
		if (!srmv1_endpoint && !srmv2_endpoint && !isclassicse)
			snprintf (errmsg, ERRMSG_LEN - 1, "%s: Unknown SE in BDII", (*gfal)->surls[0]);
		else {
			snprintf (errmsg, ERRMSG_LEN - 1, "Invalid request: Desired SE type doesn't match request parameters or SE");
			if (srmv1_endpoint) free (srmv1_endpoint);
			if (srmv2_endpoint) free (srmv2_endpoint);
		}

		gfal_internal_free (*gfal);
		*gfal = NULL;
		gfal_errmsg (errbuf, errbufsz, errmsg);
		errno = EINVAL;
		return (-1);
	}

	if ((*gfal)->generatesurls) {
		if ((*gfal)->surls == NULL) {
			if (generate_surls (*gfal, errbuf, errbufsz) < 0)
				return (-1);
		} else {
			snprintf (errmsg, ERRMSG_LEN - 1, "No SURLs must be specified with 'generatesurls' activated");
			gfal_internal_free (*gfal);
			*gfal = NULL;
			gfal_errmsg (errbuf, errbufsz, errmsg);
			errno = EINVAL;
			return (-1);
		}
	}


	return (0);
}

	static int
check_gfal_internal (gfal_internal req, char *errbuf, int errbufsz)
{
	if (req == NULL || req->setype == TYPE_NONE || req->surls == NULL ||
			(req->setype != TYPE_SE && req->endpoint == NULL)) {
		gfal_errmsg (errbuf, errbufsz, "Invalid gfal_internal argument");
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
		if (gfal[i].surl) free (gfal[i].surl);
		if (gfal[i].turl) free (gfal[i].turl);
		if (gfal[i].explanation) free (gfal[i].explanation);
		if (gfal[i].subpaths) free_gfal_results (gfal[i].subpaths, gfal[i].nbsubpaths);
	}

	free (gfal);
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
	if (!req->results) {
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
		if (req->results[i].subpaths)
			free_gfal_results (req->results[i].subpaths, req->results[i].nbsubpaths);
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
			req->results[i].surl = req->sfn_statuses[i].surl;
			req->results[i].turl = req->sfn_statuses[i].turl;
			req->results[i].status = req->sfn_statuses[i].status;
		}
	}

	return (0);
}

	int
gfal_get_results (gfal_internal req, gfal_filestatus **results)
{
	if (req == NULL) {
		*results == NULL;
		return (-1);
	}

	*results = req->results;
	return (req->results_size);
}

	void
gfal_internal_free (gfal_internal req)
{
	int i;

	if (req == NULL)
		return;
	if (req->free_endpoint && req->endpoint)
		free (req->endpoint);
	if (req->sfn_statuses)
		free (req->sfn_statuses);
	if (req->srm_statuses)
		free (req->srm_statuses);
	if (req->srm_mdstatuses)
		free (req->srm_mdstatuses);
	if (req->srmv2_statuses)
		free (req->srmv2_statuses);
	if (req->srmv2_pinstatuses)
		free (req->srmv2_pinstatuses);
	if (req->srmv2_mdstatuses)
		free (req->srmv2_mdstatuses);

	if (req->generatesurls && req->surls) {
		for (i = 0; i < req->nbfiles; ++i)
			if (req->surls[i]) free (req->surls[i]);
		free (req->surls);
	}

	if (req->results || req->results_size > 0) {
		for (i = 0; i < req->results_size; ++i) {
			if (req->results[i].surl) free (req->results[i].surl);
			if (req->results[i].turl) free (req->results[i].turl);
			if (req->results[i].explanation) free (req->results[i].explanation);
		}

		free (req->results);
	}

	free (req);
	return;
}

const char *
gfal_version () {
	return gfalversion;
}
