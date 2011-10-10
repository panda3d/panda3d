// Filename: pointEmitter.I
// Created by:  charles (26Jun00)
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

////////////////////////////////////////////////////////////////////
//    Function : set_point
//      Access : public
// Description : point setting
////////////////////////////////////////////////////////////////////
INLINE void PointEmitter::
set_location(const LPoint3& p) {
  _location = p;
}

INLINE LPoint3 PointEmitter::
get_location() const {
  return _location;
}
