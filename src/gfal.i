%module gfal

%{
#include "gfal_api.h"
%}

%include "lcg-typemaps.python.i"



/******************** POSIX functions ********************/

extern int gfal_access (const char *path, int amode);
extern int gfal_chmod (const char *path, mode_t mode);
extern int gfal_close (int fd);
extern int gfal_closedir (DIR *dir);
extern int gfal_creat (const char *filename, mode_t mode);
extern off_t gfal_lseek (int fd, off_t offset, int whence);
extern int gfal_lstat (const char *filename, struct stat *statbuf);
extern int gfal_mkdir (const char *dirname, mode_t mode);
extern int gfal_open (const char *filename, int flags, mode_t mode);
DIR *gfal_opendir (const char *dirname);
extern ssize_t gfal_read (int fd, void *BUFOUT, size_t BUFOUTSZ);
extern int gfal_rename (const char *old_name, const char *new_name);
extern int gfal_rmdir (const char *dirname);
extern ssize_t gfal_setfilchg (int fd, const void *buf, size_t size);
extern int gfal_stat (const char *filename, struct stat *statbuf);
extern int gfal_unlink (const char *filename);
extern ssize_t gfal_write (int fd, const void *BUFIN, size_t BUFINSZ);
#if defined(_LARGEFILE64_SOURCE)
extern int gfal_creat64 (const char *filename, mode_t mode);
extern off64_t gfal_lseek64 (int fd, off64_t offset, int whence);
extern int gfal_lstat64 (const char *filename, struct stat64 *statbuf);
extern int gfal_open64 (const char *filename, int flags, mode_t mode);
extern int gfal_stat64 (const char *filename, struct stat64 *statbuf);
extern struct dirent64 *gfal_readdir64 (DIR *dir);
#endif


/******************** Other functions ********************/

extern const char *gfal_version ();
extern int gfal_set_verbose (int value);
extern int gfal_set_vo (const char *vo);
extern void gfal_set_nobdii (int value);
extern void gfal_set_timeout_connect (int);
extern int gfal_get_timeout_connect ();
extern void gfal_set_timeout_sendreceive (int);
extern int gfal_get_timeout_sendreceive ();
extern void gfal_set_timeout_bdii (int);
extern int gfal_get_timeout_bdii ();
extern void gfal_set_timeout_srm (int);
extern int gfal_get_timeout_srm ();
extern int gfal_deletesurls (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_removedir (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_turlsfromsurls (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_ls (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_ls_end (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_get (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_getstatus (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_bringonline (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_prestage (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_prestagestatus (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_pin (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_release (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_set_xfer_done (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_set_xfer_running (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_abortrequest (gfal_internal req, char *errbuf, int errbufsz);
extern int gfal_abortfiles (gfal_internal req, char *errbuf, int errbufsz);
extern gfal_request gfal_request_new ();
extern int gfal_init (gfal_request req, gfal_internal *gfal, char *errbuf, int errbufsz);
extern int gfal_get_results (gfal_internal req, gfal_filestatus **statuses);
extern int gfal_get_ids_setype (gfal_internal req, enum se_type *setype, int *OUTPUT, int **OUTPUT, char **OUTPUT);
extern int gfal_get_ids (gfal_internal req, int *OUTPUT, int **OUTPUT, char **OUTPUT);
extern int gfal_set_ids (gfal_internal req, int LEN, const int *LIST, int srm_reqid, const char *srmv2_reqtoken, char *errbuf, int errbufsz);
extern void gfal_internal_free (gfal_internal req);
extern int gfal_get_errno ();


/******************** Additional stuffs ********************/

#ifdef SWIGPYTHON

%pythoncode %{
def gfal_readdir(dirobj):
    return gfal_readdir64(dirobj)

def gfal_listdir(dirpath):
    list = []
    dirobj = gfal_opendir(dirpath)
    dirent = gfal_readdir(dirobj)
    while dirent != None:
        list.append(dirent)
        dirent = gfal_readdir(dirobj)
    gfal_closedir(dirobj)
    return list
%}

#endif

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef unsigned int mode_t;
typedef unsigned int size_t;
typedef unsigned int ssize_t;
