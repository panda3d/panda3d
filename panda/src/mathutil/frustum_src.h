// Filename: frustum_src.h
// Created by:  mike (09Jan97)
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

////////////////////////////////////////////////////////////////////
//       Class : Frustum
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(Frustum) {
PUBLISHED:
  INLINE_MATHUTIL FLOATNAME(Frustum)();

  INLINE_MATHUTIL void make_ortho_2D();
  INLINE_MATHUTIL void make_ortho_2D(FLOATTYPE l, FLOATTYPE r, FLOATTYPE t, FLOATTYPE b);

  INLINE_MATHUTIL void make_ortho(FLOATTYPE fnear, FLOATTYPE ffar);
  INLINE_MATHUTIL void make_ortho(FLOATTYPE fnear, FLOATTYPE ffar,
                         FLOATTYPE l, FLOATTYPE r, FLOATTYPE t, FLOATTYPE b);

  INLINE_MATHUTIL void make_perspective_hfov(FLOATTYPE xfov, FLOATTYPE aspect,
                                    FLOATTYPE fnear, FLOATTYPE ffar);
  INLINE_MATHUTIL void make_perspective_vfov(FLOATTYPE yfov, FLOATTYPE aspect,
                                    FLOATTYPE fnear, FLOATTYPE ffar);
  INLINE_MATHUTIL void make_perspective(FLOATTYPE xfov, FLOATTYPE yfov, FLOATTYPE fnear,
                               FLOATTYPE ffar);
  INLINE_MATHUTIL void get_perspective_params(FLOATTYPE &yfov, FLOATTYPE &aspect,
                                     FLOATTYPE &fnear, FLOATTYPE &ffar) const;
  INLINE_MATHUTIL void get_perspective_params(FLOATTYPE &xfov, FLOATTYPE &yfov,
                                     FLOATTYPE &aspect, FLOATTYPE &fnear,
                                     FLOATTYPE &ffar) const;

public:
  INLINE_MATHUTIL FLOATNAME(LMatrix4)
  get_perspective_projection_mat(CoordinateSystem cs = CS_default) const;

  INLINE_MATHUTIL FLOATNAME(LMatrix4)
  get_ortho_projection_mat(CoordinateSystem cs = CS_default) const;

public:
  FLOATTYPE _l, _r, _b, _t;
  FLOATTYPE _fnear, _ffar;
};

#include "frustum_src.I"
