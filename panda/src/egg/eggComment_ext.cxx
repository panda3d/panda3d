/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggComment_ext.cxx
 * @author rdb
 * @date 2021-01-01
 */

#include "eggComment_ext.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickle support.
 */
PyObject *Extension<EggComment>::
__reduce__() const {
  extern struct Dtool_PyTypedObject Dtool_EggComment;

  std::string node_name = _this->get_name();
  std::string comment = _this->get_comment();
  return Py_BuildValue("O(s#s#)", (PyObject *)&Dtool_EggComment,
    node_name.data(), (Py_ssize_t)node_name.length(),
    comment.data(), (Py_ssize_t)comment.length());
}

#endif
