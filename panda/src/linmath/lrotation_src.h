// Filename: lrotation_src.h
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

////////////////////////////////////////////////////////////////////////
//       Class : LRotation
// Description : This is a unit quaternion representing a rotation.
////////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_LINMATH FLOATNAME(LRotation) : public FLOATNAME(LQuaternion) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LRotation)();
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LQuaternion) &);
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LVecBase4) &copy);
  INLINE_LINMATH FLOATNAME(LRotation)(FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE);
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LVector3) &, FLOATTYPE);
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LMatrix3) &);
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LMatrix4) &);
  INLINE_LINMATH FLOATNAME(LRotation)(FLOATTYPE, FLOATTYPE, FLOATTYPE);

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
