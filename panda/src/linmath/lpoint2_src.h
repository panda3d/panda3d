/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lpoint2_src.h
 * @author drose
 * @date 2000-03-08
 */

/**
 * This is a two-component point in space.
 */
class EXPCL_PANDA_LINMATH FLOATNAME(LPoint2) : public FLOATNAME(LVecBase2) {
PUBLISHED:

  INLINE_LINMATH FLOATNAME(LPoint2)() = default;
  INLINE_LINMATH FLOATNAME(LPoint2)(const FLOATNAME(LVecBase2)& copy);
  INLINE_LINMATH FLOATNAME(LPoint2)(FLOATTYPE fill_value);
  INLINE_LINMATH FLOATNAME(LPoint2)(FLOATTYPE x, FLOATTYPE y);

  EXTENSION(INLINE_LINMATH PyObject *__getattr__(PyObject *self, const std::string &attr_name) const);
  EXTENSION(INLINE_LINMATH int __setattr__(PyObject *self, const std::string &attr_name, PyObject *assign));

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

#ifndef FLOATTYPE_IS_INT
  INLINE_LINMATH FLOATNAME(LPoint2) normalized() const;
  INLINE_LINMATH FLOATNAME(LPoint2) project(const FLOATNAME(LVecBase2) &onto) const;
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

#include "lpoint2_src.I"
