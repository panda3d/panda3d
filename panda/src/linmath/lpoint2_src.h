// Filename: lpoint2_src.h
// Created by:  drose (08Mar00)
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
//       Class : LPoint2
// Description : This is a two-component point in space.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LPoint2) : public FLOATNAME(LVecBase2) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LPoint2)();
  INLINE_LINMATH FLOATNAME(LPoint2)(const FLOATNAME(LVecBase2) &copy);
  INLINE_LINMATH FLOATNAME(LPoint2) &operator = (const FLOATNAME(LVecBase2) &copy);
  INLINE_LINMATH FLOATNAME(LPoint2) &operator = (FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LPoint2)(FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LPoint2)(FLOATTYPE x, FLOATTYPE y);

  INLINE_LINMATH static const FLOATNAME(LPoint2) &zero();
  INLINE_LINMATH static const FLOATNAME(LPoint2) &unit_x();
  INLINE_LINMATH static const FLOATNAME(LPoint2) &unit_y();

  INLINE_LINMATH FLOATNAME(LPoint2) operator - () const;

  INLINE_LINMATH FLOATNAME(LVecBase2)
  operator + (const FLOATNAME(LVecBase2) &other) const;
  INLINE_LINMATH FLOATNAME(LPoint2)
  operator + (const FLOATNAME(LVector2) &other) const;

  INLINE_LINMATH FLOATNAME(LVecBase2)
  operator - (const FLOATNAME(LVecBase2) &other) const;
  INLINE_LINMATH FLOATNAME(LVector2)
  operator - (const FLOATNAME(LPoint2) &other) const;
  INLINE_LINMATH FLOATNAME(LPoint2)
  operator - (const FLOATNAME(LVector2) &other) const;

  INLINE_LINMATH FLOATNAME(LPoint2) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LPoint2) operator / (FLOATTYPE scalar) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "lpoint2_src.I"
