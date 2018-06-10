/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lpoint4_src.h
 * @author drose
 * @date 2000-03-08
 */

/**
 * This is a four-component point in space.
 */
class EXPCL_PANDA_LINMATH FLOATNAME(LPoint4) : public FLOATNAME(LVecBase4) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LPoint4)() = default;
  INLINE_LINMATH FLOATNAME(LPoint4)(const FLOATNAME(LVecBase4) &copy);
  INLINE_LINMATH FLOATNAME(LPoint4)(FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LPoint4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w);
  INLINE_LINMATH FLOATNAME(LPoint4)(const FLOATNAME(LVecBase3) &copy, FLOATTYPE w);

  EXTENSION(INLINE_LINMATH PyObject *__getattr__(PyObject *self, const std::string &attr_name) const);
  EXTENSION(INLINE_LINMATH int __setattr__(PyObject *self, const std::string &attr_name, PyObject *assign));

  INLINE_LINMATH static const FLOATNAME(LPoint4) &zero();
  INLINE_LINMATH static const FLOATNAME(LPoint4) &unit_x();
  INLINE_LINMATH static const FLOATNAME(LPoint4) &unit_y();
  INLINE_LINMATH static const FLOATNAME(LPoint4) &unit_z();
  INLINE_LINMATH static const FLOATNAME(LPoint4) &unit_w();

  INLINE_LINMATH FLOATNAME(LPoint3) get_xyz() const;
  INLINE_LINMATH FLOATNAME(LPoint2) get_xy() const;

  MAKE_PROPERTY(xyz, get_xyz);
  MAKE_PROPERTY(xy, get_xy);

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

#ifndef FLOATTYPE_IS_INT
  INLINE_LINMATH FLOATNAME(LPoint4) normalized() const;
  INLINE_LINMATH FLOATNAME(LPoint4) project(const FLOATNAME(LVecBase4) &onto) const;
#endif

  EXTENSION(INLINE_LINMATH std::string __repr__() const);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "lpoint4_src.I"
