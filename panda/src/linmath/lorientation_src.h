// Filename: lorientation_src.h
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
//       Class : LOrientation
// Description : This is a unit quaternion representing an orientation.
////////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(LOrientation) : public FLOATNAME(LQuaternion) {
PUBLISHED:
  INLINE_LINMATH FLOATNAME(LOrientation)();
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LQuaternion)&);
  INLINE_LINMATH FLOATNAME(LOrientation)(FLOATTYPE, FLOATTYPE, FLOATTYPE, FLOATTYPE);
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LVector3) &, float);
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LMatrix3) &);
  INLINE_LINMATH FLOATNAME(LOrientation)(const FLOATNAME(LMatrix4) &);

  INLINE_LINMATH FLOATNAME(LOrientation)
  operator *(const FLOATNAME(LRotation)& other) const;

  INLINE_LINMATH FLOATNAME(LOrientation)
  operator *(const FLOATNAME(LQuaternion)& other) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
private:
  static TypeHandle _type_handle;
};

#include "lorientation_src.I"
