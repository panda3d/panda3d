/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionTraverser_ext.h
 * @author rdb
 * @date 2020-12-31
 */

#ifndef COLLISIONTRAVERSER_EXT_H
#define COLLISIONTRAVERSER_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "collisionTraverser.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for CollisionTraverser, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<CollisionTraverser> : public ExtensionBase<CollisionTraverser> {
public:
  PyObject *__getstate__() const;
  void __setstate__(PyObject *state);
};

#endif  // HAVE_PYTHON

#endif  // COLLISIONTRAVERSER_EXT_H
