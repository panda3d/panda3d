/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file android_support.cxx
 * @author rdb
 * @date 2021-12-10
 */

#include <android/log.h>
#include "android_native_app_glue.h"
#include "config_android.h"

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#define PY_SSIZE_T_CLEAN 1

#include "Python.h"

/**
 * Writes a message to the Android log.
 */
static PyObject *
_py_log_write(PyObject *self, PyObject *args) {
  int prio;
  char *tag;
  char *text;
  if (PyArg_ParseTuple(args, "iss", &prio, &tag, &text)) {
    __android_log_write(prio, tag, text);
    Py_RETURN_NONE;
  }
  return NULL;
}

/**
 * Returns the path to a library, if it can be found.
 */
static PyObject *
_py_find_library(PyObject *self, PyObject *args) {
  char *lib;
  if (PyArg_ParseTuple(args, "s", &lib)) {
    Filename result = android_find_library(panda_android_app->activity, lib);
    if (!result.empty()) {
      return PyUnicode_FromStringAndSize(result.c_str(), (Py_ssize_t)result.length());
    } else {
      Py_RETURN_NONE;
    }
  }
  return NULL;
}

static PyMethodDef python_simple_funcs[] = {
  { "log_write", &_py_log_write, METH_VARARGS },
  { "find_library", &_py_find_library, METH_VARARGS },
  { NULL, NULL }
};

static struct PyModuleDef android_support_module = {
  PyModuleDef_HEAD_INIT,
  "android_support",
  NULL,
  -1,
  python_simple_funcs,
  NULL, NULL, NULL, NULL
};

__attribute__((visibility("default")))
extern "C" PyObject *PyInit_android_support() {
  return PyModule_Create(&android_support_module);
}
