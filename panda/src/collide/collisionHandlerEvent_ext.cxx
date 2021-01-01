/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerEvent_ext.cxx
 * @author rdb
 * @date 2020-12-31
 */

#include "collisionHandlerEvent_ext.h"
#include "collisionHandlerFloor.h"
#include "collisionHandlerGravity.h"
#include "collisionHandlerPusher.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickling behavior.
 */
PyObject *Extension<CollisionHandlerEvent>::
__reduce__(PyObject *self) const {
  extern struct Dtool_PyTypedObject Dtool_Datagram;

  // Call the write_datagram method via Python, since it's not a virtual method
  // on the C++ end.
#if PY_MAJOR_VERSION >= 3
  PyObject *method_name = PyUnicode_FromString("write_datagram");
#else
  PyObject *method_name = PyString_FromString("write_datagram");
#endif

  Datagram dg;
  PyObject *destination = DTool_CreatePyInstance(&dg, Dtool_Datagram, false, false);

  PyObject *retval = PyObject_CallMethodOneArg(self, method_name, destination);
  Py_DECREF(method_name);
  Py_DECREF(destination);
  if (retval == nullptr) {
    return nullptr;
  }
  Py_DECREF(retval);

  const char *data = (const char *)dg.get_data();
  Py_ssize_t size = dg.get_length();
#if PY_MAJOR_VERSION >= 3
  return Py_BuildValue("O()y#", Py_TYPE(self), data, size);
#else
  return Py_BuildValue("O()s#", Py_TYPE(self), data, size);
#endif
}

/**
 * Takes the value returned by __getstate__ and uses it to freshly initialize
 * this CollisionHandlerEvent object.
 */
void Extension<CollisionHandlerEvent>::
__setstate__(PyObject *self, vector_uchar data) {
  extern struct Dtool_PyTypedObject Dtool_DatagramIterator;

  // Call the read_datagram method via Python, since it's not a virtual method
  // on the C++ end.
#if PY_MAJOR_VERSION >= 3
  PyObject *method_name = PyUnicode_FromString("read_datagram");
#else
  PyObject *method_name = PyString_FromString("read_datagram");
#endif

  Datagram dg(std::move(data));
  DatagramIterator scan(dg);
  PyObject *source = DTool_CreatePyInstance(&scan, Dtool_DatagramIterator, false, false);

  PyObject *retval = PyObject_CallMethodOneArg(self, method_name, source);
  Py_DECREF(method_name);
  Py_DECREF(source);
  Py_XDECREF(retval);
}

#endif
