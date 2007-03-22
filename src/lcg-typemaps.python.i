%{
static PyObject* my_t_output_helper(PyObject* target, PyObject* o) {
    PyObject*   o2;
    PyObject*   o3;

    if (!target) {
        target = o;
    } else {
        if (!PyTuple_Check(target)) {
            o2 = target;
            target = PyTuple_New(1);
            PyTuple_SetItem(target, 0, o2);
        }
        o3 = PyTuple_New(1);
        PyTuple_SetItem(o3, 0, o);

        o2 = target;
        target = PySequence_Concat(o2, o3);
        Py_DECREF(o2);
        Py_DECREF(o3);
    }
    return target;
}
%}

%include "typemaps.i"

// in python, Long are 64bits
%typemap(in) GFAL_LONG64 {
	if ($input == Py_None)
		$1 = 0;
	else
		$1 = (GFAL_LONG64) PyLong_AsLongLong ($input);
}

// if the string is empty, it is replaced by NULL
%typemap(in) char * {
	if ($input == Py_None) {
		$1 = NULL;
	} else {
		$1 = PyString_AsString ($input);
		if ($1 && $1[0] == 0) $1 = NULL;
	}
}

// convert python list into C string list (char**)
%typemap(in) char **protocols {
	int i,len;

	if ($input == Py_None || (len = PyList_Size ($input)) < 1) {
		errno = EINVAL;
		return (NULL);
	}
	$1 = (char**) calloc (len + 2, sizeof (char*));
	if ($1 == NULL) {
		errno = ENOMEM;
		return (NULL);
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

	if ($input == Py_None || ($1 = PyList_Size ($input)) < 1) {
		errno = EINVAL;
		errno = EINVAL;
		return (NULL);
	}
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

%typemap(in, numinputs=0) (int *OUTPUT)(int tmp) {
	tmp = 0;
	$1 = &tmp;
}

%typemap(argout) (int *OUTPUT){
	PyObject *o = PyInt_FromLong((long) (*$1));
	resultobj = my_t_output_helper(resultobj,o);
}//end of typemap

%typemap(in, numinputs=0) (char *errbuf, int errbufsz)(char err[256]) {
	err[0] = 0;
	$1 = err;
	$2 = 256;
}

%typemap(argout) (char *errbuf){
	PyObject *o;

	if (!$1[0] && errno) {
		char buf[256];
		buf[0] = 0;
		strerror_r (errno, buf, 256);
		o = PyString_FromString (buf);
	} else {
		o = PyString_FromString ($1);
	}

	$result = my_t_output_helper ($result, o);
}//end of typemap

%typemap(in, numinputs=0) (char *GUIDOUT)(char tmp_guid[40]) {
	tmp_guid[0] = 0;
	$1 = tmp_guid;
}

%typemap(argout) (char *GUIDOUT){
	PyObject *o = PyString_FromString ($1);
	$result = my_t_output_helper ($result, o);
}//end of typemap

%typemap(in, numinputs=0) (char **OUTPUT)(char *tmp_char) {
	tmp_char = NULL;
	$1 = &tmp_char;
}

// convert output C string into python string
%typemap(argout) (char **OUTPUT){
	PyObject *o = Py_None;

	if (*$1) {
		o = PyString_FromString (*$1);
	}
	$result = my_t_output_helper ($result, o);
}//end of typemap

%typemap(in, numinputs=0) (char ***OUTPUT)(char **tmp_tabchar) {
	tmp_tabchar = NULL;
	$1 = &tmp_tabchar;
}

// convert output C string list into python list
%typemap(argout) (char ***OUTPUT){
	PyObject *o = Py_None;
	int i;

	if (*$1) {
		o = PyList_New (0);
		for (i = 0; (*$1)[i]; ++i) {
			PyList_Append (o, PyString_FromString ((*$1)[i]));
		}
	}

	$result = my_t_output_helper ($result, o);
}//end of typemap

%typemap(in, numinputs=0) (struct srm_filestatus **filestatuses)(struct srm_filestatus *filestatus_tmp) {
	filestatus_tmp = NULL;
	$1 = &filestatus_tmp;
}

// convert output C 'struct srm_filestatus' list into python dictionnary
%typemap(argout) (struct srm_filestatus **filestatuses){
	PyObject *list = Py_None, *dict = Py_None;
	int i;

	if (*$1) {
		list = PyList_New (0);
		for (i = 0; i < arg1; ++i) {
			dict = PyDict_New ();
			PyDict_SetItemString (dict, "surl", (*$1)[i].surl ? PyString_FromString ((*$1)[i].surl) : Py_None);
			PyDict_SetItemString (dict, "turl", (*$1)[i].turl ? PyString_FromString ((*$1)[i].turl) : Py_None);
			PyDict_SetItemString (dict, "fileid", PyInt_FromLong ((long)((*$1)[i].fileid)));
			PyDict_SetItemString (dict, "status", PyInt_FromLong ((long)((*$1)[i].status)));
			PyList_Append (list, dict);
		}
	}

	$result = my_t_output_helper ($result, list);
}//end of typemap

%typemap(in, numinputs=0) (struct srmv2_filestatus **filestatuses)(struct srmv2_filestatus *filestatus_tmp) {
	filestatus_tmp = NULL;
	$1 = &filestatus_tmp;
}

// convert output C 'struct srmv2_filestatus' list into python dictionnary
%typemap(argout) (struct srmv2_filestatus **filestatuses){
	PyObject *list = Py_None, *dict = Py_None;
	int i;

	if (*$1) {
		list = PyList_New (0);
		for (i = 0; i < arg1; ++i) {
			dict = PyDict_New ();
			PyDict_SetItemString (dict, "surl", (*$1)[i].surl ? PyString_FromString ((*$1)[i].surl) : Py_None);
			// PyDict_SetItemString (dict, "turl", (*$1)[i].turl ? PyString_FromString ((*$1)[i].turl) : Py_None);
			PyDict_SetItemString (dict, "status", PyInt_FromLong ((long)((*$1)[i].status)));
			PyDict_SetItemString (dict, "explanation", (*$1)[i].explanation ? PyString_FromString ((*$1)[i].explanation) : Py_None);
			PyList_Append (list, dict);
		}
	}

	$result = my_t_output_helper ($result, list);
}//end of typemap

%typemap(in, numinputs=0) (struct stat64 *)(struct stat64 statbuf) {
	$1 = &statbuf;
	bzero ($1, sizeof (struct stat64));
}

// convert output C 'struct stat64' into a python list exactly (in the same order) as the system os.stat() function
%typemap(argout) (struct stat64 *){
	PyObject *statlist = Py_None;

	if ($1) {
		statlist = PyList_New (10);
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

	$result = my_t_output_helper ($result, statlist);
}//end of typemap
