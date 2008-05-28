// Filename: collisionTraverser.I
// Created by:  drose (16Mar02)
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
//     Function: CollisionTraverser::set_respect_prev_transform
//       Access: Published
//  Description: Sets the flag that indicates whether the
//               prev_transform stored on a node (as updated via
//               set_fluid_pos(), etc.) is respected to calculate
//               collisions.  If this is true, certain types of
//               collision tests will be enhanced by the information
//               about objects in motion.  If this is false, objects
//               are always considered to be static.  The default is
//               false.
////////////////////////////////////////////////////////////////////
INLINE void CollisionTraverser::
set_respect_prev_transform(bool flag) {
  _respect_prev_transform = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::get_respect_prev_transform
//       Access: Published
//  Description: Returns the flag that indicates whether the
//               prev_transform stored on a node is respected to
//               calculate collisions.  See
//               set_respect_prev_transform().
////////////////////////////////////////////////////////////////////
INLINE bool CollisionTraverser::
get_respect_prev_transform() const {
  return _respect_prev_transform;
}

#ifdef DO_COLLISION_RECORDING

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::has_recorder
//       Access: Published
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
//       Access: Published
//  Description: Returns the CollisionRecorder currently assigned, or
//               NULL if no recorder is assigned.
////////////////////////////////////////////////////////////////////
INLINE CollisionRecorder *CollisionTraverser::
get_recorder() const {
  return _recorder;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::clear_recorder
//       Access: Published
//  Description: Removes the CollisionRecorder from the traverser and
//               restores normal low-overhead operation.
////////////////////////////////////////////////////////////////////
INLINE void CollisionTraverser::
clear_recorder() {
  set_recorder((CollisionRecorder *)NULL);
}

#endif  // DO_COLLISION_RECORDING
