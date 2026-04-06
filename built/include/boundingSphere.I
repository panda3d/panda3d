/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingSphere.I
 * @author drose
 * @date 1999-10-02
 */

/**
 * Constructs an empty sphere.
 */
INLINE_MATHUTIL BoundingSphere::
BoundingSphere() : _center(0) {
}

/**
 * Constructs a specific sphere.
 */
INLINE_MATHUTIL BoundingSphere::
BoundingSphere(const LPoint3 &center, PN_stdfloat radius) :
  _center(center), _radius(radius)
{
  _flags = 0;
  nassertd(!_center.is_nan() && !cnan(_radius)) {
    _flags = F_empty;
  }
}

/**
 *
 */
INLINE_MATHUTIL LPoint3 BoundingSphere::
get_center() const {
  nassertr(!is_infinite(), LPoint3::zero());
  return _center;
}

/**
 *
 */
INLINE_MATHUTIL PN_stdfloat BoundingSphere::
get_radius() const {
  nassertr(!is_empty(), 0.0f);
  nassertr(!is_infinite(), 0.0f);
  return _radius;
}

/**
 * Sets the center point of the sphere.
 */
INLINE_MATHUTIL void BoundingSphere::
set_center(const LPoint3 &center) {
  nassertv(!center.is_nan());
  _center = center;
}

/**
 * Sets the radius of the sphere.
 */
INLINE_MATHUTIL void BoundingSphere::
set_radius(PN_stdfloat radius) {
  nassertv(!cnan(radius));
  _radius = radius;
  _flags = 0;
}
