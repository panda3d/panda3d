/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file android_log.c
 * @author rdb
 * @date 2021-12-10
 */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#define PY_SSIZE_T_CLEAN 1

#include "Python.h"
#include <android/log.h>

/**
 * Writes a message to the Android log.
 */
static PyObject *
_py_write(PyObject *self, PyObject *args) {
  int prio;
  char *tag;
  char *text;
  if (PyArg_ParseTuple(args, "iss", &prio, &tag, &text)) {
    __android_log_write(prio, tag, text);
    Py_RETURN_NONE;
  }
  return NULL;
}

static PyMethodDef python_simple_funcs[] = {
  { "write", &_py_write, METH_VARARGS },
  { NULL, NULL }
};

static struct PyModuleDef android_log_module = {
  PyModuleDef_HEAD_INIT,
  "android_log",
  NULL,
  -1,
  python_simple_funcs,
  NULL, NULL, NULL, NULL
};

__attribute__((visibility("default")))
PyObject *PyInit_android_log() {
  return PyModule_Create(&android_log_module);
}
