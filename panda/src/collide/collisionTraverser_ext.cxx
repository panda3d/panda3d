/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionTraverser_ext.cxx
 * @author rdb
 * @date 2020-12-31
 */

#include "collisionTraverser_ext.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickling behavior.
 */
PyObject *Extension<CollisionTraverser>::
__getstate__() const {
  extern struct Dtool_PyTypedObject Dtool_CollisionHandler;
  extern struct Dtool_PyTypedObject Dtool_NodePath;

  const std::string &name = _this->get_name();
  size_t num_colliders = _this->get_num_colliders();

  PyObject *state = PyTuple_New(num_colliders * 2 + 3);
  PyTuple_SET_ITEM(state, 0, PyUnicode_FromStringAndSize(name.data(), name.size()));
  PyTuple_SET_ITEM(state, 1, PyBool_FromLong(_this->get_respect_prev_transform()));
  PyTuple_SET_ITEM(state, 2, PyLong_FromLong((long)num_colliders));

  for (size_t i = 0; i < num_colliders; ++i) {
    NodePath *collider = new NodePath(_this->get_collider(i));
    PyTuple_SET_ITEM(state, i * 2 + 3,
      DTool_CreatePyInstance((void *)collider, Dtool_NodePath, true, false));

    PT(CollisionHandler) handler = _this->get_handler(*collider);
    handler->ref();
    PyTuple_SET_ITEM(state, i * 2 + 4,
      DTool_CreatePyInstanceTyped((void *)handler.p(), Dtool_CollisionHandler, true, false, handler->get_type_index()));
    handler.cheat() = nullptr;
  }

  return state;
}

/**
 * Takes the value returned by __getstate__ and uses it to freshly initialize
 * this CollisionTraverser object.
 */
void Extension<CollisionTraverser>::
__setstate__(PyObject *state) {
  _this->clear_colliders();

  Py_ssize_t len = 0;
  const char *data = PyUnicode_AsUTF8AndSize(PyTuple_GET_ITEM(state, 0), &len);
  _this->set_name(std::string(data, len));

  _this->set_respect_prev_transform(PyTuple_GET_ITEM(state, 1) != Py_False);
  size_t num_colliders = (size_t)PyLong_AsLong(PyTuple_GET_ITEM(state, 2));

  for (size_t i = 0; i < num_colliders; ++i) {
    NodePath *collider = (NodePath *)DtoolInstance_VOID_PTR(PyTuple_GET_ITEM(state, i * 2 + 3));
    CollisionHandler *handler = (CollisionHandler *)DtoolInstance_VOID_PTR(PyTuple_GET_ITEM(state, i * 2 + 4));

    _this->add_collider(*collider, handler);
  }
}

#endif
