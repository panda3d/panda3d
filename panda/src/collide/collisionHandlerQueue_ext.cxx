/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerQueue_ext.cxx
 * @author rdb
 * @date 2020-12-31
 */

#include "collisionHandlerQueue_ext.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickling behavior.
 */
PyObject *Extension<CollisionHandlerQueue>::
__reduce__(PyObject *self) const {
  // CollisionHandlerQueue has no interesting properties.
  return Py_BuildValue("(O())", Py_TYPE(self));
}

#endif
