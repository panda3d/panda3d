// Filename: clipPlaneAttribute.I
// Created by:  drose (23Mar00)
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
//     Function: ClipPlaneAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ClipPlaneAttribute::
ClipPlaneAttribute() {
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::set_on
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ClipPlaneAttribute::
set_on(const PT(PlaneNode) &property) {
  MultiNodeAttribute::set_on(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::set_off
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ClipPlaneAttribute::
set_off(const PT(PlaneNode) &property) {
  MultiNodeAttribute::set_off(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::is_on
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneAttribute::
is_on(const PT(PlaneNode) &property) const {
  return MultiNodeAttribute::is_on(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttribute::is_off
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneAttribute::
is_off(const PT(PlaneNode) &property) const {
  return MultiNodeAttribute::is_off(property.p());
}
