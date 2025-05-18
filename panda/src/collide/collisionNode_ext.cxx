/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionNode_ext.cxx
 * @author rdb
 * @date 2024-12-12
 */

#include "collisionNode_ext.h"

#ifdef HAVE_PYTHON

#include "collisionNode.h"

/**
 * Returns the object previously set via set_owner().  If the object has been
 * destroyed, returns None.
 */
PyObject *Extension<CollisionNode>::
get_owner() const {
  PyObject *owner = (PyObject *)_this->get_owner();

#if PY_VERSION_HEX >= 0x030D0000 // 3.13
  PyObject *strong_ref;
  int result = 0;
  if (owner != nullptr) {
    result = PyWeakref_GetRef(owner, &strong_ref);
  }
  if (result > 0) {
    return strong_ref;
  }
  else if (result == 0) {
    return Py_NewRef(Py_None);
  }
  else {
    return nullptr;
  }
#else
  return Py_NewRef(owner != nullptr ? PyWeakref_GetObject(owner) : Py_None);
#endif
}

/**
 * Stores a weak reference to the given object on the CollisionNode, for later
 * use in collision events and handlers.
 */
void Extension<CollisionNode>::
set_owner(PyObject *owner) {
  if (owner != Py_None) {
    PyObject *ref = PyWeakref_NewRef(owner, nullptr);
    _this->set_owner(ref, [](void *obj) { Py_DECREF((PyObject *)obj); });
  } else {
    _this->clear_owner();
  }
}

#endif
