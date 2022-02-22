/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshPath.I
 * @author Maxwell175
 * @date 2022-02-21
 */


/**
 * Returns the vector of points that this path is based on.
 */
INLINE pvector<LPoint3> NavMeshPath::
get_points() const {
  return _points;
}

/**
 * Returns the number of points that are contained in this path.
 */
INLINE size_t NavMeshPath::
get_num_points() const {
  return _points.size();
}

/**
 * Gets the point at the specified index.
 */
INLINE LPoint3 NavMeshPath::
get_point(int index) const {
  return _points[index];
}
