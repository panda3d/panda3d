/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingBox.I
 * @author drose
 * @date 2007-05-31
 */

/**
 * Constructs an empty box object.
 */
INLINE_MATHUTIL BoundingBox::
BoundingBox() {
}

/**
 * Constructs a specific box object.
 */
INLINE_MATHUTIL BoundingBox::
BoundingBox(const LPoint3 &min, const LPoint3 &max) :
  _min(min), _max(max)
{
#ifdef NDEBUG
  _flags = F_empty;
  nassertv(!_min.is_nan() && !_max.is_nan());
  nassertv(_min[0] <= _max[0] && _min[1] <= _max[1] && _min[2] <= _max[2]);
#endif  // NDEBUG
  _flags = 0;
}

/**
 * An inline accessor for the minimum value.  get_min() would also work, but
 * it is a virtual non-inline method.
 */
INLINE_MATHUTIL const LPoint3 &BoundingBox::
get_minq() const {
  nassertr(!is_empty(), _min);
  nassertr(!is_infinite(), _min);
  return _min;
}

/**
 * An inline accessor for the maximum value.  get_max() would also work, but
 * it is a virtual non-inline method.
 */
INLINE_MATHUTIL const LPoint3 &BoundingBox::
get_maxq() const {
  nassertr(!is_empty(), _max);
  nassertr(!is_infinite(), _max);
  return _max;
}

/**
 * Returns 8: the number of vertices of a rectangular solid.
 */
INLINE_MATHUTIL int BoundingBox::
get_num_points() const {
  return 8;
}

/**
 * Returns the nth vertex of the rectangular solid.
 */
INLINE_MATHUTIL LPoint3 BoundingBox::
get_point(int n) const {
  nassertr(n >= 0 && n < 8, LPoint3::zero());

  // We do some trickery assuming that _min and _max are consecutive in
  // memory.
  const LPoint3 *a = &_min;
  return LPoint3(a[(n>>2)&1][0], a[(n>>1)&1][1], a[(n)&1][2]);
}

/**
 * Returns 6: the number of faces of a rectangular solid.
 */
INLINE_MATHUTIL int BoundingBox::
get_num_planes() const {
  return 6;
}

/**
 * Returns the nth face of the rectangular solid.
 */
INLINE_MATHUTIL LPlane BoundingBox::
get_plane(int n) const {
  nassertr(n >= 0 && n < 6, LPlane());
  return LPlane(get_point(plane_def[n][0]),
                get_point(plane_def[n][1]),
                get_point(plane_def[n][2]));
}

/**
 * Sets the min and max point of the rectangular solid.
 */
INLINE_MATHUTIL void BoundingBox::
set_min_max(const LPoint3 &min, const LPoint3 &max) {
  nassertv(!min.is_nan() && !max.is_nan());
  nassertv(min[0] <= max[0] && min[1] <= max[1] && min[2] <= max[2]);
  _min = min;
  _max = max;
  _flags = 0;
}
