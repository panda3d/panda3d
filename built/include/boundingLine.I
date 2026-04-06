/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingLine.I
 * @author drose
 * @date 2000-07-04
 */

/**
 *
 */
INLINE_MATHUTIL BoundingLine::
BoundingLine() {
}

/**
 *
 */
INLINE_MATHUTIL BoundingLine::
BoundingLine(const LPoint3 &a, const LPoint3 &b) :
  _origin(a), _vector(b - a)
{
  _vector.normalize();
  _flags = 0;
  nassertd(!_origin.is_nan() && !_vector.is_nan()) {
    _flags = F_empty;
  }
}

/**
 * Returns the first point that defines the line.
 */
INLINE_MATHUTIL const LPoint3 &BoundingLine::
get_point_a() const {
  nassertr(!is_empty(), _origin);
  nassertr(!is_infinite(), _origin);
  return _origin;
}

/**
 * Returns the second point that defines the line.
 */
INLINE_MATHUTIL LPoint3 BoundingLine::
get_point_b() const {
  nassertr(!is_empty(), _origin);
  nassertr(!is_infinite(), _origin);
  return _origin + _vector;
}
