// Filename: collisionTraverser.I
// Created by:  drose (16Mar02)
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


#ifdef DO_COLLISION_RECORDING

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::has_recorder
//       Access: Public
//  Description: Returns true if the CollisionTraverser has a
//               CollisionRecorder object currently assigned, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionTraverser::
has_recorder() const {
  return _recorder != (CollisionRecorder *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::get_recorder
//       Access: Public
//  Description: Returns the CollisionRecorder currently assigned, or
//               NULL if no recorder is assigned.
////////////////////////////////////////////////////////////////////
INLINE CollisionRecorder *CollisionTraverser::
get_recorder() const {
  return _recorder;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::clear_recorder
//       Access: Public
//  Description: Removes the CollisionRecorder from the traverser and
//               restores normal low-overhead operation.
////////////////////////////////////////////////////////////////////
INLINE void CollisionTraverser::
clear_recorder() {
  set_recorder((CollisionRecorder *)NULL);
}

#endif  // DO_COLLISION_RECORDING
