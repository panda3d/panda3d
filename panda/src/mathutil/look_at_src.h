// Filename: look_at_src.h
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

// These functions return a matrix that rotates between a coordinate
// system defined with the given forward and up vectors, and the
// standard coordinate system with y-forward and z-up.  They differ
// only in their behavior when the supplied forward and up vectors are
// not perpendicular; in this case, look_at will match the forward
// vector precisely, while heads_up will match the up vector
// precisely.

// Since these functions only return a rotation matrix, the
// translation component is always zero.  There are flavors of these
// functions that simply return the upper 3x3 part of the matrix, and
// flavors that return the whole 4x4 matrix with a zero bottom row.

BEGIN_PUBLISH

EXPCL_PANDA void
heads_up(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
         const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
         CoordinateSystem cs = CS_default);
EXPCL_PANDA void
look_at(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
        const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
        CoordinateSystem cs = CS_default);

INLINE_MATHUTIL void
heads_up(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
         CoordinateSystem cs);
INLINE_MATHUTIL void
look_at(FLOATNAME(LMatrix3) &mat, const FLOATNAME(LVector3) &fwd,
        CoordinateSystem cs);


INLINE_MATHUTIL void
heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
         const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
         CoordinateSystem cs = CS_default);
INLINE_MATHUTIL void
look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
        const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
        CoordinateSystem cs = CS_default);

INLINE_MATHUTIL void
heads_up(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
         CoordinateSystem cs);
INLINE_MATHUTIL void
look_at(FLOATNAME(LMatrix4) &mat, const FLOATNAME(LVector3) &fwd,
        CoordinateSystem cs);



INLINE_MATHUTIL void
heads_up(FLOATNAME(LQuaternion) &quat, const FLOATNAME(LVector3) &fwd,
         const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
         CoordinateSystem cs = CS_default);
INLINE_MATHUTIL void
look_at(FLOATNAME(LQuaternion) &quat, const FLOATNAME(LVector3) &fwd,
        const FLOATNAME(LVector3) &up = FLOATNAME(LVector3)::up(),
        CoordinateSystem cs = CS_default);

INLINE_MATHUTIL void
heads_up(FLOATNAME(LQuaternion) &quat, const FLOATNAME(LVector3) &fwd,
         CoordinateSystem cs);
INLINE_MATHUTIL void
look_at(FLOATNAME(LQuaternion) &quat, const FLOATNAME(LVector3) &fwd,
        CoordinateSystem cs);

END_PUBLISH

#include "look_at_src.I"
