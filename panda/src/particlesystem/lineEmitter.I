// Filename: lineEmitter.I
// Created by:  charles (26Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//    Function : set_endpoint1
//      Access : Public
// Description : endpoint assignment
////////////////////////////////////////////////////////////////////
INLINE void LineEmitter::
set_endpoint1(const LPoint3f& point) {
  _endpoint1 = point;
}

////////////////////////////////////////////////////////////////////
//    Function : set_endpoint2
//      Access : Public
// Description : endpoint assignment
////////////////////////////////////////////////////////////////////
INLINE void LineEmitter::
set_endpoint2(const LPoint3f& point) {
  _endpoint2 = point;
}

////////////////////////////////////////////////////////////////////
//    Function : get_endpoint1
//      Access : Public
// Description : endpoint accessor
////////////////////////////////////////////////////////////////////
INLINE LPoint3f LineEmitter::
get_endpoint1() const {
  return _endpoint1;
}

////////////////////////////////////////////////////////////////////
//    Function : get_endpoint2
//      Access : Public
// Description : endpoint accessor
////////////////////////////////////////////////////////////////////
INLINE LPoint3f LineEmitter::
get_endpoint2() const {
  return _endpoint2;
}
