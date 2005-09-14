// Filename: lpoint4_src.h
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
//       Class : LPoint4
// Description : This is a four-component point in space.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LPoint4) : public FLOATNAME(LVecBase4) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LPoint4)();
  INLINE_LINMATH FLOATNAME(LPoint4)(const FLOATNAME(LVecBase4) &copy);
  INLINE_LINMATH FLOATNAME(LPoint4) &operator = (const FLOATNAME(LVecBase4) &copy);
  INLINE_LINMATH FLOATNAME(LPoint4) &operator = (FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LPoint4)(FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LPoint4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w);

  INLINE_LINMATH static const FLOATNAME(LPoint4) &zero();
  INLINE_LINMATH static const FLOATNAME(LPoint4) &unit_x();
  INLINE_LINMATH static const FLOATNAME(LPoint4) &unit_y();
  INLINE_LINMATH static const FLOATNAME(LPoint4) &unit_z();
  INLINE_LINMATH static const FLOATNAME(LPoint4) &unit_w();

  INLINE_LINMATH FLOATNAME(LPoint4) operator - () const;

  INLINE_LINMATH FLOATNAME(LVecBase4)
  operator + (const FLOATNAME(LVecBase4) &other) const;
  INLINE_LINMATH FLOATNAME(LPoint4)
  operator + (const FLOATNAME(LVector4) &other) const;

  INLINE_LINMATH FLOATNAME(LVecBase4)
  operator - (const FLOATNAME(LVecBase4) &other) const;
  INLINE_LINMATH FLOATNAME(LVector4)
  operator - (const FLOATNAME(LPoint4) &other) const;
  INLINE_LINMATH FLOATNAME(LPoint4)
  operator - (const FLOATNAME(LVector4) &other) const;

  INLINE_LINMATH FLOATNAME(LPoint4) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LPoint4) operator / (FLOATTYPE scalar) const;

#ifdef HAVE_PYTHON
  INLINE_LINMATH void python_repr(ostream &out, const string &class_name) const;
#endif

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "lpoint4_src.I"
