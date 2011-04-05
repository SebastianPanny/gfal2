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
 * @(#)$RCSfile: gridftp_ifce.c,v $ $Revision: 1.5 $ $Date: 2009/02/25 13:38:08 $ CERN Remi Mollon
 */


#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "globus_ftp_client.h"
#include "gfal_internals.h"

#ifdef GLOBUS_ERROR_TYPE_MULTIPLE
#define GLOBUS_ERROR2STRING(errorobj) globus_error_print_friendly (errorobj)
#else
#define GLOBUS_ERROR2STRING(errorobj) globus_object_printable_to_string (errorobj)
#endif

#define BUFFER_SIZE 4096
#define DEFAULT_STAT_SIZE 256


typedef struct {
	globus_mutex_t mutex;
	globus_cond_t cond;
	volatile globus_bool_t done;
	volatile globus_bool_t errflag;
	globus_object_t *error;
} monitor_t;

typedef struct {
	globus_mutex_t mutex;
	globus_cond_t cond;
	volatile globus_bool_t done;
	volatile globus_bool_t errflag;
	globus_object_t *error;
	globus_byte_t buffer[BUFFER_SIZE];
	int posbuf;
	char **filenames;
	struct stat64 *statbufs;
	int nbfiles;
	int pos;
} monitor_stat_t;


static volatile int globus_activated = 0;

	static int
do_globus_activates(void)
{
	/* not thread safe */
	if (!globus_activated) {
		globus_activated = 1;
		/* (void) globus_module_activate (GLOBUS_GASS_COPY_MODULE); */
		(void) globus_module_activate (GLOBUS_FTP_CLIENT_MODULE);
	}

	return GLOBUS_SUCCESS;
}

	static int
do_globus_deactivates(void)
{
	return GLOBUS_SUCCESS;
}



	static int
gftp_client_wait(monitor_t *mp, globus_ftp_client_handle_t *gfhp, const int timeout)
{
	struct timespec ts;
	int save_errno = 0;

	globus_mutex_lock (&mp->mutex);

	if (timeout) {
		ts.tv_sec = time (0) + timeout;
		ts.tv_nsec = 0;
		while (! mp->done && !save_errno) {
			save_errno = globus_cond_timedwait (&mp->cond, &mp->mutex, &ts);
			if (save_errno == EINTR)
				save_errno = 0;
		}
	} else
		while (! mp->done && !save_errno) {
			save_errno = globus_cond_wait (&mp->cond, &mp->mutex);
			if (save_errno == EINTR)
				save_errno = 0;
		}
	if (!mp->done) {
		int ret;
		globus_ftp_client_abort (gfhp);
		do {
			ret = globus_cond_wait (&mp->cond, &mp->mutex);
			if (ret == EINTR)
				ret = 0;
		} while(!mp->done && !ret);
		if (ret && !save_errno)
			save_errno = ret;
	}

	globus_mutex_unlock (&mp->mutex);

	if (save_errno) {
		errno = save_errno;
		return(-1);
	}
	return(0);
}

static int
scan_errstring(const char *p) {
	int ret = EINVAL;

	if (p == NULL) return ret;

	if (strstr (p, "o such file"))
		ret = ENOENT;
	else if (strstr (p, "ermission denied") || strstr (p, "credential"))
		ret = EACCES;
	else if (strstr (p, "exists"))
		ret = EEXIST;

	return ret;
}

	static void
gcallback (void *callback_arg, globus_ftp_client_handle_t *ftp_handle, globus_object_t *error)
{
	monitor_t *monitor = (monitor_t *) callback_arg;
	globus_mutex_lock (&monitor->mutex);
	if (error != GLOBUS_SUCCESS) {
		monitor->errflag = GLOBUS_TRUE;
		monitor->error = globus_object_copy (error);
	}
	monitor->done = GLOBUS_TRUE;
	globus_cond_signal (&monitor->cond);
	globus_mutex_unlock (&monitor->mutex);
}

/*
 * An optional callback function any necessary i/o operations that have to
 * happen after the operation request.
 * e.g. for read after a LIST operation request
 */ 
static void
data_callback (void *callback_arg, globus_ftp_client_handle_t *handle, globus_object_t *error,
		globus_byte_t *buffer, globus_size_t length, globus_off_t offset, globus_bool_t eof)
{
	char *endline, *space;
	monitor_stat_t *monitor = (monitor_stat_t *) callback_arg;
	globus_mutex_lock(&monitor->mutex);

	if (error != GLOBUS_SUCCESS) {
		monitor->errflag = GLOBUS_TRUE;
		monitor->error = globus_object_copy (error);
	}

	/* Print the result if there was no error. */
	if (!monitor->errflag) {
		/* let's parse what we got in the monitor->buffer */
		length += monitor->posbuf;
		monitor->posbuf = 0;
		monitor->buffer[length] = '\0';

		while ((endline = strchr (monitor->buffer + monitor->posbuf, '\n')) != NULL) {
			if (monitor->pos == monitor->nbfiles) {
				monitor->nbfiles *= 2;
				monitor->filenames = (char **) realloc (monitor->filenames, monitor->nbfiles * sizeof (char *));
				monitor->statbufs = (struct stat64 *) realloc (monitor->statbufs, monitor->nbfiles * sizeof (struct stat64));
				if (monitor->filenames == NULL || monitor->statbufs == NULL) {
					monitor->errflag = GLOBUS_TRUE;
					break;
				}
			}

			/* get file type */
			if (monitor->buffer[monitor->posbuf] == 'd')
				monitor->statbufs[monitor->pos].st_mode |= S_IFDIR;
			else if (monitor->buffer[monitor->posbuf] == 'l')
				monitor->statbufs[monitor->pos].st_mode |= S_IFLNK;
			else
				monitor->statbufs[monitor->pos].st_mode |= S_IFREG;

			/* get file permissions */
			++monitor->posbuf;
			if (monitor->buffer[monitor->posbuf++] == 'r')
				monitor->statbufs[monitor->pos].st_mode |= S_IRUSR;
			if (monitor->buffer[monitor->posbuf++] == 'w')
				monitor->statbufs[monitor->pos].st_mode |= S_IWUSR;
			if (monitor->buffer[monitor->posbuf++] == 'x')
				monitor->statbufs[monitor->pos].st_mode |= S_IXUSR;
			if (monitor->buffer[monitor->posbuf++] == 'r')
				monitor->statbufs[monitor->pos].st_mode |= S_IRGRP;
			if (monitor->buffer[monitor->posbuf++] == 'w')
				monitor->statbufs[monitor->pos].st_mode |= S_IWGRP;
			if (monitor->buffer[monitor->posbuf++] == 'x')
				monitor->statbufs[monitor->pos].st_mode |= S_IXGRP;
			if (monitor->buffer[monitor->posbuf++] == 'r')
				monitor->statbufs[monitor->pos].st_mode |= S_IROTH;
			if (monitor->buffer[monitor->posbuf++] == 'w')
				monitor->statbufs[monitor->pos].st_mode |= S_IWOTH;
			if (monitor->buffer[monitor->posbuf++] == 'x')
				monitor->statbufs[monitor->pos].st_mode |= S_IXOTH;

			/* get the link number */
			while (monitor->buffer[monitor->posbuf] == ' ' || monitor->buffer[monitor->posbuf] == '\t') ++monitor->posbuf; /* skip spaces */
			if ((space = strchr (monitor->buffer + monitor->posbuf, ' ')) == NULL) {
				monitor->errflag = GLOBUS_TRUE;
				break;
			}

			*space = '\0';
			monitor->statbufs[monitor->pos].st_nlink = atoi (monitor->buffer + monitor->posbuf);
			monitor->statbufs[monitor->pos].st_uid = 2;
			monitor->statbufs[monitor->pos].st_gid = 2;
			monitor->posbuf = (space - (char *)monitor->buffer) + 1;

			/* skip the user name field */
			while (monitor->buffer[monitor->posbuf] == ' ' || monitor->buffer[monitor->posbuf] == '\t') ++monitor->posbuf; /* skip spaces */
			if ((space = strchr (monitor->buffer + monitor->posbuf, ' ')) == NULL) {
				monitor->errflag = GLOBUS_TRUE;
				break;
			}
			monitor->posbuf = (space - (char *)monitor->buffer) + 1;

			/* skip the group name field */
			while (monitor->buffer[monitor->posbuf] == ' ' || monitor->buffer[monitor->posbuf] == '\t') ++monitor->posbuf; /* skip spaces */
			if ((space = strchr (monitor->buffer + monitor->posbuf, ' ')) == NULL) {
				monitor->errflag = GLOBUS_TRUE;
				break;
			}
			monitor->posbuf = (space - (char *)monitor->buffer) + 1;

			/* get the file size */
			while (monitor->buffer[monitor->posbuf] == ' ' || monitor->buffer[monitor->posbuf] == '\t') ++monitor->posbuf; /* skip spaces */
			if ((space = strchr (monitor->buffer + monitor->posbuf, ' ')) == NULL) {
				monitor->errflag = GLOBUS_TRUE;
				break;
			}

			*space = '\0';
			monitor->statbufs[monitor->pos].st_size = (GFAL_LONG64) atoll (monitor->buffer + monitor->posbuf);
			monitor->posbuf = (space - (char *)monitor->buffer) + 1;

			/* skip date and time fields (month day time) */
			while (monitor->buffer[monitor->posbuf] == ' ' || monitor->buffer[monitor->posbuf] == '\t') ++monitor->posbuf; /* skip spaces */
			if ((space = strchr (monitor->buffer + monitor->posbuf, ' ')) == NULL) {
				monitor->errflag = GLOBUS_TRUE;
				break;
			}
			monitor->posbuf = (space - (char *)monitor->buffer) + 1;
			while (monitor->buffer[monitor->posbuf] == ' ' || monitor->buffer[monitor->posbuf] == '\t') ++monitor->posbuf; /* skip spaces */
			if ((space = strchr (monitor->buffer + monitor->posbuf, ' ')) == NULL) {
				monitor->errflag = GLOBUS_TRUE;
				break;
			}
			monitor->posbuf = (space - (char *)monitor->buffer) + 1;
			while (monitor->buffer[monitor->posbuf] == ' ' || monitor->buffer[monitor->posbuf] == '\t') ++monitor->posbuf; /* skip spaces */
			if ((space = strchr (monitor->buffer + monitor->posbuf, ' ')) == NULL) {
				monitor->errflag = GLOBUS_TRUE;
				break;
			}
			monitor->posbuf = (space - (char *)monitor->buffer) + 1;

			/* get the file name */
			while (monitor->buffer[monitor->posbuf] == ' ' || monitor->buffer[monitor->posbuf] == '\t') ++monitor->posbuf; /* skip spaces */
			*endline = '\0';
			monitor->filenames[monitor->pos] = strdup (monitor->buffer + monitor->posbuf);
			monitor->posbuf = (endline - (char *) monitor->buffer) + 1;

			++monitor->pos;
		}

		memmove (monitor->buffer, monitor->buffer + monitor->posbuf, length - monitor->posbuf);
		monitor->posbuf = length - monitor->posbuf;
	}

	/* Check for end of file. */
	if (eof) monitor->done = GLOBUS_TRUE;

	globus_cond_signal(&monitor->cond);
	globus_mutex_unlock(&monitor->mutex);

	if (!eof && !monitor->errflag) {
		globus_result_t gresult;

		/* Start another read operation. */
		gresult = globus_ftp_client_register_read(handle,
				monitor->buffer + monitor->posbuf,
				BUFFER_SIZE - monitor->posbuf - 1,
				&data_callback,
				monitor);

		if (gresult != GLOBUS_SUCCESS ) {
			monitor->errflag = GLOBUS_TRUE;
			monitor->error = globus_error_get(gresult);
		}
	}

}

	int
gridftp_delete (char *file, char *errbuf, int errbufsz, int timeout)
{
	globus_ftp_client_handle_t ftp_handle;
	globus_ftp_client_handleattr_t ftp_handleattr;
	globus_ftp_client_operationattr_t ftp_op_attr;
	globus_result_t gresult;
	monitor_t monitor;
	char *p;
	int rc;
	int save_errno = 0;

	globus_mutex_init (&monitor.mutex, NULL);
	globus_cond_init (&monitor.cond, NULL);
	monitor.done = GLOBUS_FALSE;
	monitor.errflag = GLOBUS_FALSE;
	rc = do_globus_activates();
	globus_ftp_client_handleattr_init (&ftp_handleattr);
	globus_ftp_client_handle_init (&ftp_handle, &ftp_handleattr);
	globus_ftp_client_operationattr_init (&ftp_op_attr);
	gresult = globus_ftp_client_delete (&ftp_handle, file, &ftp_op_attr,
			&gcallback, &monitor);

	/* check the state immdiately */	
	if (gresult != GLOBUS_SUCCESS ) {
		globus_object_t *errobj;
		monitor.errflag = GLOBUS_TRUE;
		errobj = globus_error_get(gresult);
		p = GLOBUS_ERROR2STRING (errobj);
		globus_object_free(errobj);
		goto err;
	}

	if (gftp_client_wait(&monitor, &ftp_handle, timeout))
		save_errno = errno;

	if (monitor.errflag) {
		int err;
		p = GLOBUS_ERROR2STRING (monitor.error);
		globus_object_free (monitor.error);
err:
		err = scan_errstring(p);
		if (err == EEXIST)
			err = EINVAL;

		if (p != NULL) {
			if (err != ENOENT && !save_errno)
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GRIDFTP][globus_ftp_client_delete][] %s", p);

			globus_libc_free(p);
		}
		if (!save_errno)
			save_errno = err;
	}

	globus_mutex_destroy(&monitor.mutex);
	globus_cond_destroy(&monitor.cond);
	globus_ftp_client_operationattr_destroy (&ftp_op_attr);
	globus_ftp_client_handle_destroy (&ftp_handle);
	globus_ftp_client_handleattr_destroy (&ftp_handleattr);
	(void) do_globus_deactivates();

	if (monitor.errflag == 0 && !save_errno)
		return (0);

	errno = save_errno;
	return (-1);
}

	int
