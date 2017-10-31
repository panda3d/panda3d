/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTask_ext.h
 * @author rdb
 * @date 2017-10-29
 */

#include "asyncTask_ext.h"
#include "nodePath.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_AsyncTask;
#endif

/**
 * Yields continuously until the task has finished.
 */
static PyObject *gen_next(PyObject *self) {
  const AsyncTask *request = nullptr;
  if (!Dtool_Call_ExtractThisPointer(self, Dtool_AsyncTask, (void **)&request)) {
    return nullptr;
  }

  if (request->is_alive()) {
    // Continue awaiting the result.
    Py_INCREF(self);
    return self;
  } else {
    // It's done.  Do we have a method like result(), eg. in the case of a
    // ModelLoadRequest?  In that case we pass that value into the exception.
    PyObject *method = PyObject_GetAttrString(self, "result");
    PyObject *result = nullptr;
    if (method != nullptr) {
      if (PyCallable_Check(method)) {
        result = _PyObject_CallNoArg(method);
        Py_DECREF(method);
        if (result == nullptr) {
          // An exception happened.  Pass it on.
          return nullptr;
        }
      }
      Py_DECREF(method);
    }
    Py_INCREF(PyExc_StopIteration);
    PyErr_Restore(PyExc_StopIteration, result, nullptr);
    return nullptr;
  }
}

/**
 * Returns a generator that continuously yields an awaitable until the task
 * has finished.  This allows syntax like `model = await loader.load...` to be
 * used in a Python coroutine.
 */
PyObject *Extension<AsyncTask>::
__await__(PyObject *self) {
  Dtool_GeneratorWrapper *gen;
  gen = (Dtool_GeneratorWrapper *)PyType_GenericAlloc(&Dtool_GeneratorWrapper_Type, 0);
  if (gen != nullptr) {
    Py_INCREF(self);
    gen->_base._self = self;
    gen->_iternext_func = &gen_next;
  }
  return (PyObject *)gen;
}

#endif
