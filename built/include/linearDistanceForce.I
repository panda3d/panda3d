/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearDistanceForce.I
 * @author charles
 * @date 2000-06-21
 */

/**
 * falloff_type encapsulating wrap
 */
INLINE void LinearDistanceForce::
set_falloff_type(FalloffType ft) {
  _falloff = ft;
}

/**
 * set the radius
 */
INLINE void LinearDistanceForce::
set_radius(PN_stdfloat r) {
  _radius = r;
}

/**
 * set the force center
 */
INLINE void LinearDistanceForce::
set_force_center(const LPoint3& p) {
  _force_center = p;
}

/**
 * falloff_type query
 */
INLINE LinearDistanceForce::FalloffType LinearDistanceForce::
get_falloff_type() const {
  return _falloff;
}

/**
 * radius query
 */
INLINE PN_stdfloat LinearDistanceForce::
get_radius() const {
  return _radius;
}

/**
 * force_center query
 */
INLINE LPoint3 LinearDistanceForce::
get_force_center() const {
  return _force_center;
}

/**
 * calculate the term based on falloff
 */
INLINE PN_stdfloat LinearDistanceForce::
get_scalar_term() const {
  PN_stdfloat r = _radius;
  if (_falloff == FT_ONE_OVER_R_SQUARED)
    r = r * r;
  else if (_falloff == FT_ONE_OVER_R_CUBED)
    r = r * r * r;

  return (1.0f / r);
}
