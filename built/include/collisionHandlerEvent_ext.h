/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerEvent_ext.h
 * @author rdb
 * @date 2020-12-31
 */

#ifndef COLLISIONHANDLEREVENT_EXT_H
#define COLLISIONHANDLEREVENT_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "collisionHandlerEvent.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for CollisionHandlerEvent, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<CollisionHandlerEvent> : public ExtensionBase<CollisionHandlerEvent> {
public:
  PyObject *__reduce__(PyObject *self) const;
  void __setstate__(PyObject *self, vector_uchar data);
};

#endif  // HAVE_PYTHON

#endif  // COLLISIONHANDLEREVENT_EXT_H
