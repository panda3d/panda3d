/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionNode_ext.h
 * @author rdb
 * @date 2024-12-12
 */

#ifndef COLLISIONNODE_EXT_H
#define COLLISIONNODE_EXT_H

#include "pandabase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "collisionNode.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for CollisionNode, which are called
 * instead of any C++ methods with the same prototype.
 *
 * @since 1.11.0
 */
template<>
class Extension<CollisionNode> : public ExtensionBase<CollisionNode> {
public:
  PyObject *get_owner() const;
  void set_owner(PyObject *owner);
};

#endif  // HAVE_PYTHON

#endif
