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

extern struct Dtool_PyTypedObject Dtool_LPoint3f;

/**
 * Verifies that the indicated Python list of points will define a
 * CollisionPolygon.
 */
bool Extension<CollisionPolygon>::
verify_points(PyObject *points) {
  pvector<LPoint3f> vec = convert_points(points);
  LPoint3f *verts_begin = &vec[0];
  LPoint3f *verts_end = verts_begin + vec.size();

  return _this->verify_points(verts_begin, verts_end);
}

/**
 * Initializes this CollisionPolygon with the given Python list of
 * points.
 */
void Extension<CollisionPolygon>::
setup_points(PyObject *points) {
  pvector<LPoint3f> vec = convert_points(points);
  LPoint3f *verts_begin = &vec[0];
  LPoint3f *verts_end = verts_begin + vec.size();

  _this->setup_points(verts_begin, verts_end);
}

/**
 * Converts a Python sequence to a list of LPoint3f objects.
 */
pvector<LPoint3f> Extension<CollisionPolygon>::
convert_points(PyObject *points) {
  pvector<LPoint3f> vec;
  PyObject *seq = PySequence_Fast(points, "function expects a sequence");

  if (seq == nullptr) {
    return vec;
  }

  PyObject **items = PySequence_Fast_ITEMS(seq);
  Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);
  void *ptr;

  vec.reserve(len);

  for (Py_ssize_t i = 0; i < len; ++i) {
    if (ptr = DtoolInstance_UPCAST(items[i], Dtool_LPoint3f)) {
      vec.push_back(*(LPoint3f *)ptr);
    }
  }

  Py_DECREF(seq);
  return vec;
}

#endif