gridftp_ls (char *path, int *nbfiles, char ***filenames, struct stat64 **statbufs, char *errbuf, int errbufsz, int timeout)
{
	globus_ftp_client_handle_t ftp_handle;
	globus_ftp_client_handleattr_t ftp_handleattr;
	globus_ftp_client_operationattr_t ftp_op_attr;
	globus_result_t gresult;
	monitor_t monitor;
	monitor_stat_t data_monitor;
	char *p = NULL;
	int rc;
	int save_errno = 0;

	if (path == NULL || filenames == NULL || statbufs == NULL) {
		gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GFAL][gridftp_ls][] Invalid arguments");
		errno = EINVAL;
		return (-1);
	}

	*nbfiles = 0;
	*filenames = NULL;
	*statbufs = NULL;

	globus_mutex_init (&monitor.mutex, NULL);
	globus_cond_init (&monitor.cond, NULL);
	monitor.done = GLOBUS_FALSE;
	monitor.errflag = GLOBUS_FALSE;

	globus_mutex_init (&data_monitor.mutex, NULL);
	globus_cond_init (&data_monitor.cond, NULL);
	data_monitor.done = GLOBUS_FALSE;
	data_monitor.errflag = GLOBUS_FALSE;
	data_monitor.nbfiles = DEFAULT_STAT_SIZE;
	data_monitor.posbuf = 0;
	data_monitor.pos = 0;
	data_monitor.filenames = (char **) calloc (data_monitor.nbfiles, sizeof (char *));
	data_monitor.statbufs = (struct stat64 *) calloc (data_monitor.nbfiles, sizeof (struct stat64));
	if (data_monitor.filenames == NULL || data_monitor.statbufs == NULL) {
		save_errno = errno;
		goto err;
	}

	rc = do_globus_activates();
	globus_ftp_client_handleattr_init (&ftp_handleattr);
	globus_ftp_client_handle_init (&ftp_handle, &ftp_handleattr);
	globus_ftp_client_operationattr_init (&ftp_op_attr);
	gresult = globus_ftp_client_verbose_list (&ftp_handle, path, &ftp_op_attr,
			&gcallback, &monitor);

	/* check the state immdiately */
	if (gresult != GLOBUS_SUCCESS ) {
		globus_object_t *errobj;
		monitor.errflag = GLOBUS_TRUE;
		errobj = globus_error_get(gresult);
		p = GLOBUS_ERROR2STRING (errobj);
		globus_object_free(errobj);
		goto err;
	}

	/* Loop until all the data has been read (or error). */
	/* Start a read operation. */
	gresult = globus_ftp_client_register_read(&ftp_handle,
			data_monitor.buffer,
			BUFFER_SIZE-1,
			&data_callback,
			(void *) &data_monitor);

	/* check the state immdiately */
	if (gresult != GLOBUS_SUCCESS ) {
		globus_object_t *errobj;
		monitor.errflag = GLOBUS_TRUE;
		errobj = globus_error_get(gresult);
		p = GLOBUS_ERROR2STRING (errobj);
		globus_object_free(errobj);
		goto err;
	}

	if (gftp_client_wait((monitor_t *) &data_monitor, &ftp_handle, timeout))
		save_errno = errno;

	if (gftp_client_wait(&monitor, &ftp_handle, timeout))
		save_errno = !save_errno ? errno : save_errno;

	if (monitor.errflag) {
		int err;
		p = GLOBUS_ERROR2STRING (monitor.error);
		globus_object_free (monitor.error);
err:
		if (p) {
			err = scan_errstring(p);
			if (err == EEXIST)
				err = EINVAL;

			if (err != ENOENT && !save_errno)
				gfal_errmsg (errbuf, errbufsz, GFAL_ERRLEVEL_ERROR, "[GRIDFTP][globus_ftp_client_verbose_list][] %s", p);

			globus_libc_free(p);
			if (!save_errno)
				save_errno = err;
		}
	}

	globus_mutex_destroy(&monitor.mutex);
	globus_cond_destroy(&monitor.cond);
	globus_mutex_destroy(&data_monitor.mutex);
	globus_cond_destroy(&data_monitor.cond);
	globus_ftp_client_operationattr_destroy (&ftp_op_attr);
	globus_ftp_client_handle_destroy (&ftp_handle);
	globus_ftp_client_handleattr_destroy (&ftp_handleattr);
	(void) do_globus_deactivates();

	if (!monitor.errflag && !data_monitor.errflag && !save_errno) {
		*nbfiles = data_monitor.pos;
		*filenames = data_monitor.filenames;
		*statbufs = data_monitor.statbufs;

		*filenames = realloc (*filenames, (*nbfiles + 1) * sizeof (char *));
		(*filenames)[*nbfiles] = NULL;
		*statbufs = realloc (*statbufs, *nbfiles * sizeof (struct stat64));
	} else {
		if (data_monitor.filenames) free (data_monitor.filenames);
		if (data_monitor.statbufs) free (data_monitor.statbufs);
	}

	errno = save_errno;
	return (monitor.errflag || data_monitor.errflag || save_errno ? -1 : 0);
}
