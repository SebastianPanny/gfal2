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

static PyObject* gfalresults_2_python (gfal_filestatus *filestatuses, int nb) {
	PyObject *list = Py_None, *dict = Py_None;
	int i;

	if (filestatuses) {
		list = PyList_New (0);
		for (i = 0; i < nb; ++i) {
			dict = PyDict_New ();
			PyDict_SetItemString (dict, "surl", filestatuses[i].surl ? PyString_FromString (filestatuses[i].surl) : Py_None);
			PyDict_SetItemString (dict, "turl", filestatuses[i].turl ? PyString_FromString (filestatuses[i].turl) : Py_None);
			PyDict_SetItemString (dict, "status", PyInt_FromLong ((long)(filestatuses[i].status)));
			PyDict_SetItemString (dict, "explanation", filestatuses[i].explanation ? PyString_FromString (filestatuses[i].explanation) : Py_None);
			if (filestatuses[i].pinlifetime > 0)
				PyDict_SetItemString (dict, "pinlifetime", PyInt_FromLong ((long)(filestatuses[i].pinlifetime)));
			if (filestatuses[i].locality > 0) {
				switch (filestatuses[i].locality) {
					case GFAL_LOCALITY_ONLINE_:
						PyDict_SetItemString (dict, "locality", PyString_FromString ("ONLINE"));
						break;
					case GFAL_LOCALITY_NEARLINE_:
						PyDict_SetItemString (dict, "locality", PyString_FromString ("NEARLINE"));
						break;
					case GFAL_LOCALITY_ONLINE_USCOREAND_USCORENEARLINE:
						PyDict_SetItemString (dict, "locality", PyString_FromString ("ONLINE_AND_NEARLINE"));
						break;
					case GFAL_LOCALITY_LOST:
						PyDict_SetItemString (dict, "locality", PyString_FromString ("LOST"));
						break;
					case GFAL_LOCALITY_NONE_:
						PyDict_SetItemString (dict, "locality", PyString_FromString ("NONE"));
						break;
					case GFAL_LOCALITY_UNAVAILABLE:
						PyDict_SetItemString (dict, "locality", PyString_FromString ("UNAVAILABLE"));
						break;
					default:
						PyDict_SetItemString (dict, "locality", PyString_FromString ("UNKNOWN"));
				}
			}
			if (filestatuses[i].stat.st_mode > 0) {
				PyObject *statlist = PyList_New (10);
				PyList_SetItem (statlist, 0, PyInt_FromLong ((long)(filestatuses[i].stat.st_mode)));
				PyList_SetItem (statlist, 1, PyLong_FromLongLong ((long long)(filestatuses[i].stat.st_ino)));
				PyList_SetItem (statlist, 2, PyInt_FromLong ((long)(filestatuses[i].stat.st_dev)));
				PyList_SetItem (statlist, 3, PyInt_FromLong ((long)(filestatuses[i].stat.st_nlink)));
				PyList_SetItem (statlist, 4, PyInt_FromLong ((long)(filestatuses[i].stat.st_uid)));
				PyList_SetItem (statlist, 5, PyInt_FromLong ((long)(filestatuses[i].stat.st_gid)));
				PyList_SetItem (statlist, 6, PyLong_FromLongLong ((long long)(filestatuses[i].stat.st_size)));
				PyList_SetItem (statlist, 7, PyLong_FromLong ((long)(filestatuses[i].stat.st_atime)));
				PyList_SetItem (statlist, 8, PyLong_FromLong ((long)(filestatuses[i].stat.st_mtime)));
				PyList_SetItem (statlist, 9, PyLong_FromLong ((long)(filestatuses[i].stat.st_ctime)));
				PyDict_SetItemString (dict, "stat", statlist);
			}
			if (filestatuses[i].subpaths)
				PyDict_SetItemString (dict, "subpaths", gfalresults_2_python (filestatuses[i].subpaths, filestatuses[i].nbsubpaths));
			PyList_Append (list, dict);
		}
	}

	return (list);
}
%}

