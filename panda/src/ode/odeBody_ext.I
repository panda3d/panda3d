/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeBody_ext.I
 * @author rdb
 * @date 2013-12-11
 */

#include "odeJoint_ext.h"

/**
 * Returns the custom data associated with the OdeBody.
 */
INLINE PyObject *Extension<OdeBody>::
get_data() const {
  PyObject *data = (PyObject *)_this->get_data();
  if (data == nullptr) {
    data = Py_None;
  }
  Py_INCREF(data);
  return data;
}

/**
 * Equivalent to get_joint().convert()
 */
INLINE PyObject *Extension<OdeBody>::
get_converted_joint(int i) const {
  OdeJoint j = _this->get_joint(i);
  return invoke_extension(&j).convert();
}
