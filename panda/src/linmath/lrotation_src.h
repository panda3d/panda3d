/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lrotation_src.h
 * @author frang, charles
 * @date 2000-06-23
 */

/**
 * This is a unit quaternion representing a rotation.
 */
class EXPCL_PANDA_LINMATH FLOATNAME(LRotation) : public FLOATNAME(LQuaternion) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LRotation)();
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LQuaternion) &c);
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LVecBase4) &copy);
  INLINE_LINMATH FLOATNAME(LRotation)(FLOATTYPE r, FLOATTYPE i, FLOATTYPE j, FLOATTYPE k);
  INLINE_LINMATH explicit FLOATNAME(LRotation)(const FLOATNAME(LVector3) &axis, FLOATTYPE angle);
  INLINE_LINMATH explicit FLOATNAME(LRotation)(const FLOATNAME(LMatrix3) &m);
  INLINE_LINMATH explicit FLOATNAME(LRotation)(const FLOATNAME(LMatrix4) &m);
  INLINE_LINMATH explicit FLOATNAME(LRotation)(FLOATTYPE h, FLOATTYPE p, FLOATTYPE r);

  INLINE_LINMATH FLOATNAME(LRotation) operator * (FLOATTYPE scalar) const;
  INLINE_LINMATH FLOATNAME(LRotation) operator / (FLOATTYPE scalar) const;

  INLINE_LINMATH FLOATNAME(LRotation)
  operator * (const FLOATNAME(LRotation) &other) const;

  INLINE_LINMATH FLOATNAME(LQuaternion)
  operator * (const FLOATNAME(LQuaternion) &other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

#include "lrotation_src.I"
