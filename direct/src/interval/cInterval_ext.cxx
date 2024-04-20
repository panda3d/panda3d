/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cInterval_ext.cxx
 * @author rdb
 * @date 2020-10-17
 */

#include "cInterval_ext.h"
#include "cIntervalManager.h"
#include "asyncFuture.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_CInterval;
#endif

/**
 * Yields continuously until the interval is done.
 */
static PyObject *gen_next_c_interval(PyObject *self) {
  const CInterval *ival;
  if (!Dtool_Call_ExtractThisPointer(self, Dtool_CInterval, (void **)&ival)) {
    return nullptr;
  }

  if (ival->get_state() != CInterval::S_final) {
    // Try again next frame.
    return Py_NewRef(Py_None);
  }
  else {
    PyErr_SetNone(PyExc_StopIteration);
    return nullptr;
  }
}

/**
 * Awaiting an interval starts it and yields a future until it is done.
 */
PyObject *Extension<CInterval>::
__await__(PyObject *self) {
  if (_this->get_state() != CInterval::S_initial) {
    PyErr_SetString(PyExc_RuntimeError, "Can only await an interval that is in the initial state.");
    return nullptr;
  }

  // This may be overridden from Python (such as is the case for Sequence), so
  // we call this via Python.
  PyObject *result = PyObject_CallMethod(self, "start", nullptr);
  Py_XDECREF(result);
  return Dtool_NewGenerator(self, &gen_next_c_interval);
}

#endif  // HAVE_PYTHON
