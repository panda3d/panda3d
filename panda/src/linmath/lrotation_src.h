// Filename: lrotation_src.h
// Created by:  frang, charles (23Jun00)
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

////////////////////////////////////////////////////////////////////////
//       Class : LRotation
// Description : This is a unit quaternion representing a rotation.
////////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LRotation) : public FLOATNAME(LQuaternion) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LRotation)();
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LQuaternion)&);
  INLINE_LINMATH FLOATNAME(LRotation)(FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE);
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LVector3) &, FLOATTYPE);
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LMatrix3) &);
  INLINE_LINMATH FLOATNAME(LRotation)(const FLOATNAME(LMatrix4) &);
  INLINE_LINMATH FLOATNAME(LRotation)(FLOATTYPE, FLOATTYPE, FLOATTYPE);

  INLINE_LINMATH FLOATNAME(LRotation)
  operator*(const FLOATNAME(LRotation)& other) const;

  INLINE_LINMATH FLOATNAME(LQuaternion)
  operator*(const FLOATNAME(LQuaternion)& other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

#include "lrotation_src.I"
