/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggGroupNode_ext.cxx
 * @author rdb
 * @date 2013-12-09
 */

#include "eggGroupNode_ext.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_EggNode;
#endif

/**
 * Returns a Python list containing the node's children.
 */
PyObject *Extension<EggGroupNode>::
get_children() const {
  EggGroupNode::iterator it;

  // Create the Python list object.
  EggGroupNode::size_type len = _this->size();
  PyObject *lst = PyList_New(len);
  if (lst == nullptr) {
    return nullptr;
  }

  // Fill in the list.
  size_t i = 0;
  for (it = _this->begin(); it != _this->end() && i < len; ++it) {
    EggNode *node = *it;
    node->ref();
    PyObject *item =
      DTool_CreatePyInstanceTyped((void *)node, Dtool_EggNode, true, false, node->get_type_index());

    PyList_SET_ITEM(lst, i++, item);
  }

  return lst;
}

#endif  // HAVE_PYTHON
