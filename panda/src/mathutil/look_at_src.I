// Filename: look_at_src.I
// Created by:  drose (25Sep99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

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

