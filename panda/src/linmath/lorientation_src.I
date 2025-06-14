/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lorientation_src.I
 * @author frang, charles
 * @date 2000-06-23
 */

/**
 *
 */
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)() {
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LQuaternion)& c) :
  FLOATNAME(LQuaternion)(c) {
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(FLOATTYPE r, FLOATTYPE i, FLOATTYPE j, FLOATTYPE k) :
  FLOATNAME(LQuaternion)(r, i, j, k) {
}

/**
 * vector + twist
 */
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LVector3) &point_at, FLOATTYPE twist) {
  FLOATTYPE radians = deg_2_rad(twist);
  FLOATTYPE theta_over_2 = radians * FLOATCONST(0.5);
  FLOATTYPE sin_to2 = csin(theta_over_2);

  set_r(ccos(theta_over_2));
  set_i(point_at[0] * sin_to2);
  set_j(point_at[1] * sin_to2);
  set_k(point_at[2] * sin_to2);
}

/**
 * matrix3
 */
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LMatrix3) &m) {
  set_from_matrix(m);
}

/**
 * matrix4
 */
INLINE_LINMATH FLOATNAME(LOrientation)::
FLOATNAME(LOrientation)(const FLOATNAME(LMatrix4) &m) {
  set_from_matrix(m);
}

/**
 * Orientation * rotation = Orientation Applies a rotation to an orientation.
 */
INLINE_LINMATH FLOATNAME(LOrientation) FLOATNAME(LOrientation)::
operator * (const FLOATNAME(LRotation) &other) const {
  return multiply((FLOATNAME(LOrientation) &)other);
}

/**
 * Orientation * Orientation This is a meaningless operation, and will always
 * simply return the rhs.
 */
INLINE_LINMATH FLOATNAME(LOrientation) FLOATNAME(LOrientation)::
operator * (const FLOATNAME(LQuaternion) &other) const {
  nassert_raise("LOrientation * LQuaternion is undefined; use LOrientation * LRotation or LQuaternion * LQuaternion");
  return multiply((FLOATNAME(LOrientation) &)other);
}
