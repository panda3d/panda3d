// Filename: lorientation_src.h
// Created by:  frang, charles (23Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

class FLOATNAME(LRotation);

////////////////////////////////////////////////////////////////////////
//       Class : LOrientation
// Description : This is a unit quaternion representing an orientation.
////////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_LINMATH FLOATNAME(LOrientation) : public FLOATNAME(LQuaternion) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LOrientation)();
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LQuaternion)&);
  INLINE_LINMATH FLOATNAME(LOrientation)(FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE);
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LVector3) &, float);
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LMatrix3) &);
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LMatrix4) &);

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
