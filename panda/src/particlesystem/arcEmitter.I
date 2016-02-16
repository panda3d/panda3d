/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file arcEmitter.I
 * @author charles
 * @date 2000-06-26
 */

/**
 * start angle set
 */

INLINE void ArcEmitter::
set_start_angle(PN_stdfloat angle) {
  _start_theta = deg_2_rad(angle);
}

/**
 * end angle set
 */

INLINE void ArcEmitter::
set_end_angle(PN_stdfloat angle) {
  _end_theta = deg_2_rad(angle);
}

/**
 * arc sweep set
 */

INLINE void ArcEmitter::
set_arc(PN_stdfloat startAngle, PN_stdfloat endAngle) {
  _start_theta = deg_2_rad(startAngle);
  _end_theta = deg_2_rad(endAngle);
}

/**
 * get start angle
 */

INLINE PN_stdfloat ArcEmitter::
get_start_angle() {
  return rad_2_deg(_start_theta);
}

/**
 * get end angle
 */

INLINE PN_stdfloat ArcEmitter::
get_end_angle() {
  return rad_2_deg(_end_theta);
}
