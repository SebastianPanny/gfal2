/*********************************************
    SWIG input file for LFC
 (including typemaps for non-trivial functions
*********************************************/

%module gfalthr

%exception{
      Py_BEGIN_ALLOW_THREADS
      $action
      Py_END_ALLOW_THREADS
}

%include "gfal.i"
int init();

%{
#include "Python.h"

int init(){
    return Cthread_init();
}
%}
