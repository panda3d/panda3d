// Filename: clipPlaneTransition.I
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
//     Function: ClipPlaneTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ClipPlaneTransition::
ClipPlaneTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::all_off
//       Access: Public
//  Description: This named constructor returns a ClipPlaneTransition
//               that's preconfigured to turn off all planes below it.
////////////////////////////////////////////////////////////////////
INLINE ClipPlaneTransition ClipPlaneTransition::
all_off() {
  ClipPlaneTransition t;
  t.set_default_dir(TD_off);
  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::set_identity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ClipPlaneTransition::
set_identity(const PT(PlaneNode) &property) {
  MultiNodeTransition::set_identity(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::set_on
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ClipPlaneTransition::
set_on(const PT(PlaneNode) &property) {
  MultiNodeTransition::set_on(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::set_off
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ClipPlaneTransition::
set_off(const PT(PlaneNode) &property) {
  MultiNodeTransition::set_off(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::is_identity
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneTransition::
is_identity(const PT(PlaneNode) &property) const {
  return MultiNodeTransition::is_identity(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::is_on
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneTransition::
is_on(const PT(PlaneNode) &property) const {
  return MultiNodeTransition::is_on(property.p());
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::is_off
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool ClipPlaneTransition::
is_off(const PT(PlaneNode) &property) const {
  return MultiNodeTransition::is_off(property.p());
}
