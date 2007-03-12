%module gfal

%{
#include "gfal_api.h"
%}

%include "typemaps.i"

// in python, Long are 64bits
%typemap(in) GFAL_LONG64 {
	$1 = (GFAL_LONG64) PyLong_AsLongLong ($input);
}

// if the string is empty, it is replaced by NULL
%typemap(in) char * {
	$1 = PyString_AsString ($input);
	if ($1[0] == 0) {
		$1 = NULL;
	}
}

// convert python list into C string list (char**)
%typemap(in) char **protocols {
	int i,len;

	len = PyList_Size ($input);
	$1 = (char**) calloc (len + 2, sizeof (char*));
	if ($1 == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	for (i = 0; i < len; ++i) {
		$1[i] = PyString_AsString (PyList_GetItem ($input, i));
	}

	$1[i++] = "";
	$1[i] = NULL;
}

// Feed list length and C-style list from python list
%typemap(in) (int LEN, char **LIST) {
	int i;

	$1 = PyList_Size ($input);
	$2 = (char**) calloc ($1 + 1, sizeof (char*));
	if ($2 == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	for (i = 0; i < $1; ++i) {
		$2[i] = PyString_AsString (PyList_GetItem ($input, i));
	}

	$2[i] = NULL;
}

%typemap(in, numinputs=0) (char *errbuf, int errbufsz)(char err[512]) {
	err[0] = 0;
	$1 = err;
	$2 = 512;
}

%typemap(argout) (char *errbuf){
	PyObject *o = PyString_FromString ($1);
	$result = t_output_helper ($result, o);
}//end of typemap

%typemap(in, numinputs=0) (char *GUIDOUT)(char tmp_guid[40]) {
	$1 = tmp_guid;
}

%typemap(argout) (char *GUIDOUT){
	PyObject *o = PyString_FromString ($1);
	$result = t_output_helper ($result, o);
}//end of typemap

%typemap(in, numinputs=0) (char **OUTPUT)(char *tmp_char) {
	$1 = &tmp_char;
}

// convert output C string into python string
%typemap(argout) (char **OUTPUT){
	PyObject *o;

	if (*$1 == NULL) {
		o = Py_None;
	} else {
		o = PyString_FromString (*$1);
	}
	$result = t_output_helper ($result, o);
}//end of typemap

%typemap(in, numinputs=0) (char ***OUTPUT)(char **tmp_tabchar) {
	$1 = &tmp_tabchar;
}

// convert output C string list into python list
%typemap(argout) (char ***OUTPUT){
	PyObject *o = PyList_New (0);
	int i;

	if (*$1) {
		for (i = 0; (*$1)[i]; ++i) {
			PyList_Append (o, PyString_FromString ((*$1)[i]));
		}
	}

	$result = t_output_helper ($result, o);
}//end of typemap

%typemap(in, numinputs=0) (struct srm_filestatus **filestatuses)(struct srm_filestatus *filestatus_tmp) {
	$1 = &filestatus_tmp;
}

// convert output C 'struct srm_filestatus' list into python dictionnary
%typemap(argout) (struct srm_filestatus **filestatuses){
	PyObject *list, *dict;
	int i;
	list = PyList_New (0);

	if (*$1) {
		for (i = 0; i < arg1; ++i) {
			dict = PyDict_New ();
			PyDict_SetItemString (dict, "surl", PyString_FromString ((*$1)[i].surl));
			PyDict_SetItemString (dict, "turl", PyString_FromString ((*$1)[i].turl));
			PyDict_SetItemString (dict, "fileid", PyInt_FromLong ((long)((*$1)[i].fileid)));
			PyDict_SetItemString (dict, "status", PyInt_FromLong ((long)((*$1)[i].status)));
			PyList_Append (list, dict);
		}
	}

	$result = t_output_helper ($result, list);
}//end of typemap

%typemap(in, numinputs=0) (struct srmv2_filestatus **filestatuses)(struct srmv2_filestatus *filestatus_tmp) {
	$1 = &filestatus_tmp;
}

// convert output C 'struct srmv2_filestatus' list into python dictionnary
%typemap(argout) (struct srmv2_filestatus **filestatuses){
	PyObject *list, *dict;
	int i;
	list = PyList_New (0);

	if (*$1) {
		for (i = 0; i < arg1; ++i) {
			dict = PyDict_New ();
			PyDict_SetItemString (dict, "surl", PyString_FromString ((*$1)[i].surl));
			PyDict_SetItemString (dict, "turl", PyString_FromString ((*$1)[i].turl));
			PyDict_SetItemString (dict, "status", PyInt_FromLong ((long)((*$1)[i].status)));
			PyDict_SetItemString (dict, "explanation", PyString_FromString ((*$1)[i].explanation));
			PyList_Append (list, dict);
		}
	}

	$result = t_output_helper ($result, list);
}//end of typemap

%typemap(in, numinputs=0) (struct stat64 *)(struct stat64 statbuf) {
	$1 = &statbuf;
	bzero ($1, sizeof (struct stat64));
}

// convert output C 'struct stat64' into a python list exactly (in the same order) as the system os.stat() function
%typemap(argout) (struct stat64 *){
	PyObject *statlist;
	statlist = PyList_New (10);

	if ($1) {
		PyList_SetItem (statlist, 0, PyInt_FromLong ((long)((*$1).st_mode)));
		PyList_SetItem (statlist, 1, PyLong_FromLongLong ((long long)((*$1).st_ino)));
		PyList_SetItem (statlist, 2, PyInt_FromLong ((long)((*$1).st_dev)));
		PyList_SetItem (statlist, 3, PyInt_FromLong ((long)((*$1).st_nlink)));
		PyList_SetItem (statlist, 4, PyInt_FromLong ((long)((*$1).st_uid)));
		PyList_SetItem (statlist, 5, PyInt_FromLong ((long)((*$1).st_gid)));
		PyList_SetItem (statlist, 6, PyLong_FromLongLong ((long long)((*$1).st_size)));
		PyList_SetItem (statlist, 7, PyLong_FromLong ((long)((*$1).st_atime)));
		PyList_SetItem (statlist, 8, PyLong_FromLong ((long)((*$1).st_mtime)));
		PyList_SetItem (statlist, 9, PyLong_FromLong ((long)((*$1).st_ctime)));
	}

	$result = t_output_helper ($result, statlist);
}//end of typemap


extern int deletesurl (const char *surl, char *errbuf, int errbufsz, int timeout);
extern int deletesurl2 (const char *surl, char *spacetokendesc, char *errbuf,
			int errbufsz, int timeout);

extern int getfilemd (const char *surl, struct stat64 *statbuf, char *errbuf,
			int errbufsz, int timeout);

extern int set_xfer_done (const char *surl, int reqid, int fileid, char *token,
			int oflag, char *errbuf, int errbufsz, int timeout);
extern int set_xfer_running (const char *surl, int reqid, int fileid, char *token,
			char *errbuf, int errbufsz, int timeout);

extern char *turlfromsurl (const char *surl, char **protocols, int oflag, int *OUTPUT,
			int *OUTPUT, char **OUTPUT, char *errbuf, int errbufsz,
			int timeout);
extern char *turlfromsurlx (const char *surl, GFAL_LONG64 filesize, char **protocols,
			int oflag, int *OUTPUT, int *OUTPUT, char **OUTPUT, char *errbuf,
			int errbufsz, int timeout);
extern char *turlfromsurl2 (const char *surl, GFAL_LONG64 filesize,
			const char *spacetokendesc, char **protocols, int oflag,
			int *OUTPUT, int *OUTPUT, char **OUTPUT, char *errbuf, int errbufsz,
			int timeout);

extern int srm_get (int LEN, char **LIST, int LEN, char **LIST, int *OUTPUT, char **OUTPUT,
			struct srm_filestatus **filestatuses, int timeout);
extern int srm_getx (int LEN, char **LIST, int LEN, char **LIST, int *OUTPUT, char **OUTPUT,
			struct srm_filestatus **filestatuses, char *errbuf, int errbufsz,
			int timeout);
extern int srm_getstatus (int LEN, char **LIST, int reqid, char *token,
			struct srm_filestatus **filestatuses, int timeout);
extern int srm_getstatusx (int LEN, char **LIST, int reqid, char *token,
			struct srm_filestatus **filestatuses, char *errbuf, int errbufsz,
			int timeout);

extern int srmv2_prestage (int LEN, char **LIST, char *spacetokendesc, int LEN,	char **LIST,
			char **OUTPUT, struct srmv2_filestatus **filestatuses,
			char *errbuf, int errbufsz, int timeout);
extern int srmv2_prestagestatus (int LEN, char **LIST, char *reqtoken,
			struct srmv2_filestatus **filestatuses, char *errbuf, int errbufsz,
			int timeout);
