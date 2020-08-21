/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lpoint3_src.h
 * @author drose
 * @date 1999-09-25
 */

/**
 * This is a three-component point in space (as opposed to a three-component
 * vector, which represents a direction and a distance).  Some of the methods
 * are slightly different between LPoint3 and LVector3; in particular,
 * subtraction of two points yields a vector, while addition of a vector and a
 * point yields a point.
 */
class EXPCL_PANDA_LINMATH FLOATNAME(LPoint3) : public FLOATNAME(LVecBase3) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LPoint3)() = default;
  INLINE_LINMATH FLOATNAME(LPoint3)(const FLOATNAME(LVecBase3) &copy);
  INLINE_LINMATH FLOATNAME(LPoint3)(FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LPoint3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z);
  INLINE_LINMATH FLOATNAME(LPoint3)(const FLOATNAME(LVecBase2) &copy, FLOATTYPE z);

  EXTENSION(INLINE_LINMATH PyObject *__getattr__(PyObject *self, const std::string &attr_name) const);
  EXTENSION(INLINE_LINMATH int __setattr__(PyObject *self, const std::string &attr_name, PyObject *assign));

  INLINE_LINMATH static const FLOATNAME(LPoint3) &zero();
  INLINE_LINMATH static const FLOATNAME(LPoint3) &unit_x();
  INLINE_LINMATH static const FLOATNAME(LPoint3) &unit_y();
  INLINE_LINMATH static const FLOATNAME(LPoint3) &unit_z();

  INLINE_LINMATH FLOATNAME(LPoint2) get_xy() const;
  INLINE_LINMATH FLOATNAME(LPoint2) get_xz() const;
  INLINE_LINMATH FLOATNAME(LPoint2) get_yz() const;

  MAKE_PROPERTY(xy, get_xy);
  MAKE_PROPERTY(xz, get_xz);
  MAKE_PROPERTY(yz, get_yz);

  INLINE_LINMATH FLOATNAME(LPoint3) operator - () const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  operator + (const FLOATNAME(LVecBase3) &other) const;
  INLINE_LINMATH FLOATNAME(LPoint3)
  operator + (const FLOATNAME(LVector3) &other) const;

  INLINE_LINMATH FLOATNAME(LVecBase3)
  operator - (const FLOATNAME(LVecBase3) &other) const;
  INLINE_LINMATH FLOATNAME(LVector3)
  operator - (const FLOATNAME(LPoint3) &other) const;
  INLINE_LINMATH FLOATNAME(LPoint3)
  operator - (const FLOATNAME(LVector3) &other) const;

  INLINE_LINMATH FLOATNAME(LPoint3) cross(const FLOATNAME(LVecBase3) &other) const;

#ifndef FLOATTYPE_IS_INT
  INLINE_LINMATH FLOATNAME(LPoint3) normalized() const;
  INLINE_LINMATH FLOATNAME(LPoint3) project(const FLOATNAME(LVecBase3) &onto) const;
#endif

  INLINE_LINMATH FLOATNAME(LPoint3) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LPoint3) operator / (FLOATTYPE scalar) const;

  // Some special named constructors for LPoint3.

  INLINE_LINMATH static const FLOATNAME(LPoint3) &origin(CoordinateSystem cs = CS_default);
  INLINE_LINMATH static FLOATNAME(LPoint3) rfu(FLOATTYPE right,
                                       FLOATTYPE fwd,
                                       FLOATTYPE up,
                                       CoordinateSystem cs = CS_default);

  EXTENSION(INLINE_LINMATH std::string __repr__() const);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "lpoint3_src.I"
