/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshPath.cxx
 * @author Maxwell175
 * @date 2022-02-20
 */

#include "navMeshPath.h"


/**
 * Makes NavMeshPath that wraps the given vector of points.
 */
NavMeshPath::NavMeshPath(pvector<LPoint3> &points) :
    _points(points) { }

/**
 * Copies an existing NavMeshPath object.
 */
NavMeshPath::NavMeshPath(const NavMeshPath &copy) :
    _points(copy.get_points()) { }

/**
 * Returns the total length of the path.
 */
PN_stdfloat NavMeshPath::get_length() {
  PN_stdfloat total_length = 0;

  for (size_t i = 1; i < _points.size(); i++) {
    total_length += (_points[i] - _points[i - 1]).length();
  }

  return total_length;
}
