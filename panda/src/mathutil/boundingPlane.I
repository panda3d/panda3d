/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingPlane.I
 * @author drose
 * @date 2005-08-19
 */

/**
 * Constructs an empty "plane" that has no intersections.
 */
INLINE_MATHUTIL BoundingPlane::
BoundingPlane() {
}

/**
 *
 */
INLINE_MATHUTIL BoundingPlane::
BoundingPlane(const LPlane &plane) :
  _plane(plane)
{
  _flags = 0;
}

/**
 *
 */
INLINE_MATHUTIL const LPlane &BoundingPlane::
get_plane() const {
  nassertr(!is_empty(), _plane);
  nassertr(!is_infinite(), _plane);
  return _plane;
}