%include "typemaps.i"

// in python, Long are 64bits
%typemap(python, in) GFAL_LONG64 {
	if ($input == Py_None)
		$1 = 0;
	else
		$1 = (GFAL_LONG64) PyLong_AsLongLong ($input);
}

// if the string is empty, it is replaced by NULL
%typemap(python, in) char * {
	if ($input == Py_None) {
		$1 = NULL;
	} else {
		$1 = PyString_AsString ($input);
		if ($1 && $1[0] == 0) $1 = NULL;
	}
}

// convert python list into C string list (char**)
%typemap(python, in) char **protocols {
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
%typemap(python, in) (int LEN, char **LIST) {
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

%typemap(python, in, numinputs=0) (int *OUTPUT)(int tmp) {
	tmp = -1;
	$1 = &tmp;
}

%typemap(python, argout) (int *OUTPUT){
	PyObject *o = PyInt_FromLong((long) (*$1));
	resultobj = my_t_output_helper(resultobj,o);
}//end of typemap

%typemap(python, in, numinputs=0) (int **OUTPUT)(int *tmp_tabint) {
	tmp_tabint = NULL;
	$1 = &tmp_tabint;
}

// convert output C string list into python list
%typemap(python, argout) (int **OUTPUT){
	PyObject *o = Py_None;
	int i;

	if (*$1) {
		o = PyList_New (0);
		for (i = 0; i < result; ++i) {
			PyList_Append (o, PyInt_FromLong ((long) (*$1)[i]));
		}
	}

	$result = my_t_output_helper ($result, o);
}//end of typemap

%typemap(python, in, numinputs=0) (char *errbuf, int errbufsz)(char err[256]) {
	err[0] = 0;
	$1 = err;
	$2 = 256;
}

%typemap(python, argout) (char *errbuf){
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

%typemap(python, in, numinputs=0) (char *GUIDOUT)(char tmp_guid[40]) {
	tmp_guid[0] = 0;
	$1 = tmp_guid;
}

%typemap(python, argout) (char *GUIDOUT){
	PyObject *o = PyString_FromString ($1);
	$result = my_t_output_helper ($result, o);
}//end of typemap

%typemap(python, in, numinputs=0) (char **OUTPUT)(char *tmp_char) {
	tmp_char = NULL;
	$1 = &tmp_char;
}

// convert output C string into python string
%typemap(python, argout) (char **OUTPUT){
	PyObject *o = Py_None;

	if (*$1) {
		o = PyString_FromString (*$1);
	}
	$result = my_t_output_helper ($result, o);
}//end of typemap

%typemap(python, in, numinputs=0) (char ***OUTPUT)(char **tmp_tabchar) {
	tmp_tabchar = NULL;
	$1 = &tmp_tabchar;
}

// convert output C string list into python list
%typemap(python, argout) (char ***OUTPUT){
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

%typemap(python, in, numinputs=0) (struct stat64 *)(struct stat64 statbuf) {
	$1 = &statbuf;
	memset ($1, 0, sizeof (struct stat64));
}

// convert output C 'struct stat64' into a python list exactly (in the same order) as the system os.stat() function
%typemap(python, argout) (struct stat64 *){
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

// convert python dictionnary into C gfal request structure (gfal_request)
%typemap(python, in) gfal_request {
	int i,len;
	PyObject *item;

	if ($input == Py_None || !PyDict_Check ($input)) {
		PyErr_SetString (PyExc_TypeError, "Invalid dictionary");
		errno = EINVAL;
		return (NULL);
	}

	$1 = gfal_request_new ();
	if ($1 == NULL) {
		PyErr_SetString (PyExc_MemoryError, strerror (errno));
		return (NULL);
	}

	if ((item = PyDict_GetItem ($input, PyString_FromString ("generatesurls"))) != NULL)
		$1->generatesurls = (int) PyInt_AsLong (item);

	if ((item = PyDict_GetItem ($input, PyString_FromString ("relative_path"))) != NULL) {
		$1->relative_path = PyString_AsString (item);
		if ($1->relative_path && $1->relative_path[0] == 0)
			$1->relative_path = NULL;
	}

	if ((item = PyDict_GetItem ($input, PyString_FromString ("surls"))) != NULL) {
		if (!PyList_Check (item) || (len = PyList_Size (item)) < 1) {
			PyErr_SetString (PyExc_MemoryError, "Invalid value in 'surls' field");
			errno = EINVAL;
			return (NULL);
		}
		$1->nbfiles = len;
		$1->surls = (char**) calloc (len + 1, sizeof (char*));
		if ($1->surls == NULL) {
			PyErr_SetString (PyExc_MemoryError, "No enough memory");
			errno = ENOMEM;
			return (NULL);
		}
		for (i = 0; i < len; ++i) {
			$1->surls[i] = PyString_AsString (PyList_GetItem (item, i));
		}
		$1->surls[i] = NULL;
	}

	if ((item = PyDict_GetItem ($input, PyString_FromString ("endpoint"))) != NULL) {
		$1->endpoint = PyString_AsString (item);
		if ($1->endpoint && $1->endpoint[0] == 0)
			$1->endpoint = NULL;
	}

	if ((item = PyDict_GetItem ($input, PyString_FromString ("oflag"))) != NULL)
		$1->oflag = (int) PyInt_AsLong (item);

	if ((item = PyDict_GetItem ($input, PyString_FromString ("filesizes"))) != NULL) {
		if (!PyList_Check (item)) {
			PyErr_SetString (PyExc_MemoryError, "Invalid value in 'filesizes' field");
			errno = EINVAL;
			return (NULL);
		} else if ((len = PyList_Size (item)) == 0) {
			$1->filesizes == NULL;
		} else {
			$1->filesizes = (GFAL_LONG64 *) calloc (len, sizeof (GFAL_LONG64));
			if ($1->filesizes == NULL) {
				PyErr_SetString (PyExc_MemoryError, "No enough memory");
				errno = ENOMEM;
				return (NULL);
			}
			for (i = 0; i < len; ++i) {
				$1->filesizes[i] = PyInt_AsLong (PyList_GetItem (item, i));
			}
		}
	}

	if ((item = PyDict_GetItem ($input, PyString_FromString ("defaultsetype"))) != NULL) {
		char *defaultsetype = PyString_AsString (item);
		if (!defaultsetype || defaultsetype[0] == 0 || strcmp (defaultsetype, "none") == 0)
			$1->defaultsetype = TYPE_NONE;
		else if (strcmp (defaultsetype, "se") == 0)
			$1->defaultsetype = TYPE_SE;
		else if (strcmp (defaultsetype, "srmv1") == 0)
			$1->defaultsetype = TYPE_SRM;
		else if (strcmp (defaultsetype, "srmv2") == 0)
			$1->defaultsetype = TYPE_SRMv2;
		else {
			PyErr_SetString (PyExc_MemoryError, "Invalid value in 'defaultsetype' field");
			errno = EINVAL;
			return (NULL);
		}
	}

	if ((item = PyDict_GetItem ($input, PyString_FromString ("setype"))) != NULL) {
		char *setype = PyString_AsString (item);
		if (!setype || setype[0] == 0 || strcmp (setype, "none") == 0)
			$1->setype = TYPE_NONE;
		else if (strcmp (setype, "se") == 0)
			$1->setype = TYPE_SE;
		else if (strcmp (setype, "srmv1") == 0)
			$1->setype = TYPE_SRM;
		else if (strcmp (setype, "srmv2") == 0)
			$1->setype = TYPE_SRMv2;
		else {
			PyErr_SetString (PyExc_MemoryError, "Invalid value in 'setype' field");
			errno = EINVAL;
			return (NULL);
		}
	}

	if ((item = PyDict_GetItem ($input, PyString_FromString ("no_bdii_check"))) != NULL)
		$1->no_bdii_check = (int) PyInt_AsLong (item);

	if ((item = PyDict_GetItem ($input, PyString_FromString ("timeout"))) != NULL)
		$1->timeout = (int) PyInt_AsLong (item);

	if ((item = PyDict_GetItem ($input, PyString_FromString ("protocols"))) != NULL) {
		if (!PyList_Check (item) || (len = PyList_Size (item)) < 1) {
			PyErr_SetString (PyExc_MemoryError, "Invalid value in 'protocols' field");
			errno = EINVAL;
			return (NULL);
		}
		$1->protocols = (char**) calloc (len + 2, sizeof (char*));
		if ($1->protocols == NULL) {
			PyErr_SetString (PyExc_MemoryError, "No enough memory");
			errno = ENOMEM;
			return (NULL);
		}
		for (i = 0; i < len; ++i) {
			$1->protocols[i] = PyString_AsString (PyList_GetItem (item, i));
		}
		$1->protocols[i] = "";
		$1->protocols[i + 1] = NULL;
	}

	if ((item = PyDict_GetItem ($input, PyString_FromString ("srmv2_spacetokendesc"))) != NULL) {
		$1->srmv2_spacetokendesc = PyString_AsString (item);
		if ($1->srmv2_spacetokendesc && $1->srmv2_spacetokendesc[0] == 0)
			$1->srmv2_spacetokendesc = NULL;
	}

	if ((item = PyDict_GetItem ($input, PyString_FromString ("srmv2_desiredpintime"))) != NULL)
		$1->srmv2_desiredpintime = (int) PyInt_AsLong (item);

	if ((item = PyDict_GetItem ($input, PyString_FromString ("srmv2_lslevels"))) != NULL)
		$1->srmv2_lslevels = (int) PyInt_AsLong (item);

	if ((item = PyDict_GetItem ($input, PyString_FromString ("srmv2_lsoffset"))) != NULL)
		$1->srmv2_lsoffset = (int) PyInt_AsLong (item);

	if ((item = PyDict_GetItem ($input, PyString_FromString ("srmv2_lscount"))) != NULL)
		$1->srmv2_lscount = (int) PyInt_AsLong (item);
}

%typemap(python, in, numinputs=0) (gfal_internal *)(gfal_internal gfal) {
	$1 = &gfal;
}

// convert C gfal_internal into Python object
%typemap(python, argout) gfal_internal * {
	PyObject *o;

	if ($1 == NULL || *$1 == NULL)
		o = Py_None;
	else
		o = SWIG_NewPointerObj(*$1, $descriptor(gfal_internal), 1);

	$result = my_t_output_helper ($result, o);
}

// convert python object into C gfal_internal
%typemap(python, in) gfal_internal {
	if ((SWIG_ConvertPtr($input,(void **) &$1, $1_descriptor,SWIG_POINTER_EXCEPTION)) < 0)
		return (NULL);
}

// convert C gfal_internal into Python object
%typemap(python, argout) gfal_internal {
	PyObject *o;

	if ($1 == NULL)
		o = Py_None;
	else
		o = SWIG_NewPointerObj($1, $descriptor(gfal_internal), 1);

	$result = my_t_output_helper ($result, o);
}

%typemap(python, in, numinputs=0) (gfal_filestatus **)(gfal_filestatus *filestatus_tmp) {
	filestatus_tmp = NULL;
	$1 = &filestatus_tmp;
}

// convert output C 'gfal_filestatus' list into python dictionnary
%typemap(python, argout) (gfal_filestatus **){
	PyObject *list = $1 != NULL ? gfalresults_2_python (*$1, result) : Py_None;
	$result = my_t_output_helper ($result, list);
}//end of typemap
