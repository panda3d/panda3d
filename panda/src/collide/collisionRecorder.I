// Filename: collisionRecorder.I
// Created by:  drose (17Apr03)
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
//     Function: CollisionRecorder::get_type_index
//       Access: Published
//  Description: Duplicates the functionality of
//               TypedObject::get_type_index().
////////////////////////////////////////////////////////////////////
INLINE int CollisionRecorder::
get_type_index() const {
  return get_type().get_index();
}

