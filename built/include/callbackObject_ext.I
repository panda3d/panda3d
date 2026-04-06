/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file callbackObject_ext.I
 * @author rdb
 * @date 2015-02-25
 */

/**
 * This static constructor is merely provided so that interrogate can
 * automatically coerce Python functions when passing them to a C++ function
 * that accepts a CallbackObject.
 */
INLINE PT(CallbackObject) Extension<CallbackObject>::
make(PyObject *function) {
  if (function != Py_None && !PyCallable_Check(function)) {
    PyErr_SetString(PyExc_TypeError, "expected callable or None");
    return nullptr;
  } else {
    return new PythonCallbackObject(function);
  }
}
