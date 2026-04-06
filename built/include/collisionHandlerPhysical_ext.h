/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerPhysical_ext.h
 * @author rdb
 * @date 2020-12-31
 */

#ifndef COLLISIONHANDLERPHYSICAL_EXT_H
#define COLLISIONHANDLERPHYSICAL_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "collisionHandlerPhysical.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for CollisionHandlerPhysical, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<CollisionHandlerPhysical> : public ExtensionBase<CollisionHandlerPhysical> {
public:
  PyObject *__reduce__(PyObject *self) const;
  void __setstate__(PyObject *self, vector_uchar data, PyObject *nodepaths);
};

#endif  // HAVE_PYTHON

#endif  // COLLISIONHANDLERPHYSICAL_EXT_H
