/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCoordinateSystem_ext.cxx
 * @author rdb
 * @date 2021-01-01
 */

#include "eggCoordinateSystem_ext.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickle support.
 */
PyObject *Extension<EggCoordinateSystem>::
__reduce__() const {
  extern struct Dtool_PyTypedObject Dtool_EggCoordinateSystem;

  // We can't use the regular EggNode handling for EggCoordinateSystem, because
  // the <CoordinateSystem> node is removed from the EggData after reading.
  // Oh well, this is more efficient anyway.
  int value = _this->get_value();
  return Py_BuildValue("O(i)", (PyObject *)&Dtool_EggCoordinateSystem, value);  
}

#endif
