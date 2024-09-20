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
  pvector<LPoint3> vec;
  if (!convert_points(vec, points)) {
    return false;
  }

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
  pvector<LPoint3> vec;
  if (!convert_points(vec, points)) {
    return;
  }

  if (vec.size() < 3) {
    PyErr_SetString(PyExc_ValueError, "expected at least 3 points");
    return;
  }

  const LPoint3 *verts_begin = &vec[0];
  const LPoint3 *verts_end = verts_begin + vec.size();

  _this->setup_points(verts_begin, verts_end);
}

/**
 * Converts a Python sequence to a list of LPoint3 objects.
 */
bool Extension<CollisionPolygon>::
convert_points(pvector<LPoint3> &vec, PyObject *points) {
  PyObject *seq = PySequence_Fast(points, "function expects a sequence");
  if (!seq) {
    return false;
  }

  bool success = true;

  Py_BEGIN_CRITICAL_SECTION(seq);
  PyObject **items = PySequence_Fast_ITEMS(seq);
  Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);
  void *ptr;

  vec.reserve(len);

  for (Py_ssize_t i = 0; i < len; ++i) {
#ifdef STDFLOAT_DOUBLE
    if (DtoolInstance_Check(items[i]) &&
        (ptr = DtoolInstance_UPCAST(items[i], Dtool_LPoint3d))) {
#else
    if (DtoolInstance_Check(items[i]) &&
        (ptr = DtoolInstance_UPCAST(items[i], Dtool_LPoint3f))) {
#endif
      vec.push_back(*(LPoint3 *)ptr);
    }
    else {
      Dtool_Raise_TypeError("Argument must be of LPoint3 type.");
      success = false;
      break;
    }
  }

  Py_END_CRITICAL_SECTION();
  Py_DECREF(seq);
  return success;
}

#endif
