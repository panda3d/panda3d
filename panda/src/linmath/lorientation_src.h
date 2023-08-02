/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lorientation_src.h
 * @author frang, charles
 * @date 2000-06-23
 */

class FLOATNAME(LRotation);

/**
 * This is a unit quaternion representing an orientation.
 */
class EXPCL_PANDA_LINMATH FLOATNAME(LOrientation) : public FLOATNAME(LQuaternion) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LOrientation)();
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LQuaternion) &c);
  INLINE_LINMATH FLOATNAME(LOrientation)(FLOATTYPE r, FLOATTYPE i, FLOATTYPE j, FLOATTYPE k);
  INLINE_LINMATH explicit FLOATNAME(LOrientation)(const FLOATNAME(LVector3) &point_at, FLOATTYPE twist);
  INLINE_LINMATH explicit FLOATNAME(LOrientation)(const FLOATNAME(LMatrix3) &m);
  INLINE_LINMATH explicit FLOATNAME(LOrientation)(const FLOATNAME(LMatrix4) &m);

  INLINE_LINMATH FLOATNAME(LOrientation)
  operator * (const FLOATNAME(LRotation) &other) const;

  INLINE_LINMATH FLOATNAME(LOrientation)
  operator * (const FLOATNAME(LQuaternion) &other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

#include "lorientation_src.I"
