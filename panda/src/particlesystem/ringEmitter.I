/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ringEmitter.I
 * @author charles
 * @date 2000-06-26
 */

/**
 * radius set
 */

INLINE void RingEmitter::
set_radius(PN_stdfloat r) {
  _radius = r;
}

/**
 * angle of elevation set
 */

INLINE void RingEmitter::
set_angle(PN_stdfloat angle) {
  _aoe = angle;
}

/**
 * radius_spread set
 */

INLINE void RingEmitter::
set_radius_spread(PN_stdfloat spread) {
  _radius_spread = spread;
}

/**
 * uniform_emission set
 */

INLINE void RingEmitter::
set_uniform_emission(int uniform_emission) {
  _uniform_emission = uniform_emission;
}

/**
 * radius get
 */

INLINE PN_stdfloat RingEmitter::
get_radius() const {
  return _radius;
}

/**
 * angle of elevation get
 */

INLINE PN_stdfloat RingEmitter::
get_angle() const {
  return _aoe;
}

/**
 * radius_spread get
 */

INLINE PN_stdfloat RingEmitter::
get_radius_spread() const {
  return _radius_spread;
}

/**
 * uniform_emission get
 */

INLINE int RingEmitter::
get_uniform_emission() const {
  return _uniform_emission;
}
