// Filename: pystub.C
// Created by:  drose (09Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "pystub.h"

extern "C" {
  EXPCL_DTOOLCONFIG int PyArg_ParseTuple(...);
  EXPCL_DTOOLCONFIG int Py_BuildValue(...);
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
