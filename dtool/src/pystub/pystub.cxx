// Filename: pystub.cxx
// Created by:  drose (09Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pystub.h"

extern "C" {
  EXPCL_DTOOLCONFIG int PyArg_ParseTuple(...);
  EXPCL_DTOOLCONFIG int Py_BuildValue(...);
  EXPCL_DTOOLCONFIG int PyInt_FromLong(...);
  EXPCL_DTOOLCONFIG int PyFloat_FromDouble(...);
  EXPCL_DTOOLCONFIG int PyString_FromString(...);
  EXPCL_DTOOLCONFIG int PyString_FromStringAndSize(...);
  EXPCL_DTOOLCONFIG int Py_InitModule4(...);
  EXPCL_DTOOLCONFIG int PyObject_IsTrue(...);
  EXPCL_DTOOLCONFIG int PyErr_SetString(...);
  EXPCL_DTOOLCONFIG extern void *PyExc_AssertionError;
};

int
PyArg_ParseTuple(...) {
  return 0;
}

int
Py_BuildValue(...) {
  return 0;
}

int
PyInt_FromLong(...) {
  return 0;
}

int
PyFloat_FromDouble(...) {
  return 0;
}

int
PyString_FromStringAndSize(...) {
  return 0;
}

int
PyString_FromString(...) {
  return 0;
}

int
Py_InitModule4(...) {
  return 0;
}

int
PyObject_IsTrue(...) {
  return 0;
}

int
PyErr_SetString(...) {
  return 0;
}

void *PyExc_AssertionError = (void *)NULL;


void
pystub() {
}
