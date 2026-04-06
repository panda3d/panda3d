/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramPyObject.I
 * @author rdb
 * @date 2021-03-01
 */

#include "paramPyObject.h"

/**
 * Increments the reference count.  Assumes the GIL is held.
 */
INLINE ParamPyObject::
ParamPyObject(PyObject *value) : _value(Py_NewRef(value)) {
}

/**
 * Returns a new reference to the stored value.
 */
INLINE PyObject *ParamPyObject::
get_value() const {
  return Py_NewRef(_value);
}
