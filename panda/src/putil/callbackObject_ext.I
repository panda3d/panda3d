// Filename: callbackObject_ext.I
// Created by:  rdb (25Feb15)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: Extension<CallbackObject>::make
//       Access: Published
//  Description: This static constructor is merely provided so that
//               interrogate can automatically coerce Python
//               functions when passing them to a C++ function that
//               accepts a CallbackObject.
////////////////////////////////////////////////////////////////////
INLINE PT(CallbackObject) Extension<CallbackObject>::
make(PyObject *function) {
  if (function != Py_None && !PyCallable_Check(function)) {
    PyErr_SetString(PyExc_TypeError, "expected callable or None");
    return NULL;
  } else {
    return new PythonCallbackObject(function);
  }
}
