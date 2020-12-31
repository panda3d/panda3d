/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerEvent_ext.cxx
 * @author rdb
 * @date 2020-12-31
 */

#include "collisionHandlerEvent_ext.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickling behavior.
 */
PyObject *Extension<CollisionHandlerEvent>::
__getstate__() const {
  PyObject *state = PyTuple_New(3);
  if (state == nullptr) {
    return nullptr;
  }

  size_t num_patterns;
  PyObject *patterns;

  num_patterns = _this->get_num_in_patterns();
  patterns = PyTuple_New(num_patterns);
  for (size_t i = 0; i < num_patterns; ++i) {
    std::string pattern = _this->get_in_pattern(i);
#if PY_MAJOR_VERSION >= 3
    PyTuple_SET_ITEM(patterns, i, PyUnicode_FromStringAndSize(pattern.data(), pattern.size()));
#else
    PyTuple_SET_ITEM(patterns, i, PyString_FromStringAndSize(pattern.data(), pattern.size()));
#endif
  }
  PyTuple_SET_ITEM(state, 0, patterns);

  num_patterns = _this->get_num_again_patterns();
  patterns = PyTuple_New(num_patterns);
  for (size_t i = 0; i < num_patterns; ++i) {
    std::string pattern = _this->get_again_pattern(i);
#if PY_MAJOR_VERSION >= 3
    PyTuple_SET_ITEM(patterns, i, PyUnicode_FromStringAndSize(pattern.data(), pattern.size()));
#else
    PyTuple_SET_ITEM(patterns, i, PyString_FromStringAndSize(pattern.data(), pattern.size()));
#endif
  }
  PyTuple_SET_ITEM(state, 1, patterns);

  num_patterns = _this->get_num_out_patterns();
  patterns = PyTuple_New(num_patterns);
  for (size_t i = 0; i < num_patterns; ++i) {
    std::string pattern = _this->get_out_pattern(i);
#if PY_MAJOR_VERSION >= 3
    PyTuple_SET_ITEM(patterns, i, PyUnicode_FromStringAndSize(pattern.data(), pattern.size()));
#else
    PyTuple_SET_ITEM(patterns, i, PyString_FromStringAndSize(pattern.data(), pattern.size()));
#endif
  }
  PyTuple_SET_ITEM(state, 2, patterns);

  return state;
}

/**
 * Takes the value returned by __getstate__ and uses it to freshly initialize
 * this CollisionHandlerEvent object.
 */
void Extension<CollisionHandlerEvent>::
__setstate__(PyObject *state) {
  nassertv(Py_SIZE(state) >= 3);

  PyObject *patterns;

  _this->clear_in_patterns();
  patterns = PyTuple_GET_ITEM(state, 0);
  for (size_t i = 0; i < Py_SIZE(patterns); ++i) {
    PyObject *pattern = PyTuple_GET_ITEM(patterns, i);
    Py_ssize_t len = 0;
#if PY_MAJOR_VERSION >= 3
    const char *data = PyUnicode_AsUTF8AndSize(pattern, &len);
#else
    char *data;
    PyString_AsStringAndSize(pattern, &data, &len);
#endif
    _this->add_in_pattern(std::string(data, len));
  }

  _this->clear_again_patterns();
  patterns = PyTuple_GET_ITEM(state, 1);
  for (size_t i = 0; i < Py_SIZE(patterns); ++i) {
    PyObject *pattern = PyTuple_GET_ITEM(patterns, i);
    Py_ssize_t len = 0;
#if PY_MAJOR_VERSION >= 3
    const char *data = PyUnicode_AsUTF8AndSize(pattern, &len);
#else
    char *data;
    PyString_AsStringAndSize(pattern, &data, &len);
#endif
    _this->add_again_pattern(std::string(data, len));
  }

  _this->clear_out_patterns();
  patterns = PyTuple_GET_ITEM(state, 2);
  for (size_t i = 0; i < Py_SIZE(patterns); ++i) {
    PyObject *pattern = PyTuple_GET_ITEM(patterns, i);
    Py_ssize_t len = 0;
#if PY_MAJOR_VERSION >= 3
    const char *data = PyUnicode_AsUTF8AndSize(pattern, &len);
#else
    char *data;
    PyString_AsStringAndSize(pattern, &data, &len);
#endif
    _this->add_out_pattern(std::string(data, len));
  }
}

#endif
