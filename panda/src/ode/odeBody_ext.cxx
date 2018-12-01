/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBody_ext.cxx
 * @author rdb
 * @date 2018-11-06
 */

#include "odeBody_ext.h"

static void destroy_callback(OdeBody &body) {
  Py_XDECREF((PyObject *)body.get_data());
}

/**
 * Sets custom data to be associated with the OdeBody.
 */
void Extension<OdeBody>::
set_data(PyObject *data) {
  void *old_data = _this->get_data();

  if (data != nullptr && data != Py_None) {
    Py_INCREF(data);
    _this->set_data((void *)data);
    _this->_destroy_callback = &destroy_callback;
  } else {
    _this->set_data(nullptr);
    _this->_destroy_callback = nullptr;
  }

  Py_XDECREF((PyObject *)old_data);
}
