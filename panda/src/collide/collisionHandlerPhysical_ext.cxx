/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerPhysical_ext.cxx
 * @author rdb
 * @date 2020-12-31
 */

#include "collisionHandlerPhysical_ext.h"
#include "collisionHandlerEvent_ext.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickling behavior.
 */
PyObject *Extension<CollisionHandlerPhysical>::
__reduce__(PyObject *self) const {
  extern struct Dtool_PyTypedObject Dtool_NodePath;

  // Create a tuple with all the NodePath pointers.
  PyObject *nodepaths = PyTuple_New(_this->_colliders.size() * 2 + 1);
  Py_ssize_t i = 0;

  if (_this->has_center()) {
    const NodePath *center = &(_this->get_center());
    PyTuple_SET_ITEM(nodepaths, i++,
      DTool_CreatePyInstance((void *)center, Dtool_NodePath, false, true));
  } else {
    PyTuple_SET_ITEM(nodepaths, i++, Py_NewRef(Py_None));
  }

  CollisionHandlerPhysical::Colliders::const_iterator it;
  for (it = _this->_colliders.begin(); it != _this->_colliders.end(); ++it) {
    const NodePath *collider = &(it->first);
    const NodePath *target = &(it->second._target);
    PyTuple_SET_ITEM(nodepaths, i++,
      DTool_CreatePyInstance((void *)collider, Dtool_NodePath, false, true));
    PyTuple_SET_ITEM(nodepaths, i++,
      DTool_CreatePyInstance((void *)target, Dtool_NodePath, false, true));
  }

  Datagram dg;
  _this->write_datagram(dg);

  const char *data = (const char *)dg.get_data();
  Py_ssize_t size = dg.get_length();
  return Py_BuildValue("O()(y#N)", Py_TYPE(self), data, size, nodepaths);
}

/**
 * Takes the value returned by __getstate__ and uses it to freshly initialize
 * this CollisionHandlerPhysical object.
 */
void Extension<CollisionHandlerPhysical>::
__setstate__(PyObject *self, vector_uchar data, PyObject *nodepaths) {
  {
    Datagram dg(std::move(data));
    DatagramIterator scan(dg);
    _this->read_datagram(scan);
  }

  PyObject *center = PyTuple_GET_ITEM(nodepaths, 0);
  if (center != Py_None) {
    _this->set_center(*(NodePath *)DtoolInstance_VOID_PTR(center));
  } else {
    _this->clear_center();
  }

  size_t num_nodepaths = Py_SIZE(nodepaths);
  for (size_t i = 1; i < num_nodepaths;) {
    NodePath *collider = (NodePath *)DtoolInstance_VOID_PTR(PyTuple_GET_ITEM(nodepaths, i++));
    NodePath *target = (NodePath *)DtoolInstance_VOID_PTR(PyTuple_GET_ITEM(nodepaths, i++));
    _this->add_collider(*collider, *target);
  }
}

#endif
