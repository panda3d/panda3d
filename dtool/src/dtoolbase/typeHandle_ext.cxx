/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeHandle_ext.cxx
 * @author rdb
 * @date 2014-09-17
 */

#include "typeHandle_ext.h"

#ifdef HAVE_PYTHON

/**
 * Constructs a TypeHandle from a Python class object.  Useful for automatic
 * coercion, to allow a class object to be passed wherever a TypeHandle is
 * expected.
 */
TypeHandle Extension<TypeHandle>::
make(PyTypeObject *tp) {
  Dtool_PyTypedObject *super_base = Dtool_GetSuperBase();
  if (!PyType_IsSubtype(tp, (PyTypeObject *)super_base)) {
    PyErr_SetString(PyExc_TypeError, "a Panda type is required");
    return TypeHandle::none();
  }

  Dtool_PyTypedObject *dtool_tp = (Dtool_PyTypedObject *) tp;
  return dtool_tp->_type;
}

#endif
