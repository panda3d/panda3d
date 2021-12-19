/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionPolygon_ext.h
 * @author Derzsi Daniel
 * @date 2020-10-13
 */

#ifndef COLLISIONPOLYGON_EXT_H
#define COLLISIONPOLYGON_EXT_H

#include "pandabase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "collisionPolygon.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for CollisionPolygon, which are called
 * instead of any C++ methods with the same prototype.
 *
 * @since 1.11.0
 */
template<>
class Extension<CollisionPolygon> : public ExtensionBase<CollisionPolygon> {
public:
  static bool verify_points(PyObject *points);
  void setup_points(PyObject *points);

private:
  static bool convert_points(pvector<LPoint3> &vec, PyObject *points);
};

#endif  // HAVE_PYTHON

#endif
