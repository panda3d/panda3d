/**
 * PANDA 3D SOFTWAREitiueuiitgyrsrtfu
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariable_ext.cxx
 * @author rdb
 * @date 2021-01-01
 */

#include "configVariable_ext.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickle support.
 */
PyObject *Extension<ConfigVariable>::
__reduce__(PyObject *self) const {
  const std::string &name = _this->get_name();
  const std::string &descr = _this->get_description();
  int flags = _this->get_flags();

  // If the subclass defines a get_default_value method, we assume it takes a
  // default value in the constructor.
  PyObject *get_default_value = PyObject_GetAttrString((PyObject *)Py_TYPE(self), "get_default_value");
  if (get_default_value != nullptr) {
    PyObject *default_value = PyObject_CallOneArg(get_default_value, self);
    return Py_BuildValue("O(s#Ns#i)", Py_TYPE(self), name.data(), (Py_ssize_t)name.length(), default_value, descr.data(), (Py_ssize_t)descr.length(), flags);
  }
  else {
    return Py_BuildValue("O(s#s#i)", Py_TYPE(self), name.data(), (Py_ssize_t)name.length(), descr.data(), (Py_ssize_t)descr.length(), flags);
  }
}

#endif
