/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingHexahedron.I
 * @author drose
 * @date 1999-10-03
 */

/**
 *
 */
INLINE_MATHUTIL BoundingHexahedron::
BoundingHexahedron() {
}

/**
 * Returns 8: the number of vertices of a hexahedron.
 */
INLINE_MATHUTIL int BoundingHexahedron::
get_num_points() const {
  return num_points;
}

/**
 * Returns the nth vertex of the hexahedron.
 */
INLINE_MATHUTIL LPoint3 BoundingHexahedron::
get_point(int n) const {
  nassertr(n >= 0 && n < num_points, LPoint3::zero());
  return _points[n];
}

/**
 * Returns 6: the number of faces of a hexahedron.
 */
INLINE_MATHUTIL int BoundingHexahedron::
get_num_planes() const {
  return num_planes;
}

/**
 * Returns the nth face of the hexahedron.
 */
INLINE_MATHUTIL LPlane BoundingHexahedron::
get_plane(int n) const {
  nassertr(n >= 0 && n < num_planes, LPlane());
  return _planes[n];
}
