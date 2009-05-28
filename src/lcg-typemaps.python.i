
#ifdef SWIGPYTHON

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
            if (filestatuses[i].checksumtype && filestatuses[i].checksum) {
                PyDict_SetItemString (dict, "checksumtype", PyString_FromString (filestatuses[i].checksumtype));
                PyDict_SetItemString (dict, "checksum", PyString_FromString (filestatuses[i].checksum));
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

%typemap(out) void %{ $result = Py_None; %}

// in python, Long are 64bits
%typemap(in) GFAL_LONG64 {
    if ($input == Py_None)
        $1 = 0;
    else
        $1 = (GFAL_LONG64) PyLong_AsLongLong ($input);
}

// 
%typemap(in) enum se_type {
    if ($input == Py_None)
        $1 = TYPE_NONE;
    else if (PyInt_Check ($input))
        $1 = (enum se_type) PyInt_AsLong ($input);
    else if (PyString_Check ($input)) {
        char *setype = PyString_AsString ($input);
        if (!setype || setype[0] == 0 || strcmp (setype, "none") == 0)
            $1 = TYPE_NONE;
        else if (strcmp (setype, "se") == 0)
            $1 = TYPE_SE;
        else if (strcmp (setype, "srmv1") == 0)
            $1 = TYPE_SRM;
        else if (strcmp (setype, "srmv2") == 0)
            $1 = TYPE_SRMv2;
        else {
            PyErr_SetString (PyExc_MemoryError, "Invalid setype value, must be: none, se, srmv1, srmv2");
            errno = EINVAL;
            return (NULL);
        }
    }
}

%typemap(in, numinputs=0) (enum se_type *OUTPUT)(enum se_type tmp) {
    tmp = TYPE_NONE;
    $1 = &tmp;
}

%typemap(argout) (enum se_type *OUTPUT){
    char setype[6];

    switch (*$1) {
        case TYPE_SRM:
            snprintf (setype, 6, "srmv1");
            break;
        case TYPE_SRMv2:
            snprintf (setype, 6, "srmv2");
            break;
        case TYPE_SE:
            snprintf (setype, 6, "se");
            break;
    }
    $result = my_t_output_helper($result, PyString_FromString (setype));
}//end of typemap

// if the string is empty, it is replaced by NULL
%typemap(in) char * {
    if ($input == Py_None) {
        $1 = NULL;
    } else {
        $1 = PyString_AsString ($input);
        if ($1 && $1[0] == 0) $1 = NULL;
    }
}

// No need to free pointers which are returned by python API functions
%typemap(freearg) char * {}

// convert python list into C integer list (int*)
%typemap(in) (int LEN, const int *LIST) {
    int i;

    if ($input != Py_None && !PyList_Check ($input)) {
        PyErr_SetString (PyExc_TypeError, "Should be a list");
        return (NULL);
    }
    if ($input == Py_None || ($1 = PyList_Size ($input)) < 1) {
        $1 = 0;
        $2 = NULL;
    } else {
        $2 = (int*) calloc ($1, sizeof (int));
        if ($2 == NULL) {
            errno = ENOMEM;
            return NULL;
        }

        for (i = 0; i < $1; ++i) {
            $2[i] = PyInt_AsLong (PyList_GetItem ($input, i));
        }
    }
}

// Free the int* used as input for the C function, if that is not used to build the return value
%typemap(freearg) (int LEN, const int *LIST) {
    if ($2) free ($2);
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

// Free the char** used as input for the C function, if that is not used to build the return value
%typemap(freearg) (char **protocols) {
    if ($1) free ($1);
}

// Feed list length and C-style list from python list
%typemap(in) (int LEN, char **LIST) {
    int i;

    if ($input != Py_None && !PyList_Check ($input)) {
        PyErr_SetString (PyExc_TypeError, "Should be a list");
        return (NULL);
    }
    if ($input == Py_None || ($1 = PyList_Size ($input)) < 1) {
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

// Free the char** used as input for the C function, if that is not used to build the return value
%typemap(freearg) (int LEN, char **LIST) {
    if ($2) free ($2);
}

%typemap(in, numinputs=0) (int *OUTPUT)(int tmp) {
    tmp = -1;
    $1 = &tmp;
}

%typemap(argout) (int *OUTPUT){
    PyObject *o = PyInt_FromLong((long) (*$1));
    $result = my_t_output_helper($result,o);
}//end of typemap

%typemap(in, numinputs=0) (int **OUTPUT)(int *tmp_tabint) {
    tmp_tabint = NULL;
    $1 = &tmp_tabint;
}

// convert output C string list into python list
%typemap(argout) (int **OUTPUT){
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

// convert output C 'struct dirent' into a string
// Use of other fields than d_name is not portable
#if defined(_LARGEFILE64_SOURCE)
%typemap(out) (struct dirent64 *){
    if ($1) {
        $result = PyString_FromString ($1->d_name);
    } else {
        $result = Py_None;
    }
}//end of typemap
#endif

%typemap(in, numinputs=0) (struct stat64 *)(struct stat64 statbuf) {
    $1 = &statbuf;
    memset ($1, 0, sizeof (struct stat64));
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

%typemap(in, numinputs=0) (struct stat *)(struct stat statbuf) {
    $1 = &statbuf;
    memset ($1, 0, sizeof (struct stat));
}

// convert output C 'struct stat' into a python list exactly (in the same order) as the system os.stat() function
%typemap(argout) (struct stat *){
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

%typemap(in, numinputs=0) (struct stat64 *)(struct stat64 statbuf) {
    $1 = &statbuf;
    memset ($1, 0, sizeof (struct stat64));
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

// convert python dictionnary into C gfal request structure (gfal_request)
%typemap(in) gfal_request {
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
    else if ((item = PyDict_GetItem ($input, PyString_FromString ("nbfiles"))) != NULL)
        $1->nbfiles = (int) PyInt_AsLong (item);

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

%typemap(in, numinputs=0) (gfal_internal *)(gfal_internal gfal) {
    $1 = &gfal;
}

// convert C gfal_internal into Python object
%typemap(argout) gfal_internal * {
    /* Only return the object if something (a return code) has already been returned
     * => not return the object for gfal_internal_free function */
    if ($result) {
        PyObject *o;

        if ($1 == NULL || *$1 == NULL)
            o = Py_None;
        else
            o = SWIG_NewPointerObj(*$1, $descriptor(gfal_internal), 1);

        $result = my_t_output_helper ($result, o);
    }
}

// convert python object into C gfal_internal
%typemap(in) gfal_internal {
    if ((SWIG_ConvertPtr($input,(void **) &$1, $1_descriptor,SWIG_POINTER_EXCEPTION)) < 0)
        return (NULL);
}

// convert C gfal_internal into Python object
%typemap(argout) gfal_internal {
    PyObject *o;

    if ($1 == NULL)
        o = Py_None;
    else
        o = SWIG_NewPointerObj($1, $descriptor(gfal_internal), 1);

    $result = my_t_output_helper ($result, o);
}

%typemap(in, numinputs=0) (lcg_filestatus **OUTPUT)(lcg_filestatus *filestatus_tmp) {
    filestatus_tmp = NULL;
    $1 = &filestatus_tmp;
}

// convert output C 'lcg_filestatus' list into python dictionnary
%typemap(argout) (lcg_filestatus **OUTPUT){
    PyObject *list = Py_None, *dict = Py_None;
    int i;

    if (*$1) {
        list = PyList_New (0);
        for (i = 0; i < arg1; ++i) {
            dict = PyDict_New ();
            PyDict_SetItemString (dict, "file", (*$1)[i].file ? PyString_FromString ((*$1)[i].file) : Py_None);
            PyDict_SetItemString (dict, "status", PyInt_FromLong ((long)((*$1)[i].status)));
            PyDict_SetItemString (dict, "explanation", (*$1)[i].explanation ? PyString_FromString ((*$1)[i].explanation) : Py_None);
            PyList_Append (list, dict);
        }
    }

    $result = my_t_output_helper ($result, list);
}//end of typemap

%typemap(in, numinputs=0) (lcg_checksum **OUTPUT)(lcg_checksum *filestatus_tmp) {
    filestatus_tmp = NULL;
    $1 = &filestatus_tmp;
}

// convert output C 'lcg_checksum' list into python dictionnary
%typemap(argout) (lcg_checksum **OUTPUT){
    PyObject *list = Py_None, *dict = Py_None;
    int i;

    if (*$1) {
        list = PyList_New (0);
        for (i = 0; i < arg1; ++i) {
            dict = PyDict_New ();
            PyDict_SetItemString (dict, "file", (*$1)[i].file ? PyString_FromString ((*$1)[i].file) : Py_None);
            PyDict_SetItemString (dict, "status", PyInt_FromLong ((long)((*$1)[i].status)));
            PyDict_SetItemString (dict, "explanation", (*$1)[i].explanation ? PyString_FromString ((*$1)[i].explanation) : Py_None);
            PyDict_SetItemString (dict, "checksum", (*$1)[i].checksum ? PyString_FromString ((*$1)[i].checksum) : Py_None);
            PyList_Append (list, dict);
        }
    }

    $result = my_t_output_helper ($result, list);
}//end of typemap

%typemap(in, numinputs=0) (gfal_filestatus **)(gfal_filestatus *filestatus_tmp) {
    filestatus_tmp = NULL;
    $1 = &filestatus_tmp;
}

// convert output C 'gfal_filestatus' list into python dictionnary
%typemap(argout) (gfal_filestatus **){
    PyObject *list = $1 != NULL ? gfalresults_2_python (*$1, result) : Py_None;
    $result = my_t_output_helper ($result, list);
}//end of typemap

%typemap(in, numinputs=0) (int *LEN, gfal_spacemd **LISTOUT)(int len_tmp, gfal_spacemd *spacemd_tmp) {
    len_tmp = 0;
    spacemd_tmp = NULL;
    $1 = &len_tmp;
    $2 = &spacemd_tmp;
}

// convert output C 'gfal_spacemd' list into python dictionnary
%typemap(argout) (int *LEN, gfal_spacemd **LISTOUT){
    PyObject *list = Py_None, *dict = Py_None;
    int i;

    if (*$2) {
        list = PyList_New (0);
        for (i = 0; i < *$1; ++i) {
            dict = PyDict_New ();
            PyDict_SetItemString (dict, "spacetoken", (*$2)[i].spacetoken ? PyString_FromString ((*$2)[i].spacetoken) : Py_None);
            PyDict_SetItemString (dict, "owner", (*$2)[i].owner ? PyString_FromString ((*$2)[i].owner) : Py_None);
            PyDict_SetItemString (dict, "totalsize", PyLong_FromLongLong ((long long) ((*$2)[i].totalsize)));
            PyDict_SetItemString (dict, "guaranteedsize", PyLong_FromLongLong ((long long) ((*$2)[i].guaranteedsize)));
            PyDict_SetItemString (dict, "unusedsize", PyLong_FromLongLong ((long long) ((*$2)[i].unusedsize)));
            PyDict_SetItemString (dict, "lifetimeassigned", PyInt_FromLong ((long)((*$2)[i].lifetimeassigned)));
            PyDict_SetItemString (dict, "lifetimeleft", PyInt_FromLong ((long)((*$2)[i].lifetimeleft)));

            switch ((*$2)[i].retentionpolicy) {
                case GFAL_POLICY_REPLICA:
                    PyDict_SetItemString (dict, "retentionpolicy", PyString_FromString ("replica"));
                    break;
                case GFAL_POLICY_OUTPUT:
                    PyDict_SetItemString (dict, "retentionpolicy", PyString_FromString ("output"));
                    break;
                case GFAL_POLICY_CUSTODIAL:
                    PyDict_SetItemString (dict, "retentionpolicy", PyString_FromString ("custodial"));
                    break;
                default:
                    PyDict_SetItemString (dict, "retentionpolicy", PyString_FromString ("unknown"));
            }

            switch ((*$2)[i].accesslatency) {
                case GFAL_LATENCY_ONLINE:
                    PyDict_SetItemString (dict, "accesslatency", PyString_FromString ("online"));
                    break;
                case GFAL_LATENCY_NEARLINE:
                    PyDict_SetItemString (dict, "accesslatency", PyString_FromString ("nearline"));
                    break;
                default:
                    PyDict_SetItemString (dict, "accesslatency", PyString_FromString ("unknown"));
            }

            PyList_Append (list, dict);
        }
    }

    $result = my_t_output_helper ($result, list);
}//end of typemap


%typemap(in) (void *BUFOUT, size_t BUFOUTSZ)(char *buf) {
    if (!PyInt_Check ($input)) {
        PyErr_SetString (PyExc_TypeError, "Should be an integer");
        return (NULL);
    }
    $2 = (int) PyInt_AsLong ($input);
    $1 = (void *) calloc ($2, sizeof (char));
    if ($1 == NULL) {
        PyErr_NoMemory ();
        return (NULL);
    }
}

%typemap(argout) (void *BUFOUT, size_t BUFOUTSZ){
    if (result >= 0) {
        $result = my_t_output_helper ($result, PyBuffer_FromMemory ($1, $2));
    } else {
        Py_INCREF (Py_None);
        $result = my_t_output_helper ($result, Py_None);
    }
}//end of typemap


%typemap(in) (const void *BUFIN, size_t BUFINSZ) {
    if (!PyString_Check ($input)) {
        PyErr_SetString (PyExc_TypeError, "Should be a string");
        return (NULL);
    }
    $1 = (void *) PyString_AsString ($input);
    $2 = PyString_Size ($input);
}

#endif
