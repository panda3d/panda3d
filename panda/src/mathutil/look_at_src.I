/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file look_at_src.I
 * @author drose
 * @date 1999-09-25
 */

INLINE_MATHUTIL void
heads_up(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
         CoordinateSystem cs) {
  heads_up(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE_MATHUTIL void
look_at(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
        CoordinateSystem cs) {
  look_at(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE_MATHUTIL void
heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
         const FLOATNAME(LVector3) &up, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat3;
  heads_up(mat3, fwd, up, cs);
  mat = FLOATNAME(LMatrix4)(mat3);
}

INLINE_MATHUTIL void
look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
        const FLOATNAME(LVector3) &up, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat3;
  look_at(mat3, fwd, up, cs);
  mat = FLOATNAME(LMatrix4)(mat3);
}

INLINE_MATHUTIL void
heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
         CoordinateSystem cs) {
  heads_up(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE_MATHUTIL void
look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
        CoordinateSystem cs) {
  look_at(mat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE_MATHUTIL void
heads_up(FLOATNAME(LQuaternion) &quat, const FLOATNAME(LVector3) &fwd,
         const FLOATNAME(LVector3) &up, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat3;
  heads_up(mat3, fwd, up, cs);
  quat.set_from_matrix(mat3);
}

INLINE_MATHUTIL void
look_at(FLOATNAME(LQuaternion) &quat, const FLOATNAME(LVector3) &fwd,
        const FLOATNAME(LVector3) &up, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat3;
  look_at(mat3, fwd, up, cs);
  quat.set_from_matrix(mat3);
}

INLINE_MATHUTIL void
heads_up(FLOATNAME(LQuaternion) &quat, const FLOATNAME(LVector3) &fwd,
         CoordinateSystem cs) {
  heads_up(quat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}

INLINE_MATHUTIL void
look_at(FLOATNAME(LQuaternion) &quat, const FLOATNAME(LVector3) &fwd,
        CoordinateSystem cs) {
  look_at(quat, fwd, FLOATNAME(LVector3)::up(cs), cs);
}
