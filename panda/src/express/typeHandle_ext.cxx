// Filename: typeHandle_ext.cxx
// Created by:  rdb (17Sep14)
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

#include "typeHandle_ext.h"

#ifdef HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::make
//       Access: Published, Static
//  Description: Constructs a TypeHandle from a Python class object.
//               Useful for automatic coercion, to allow a class
//               object to be passed wherever a TypeHandle is
//               expected.
////////////////////////////////////////////////////////////////////
TypeHandle Extension<TypeHandle>::
make(PyTypeObject *tp) {
  if (!PyType_IsSubtype(tp, &Dtool_DTOOL_SUPER_BASE._PyType)) {
    PyErr_SetString(PyExc_TypeError, "a Panda type is required");
    return TypeHandle::none();
  }

  Dtool_PyTypedObject *dtool_tp = (Dtool_PyTypedObject *) tp;
  return dtool_tp->_type;
}

#endif
