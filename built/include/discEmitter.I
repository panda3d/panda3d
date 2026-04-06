/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file discEmitter.I
 * @author charles
 * @date 2000-06-22
 */

/**
 * radius assignment
 */

INLINE void DiscEmitter::
set_radius(PN_stdfloat r) {
  _radius = r;
}

/**
 * aoe assignement
 */

INLINE void DiscEmitter::
set_outer_angle(PN_stdfloat o_angle) {
  _outer_aoe = o_angle;
}

/**
 * aoe assignment
 */

INLINE void DiscEmitter::
set_inner_angle(PN_stdfloat i_angle) {
  _inner_aoe = i_angle;
}

/**
 * mag assignment
 */

INLINE void DiscEmitter::
set_outer_magnitude(PN_stdfloat o_mag) {
  _outer_magnitude = o_mag;
}

/**
 * mag assignment
 */

INLINE void DiscEmitter::
set_inner_magnitude(PN_stdfloat i_mag) {
  _inner_magnitude = i_mag;
}

/**
 * clerp flag
 */

INLINE void DiscEmitter::
set_cubic_lerping(bool clerp) {
  _cubic_lerping = clerp;
}

/**
 * radius accessor
 */

INLINE PN_stdfloat DiscEmitter::
get_radius() const {
  return _radius;
}

/**
 * aoe accessor
 */

INLINE PN_stdfloat DiscEmitter::
get_outer_angle() const {
  return _outer_aoe;
}

/**
 * aoe accessor
 */

INLINE PN_stdfloat DiscEmitter::
get_inner_angle() const {
  return _inner_aoe;
}

/**
 * mag accessor
 */

INLINE PN_stdfloat DiscEmitter::
get_outer_magnitude() const {
  return _outer_magnitude;
}

/**
 * mag accessor
 */

INLINE PN_stdfloat DiscEmitter::
get_inner_magnitude() const {
  return _inner_magnitude;
}

/**
 * clerp flag accessor
 */

INLINE bool DiscEmitter::
get_cubic_lerping() const {
  return _cubic_lerping;
}
