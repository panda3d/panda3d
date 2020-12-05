/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionPolygon_ext.cxx
 * @author Derzsi Daniel
 * @date 2020-10-13
 */

#include "collisionPolygon_ext.h"

#ifdef HAVE_PYTHON

#include "collisionPolygon.h"

#ifdef STDFLOAT_DOUBLE
extern struct Dtool_PyTypedObject Dtool_LPoint3d;
#else
extern struct Dtool_PyTypedObject Dtool_LPoint3f;
#endif

/**
 * Verifies that the indicated Python list of points will define a
 * CollisionPolygon.
 */
bool Extension<CollisionPolygon>::
verify_points(PyObject *points) {
  const pvector<LPoint3> vec = convert_points(points);
  const LPoint3 *verts_begin = &vec[0];
  const LPoint3 *verts_end = verts_begin + vec.size();

  return CollisionPolygon::verify_points(verts_begin, verts_end);
}

/**
 * Initializes this CollisionPolygon with the given Python list of
 * points.
 */
void Extension<CollisionPolygon>::
setup_points(PyObject *points) {
  const pvector<LPoint3> vec = convert_points(points);
  const LPoint3 *verts_begin = &vec[0];
  const LPoint3 *verts_end = verts_begin + vec.size();

  _this->setup_points(verts_begin, verts_end);
}

/**
 * Converts a Python sequence to a list of LPoint3 objects.
 */
pvector<LPoint3> Extension<CollisionPolygon>::
convert_points(PyObject *points) {
  pvector<LPoint3> vec;
  PyObject *seq = PySequence_Fast(points, "function expects a sequence");

  if (!seq) {
    return vec;
  }

  PyObject **items = PySequence_Fast_ITEMS(seq);
  Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);
  void *ptr;

  vec.reserve(len);

  for (Py_ssize_t i = 0; i < len; ++i) {
#ifdef STDFLOAT_DOUBLE
    if (ptr = DtoolInstance_UPCAST(items[i], Dtool_LPoint3d)) {
#else
    if (ptr = DtoolInstance_UPCAST(items[i], Dtool_LPoint3f)) {
#endif
      vec.push_back(*(LPoint3 *)ptr);
    } else {
      collide_cat.warning() << "Argument must be of LPoint3 type.\n";
    }
  }

  Py_DECREF(seq);
  return vec;
}

#endif
