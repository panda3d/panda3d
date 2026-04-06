/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandler_ext.h
 * @author rdb
 * @date 2020-12-31
 */

#ifndef COLLISIONHANDLERQUEUE_EXT_H
#define COLLISIONHANDLERQUEUE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "collisionHandlerQueue.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for CollisionHandlerQueue, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<CollisionHandlerQueue> : public ExtensionBase<CollisionHandlerQueue> {
public:
  PyObject *__reduce__(PyObject *self) const;
};

#endif  // HAVE_PYTHON

#endif  // COLLISIONHANDLERQUEUE_EXT_H
