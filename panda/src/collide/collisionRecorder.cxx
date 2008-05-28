// Filename: collisionRecorder.cxx
// Created by:  drose (16Apr03)
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

#include "collisionRecorder.h"
#include "collisionTraverser.h"

#ifdef DO_COLLISION_RECORDING

TypeHandle CollisionRecorder::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionRecorder::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
CollisionRecorder::
CollisionRecorder() {
  _num_missed = 0;
  _num_detected = 0;
  _trav = (CollisionTraverser *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRecorder::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionRecorder::
~CollisionRecorder() {
  if (_trav != (CollisionTraverser *)NULL) {
    _trav->clear_recorder();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRecorder::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CollisionRecorder::
output(ostream &out) const {
  out << "tested " << _num_missed + _num_detected << ", detected "
      << _num_detected << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRecorder::begin_traversal
//       Access: Public, Virtual
//  Description: This method is called at the beginning of a
//               CollisionTraverser::traverse() call.  It is provided
//               as a hook for the derived class to reset its state as
//               appropriate.
////////////////////////////////////////////////////////////////////
void CollisionRecorder::
begin_traversal() {
  _num_missed = 0;
  _num_detected = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRecorder::collision_tested
//       Access: Public, Virtual
//  Description: This method is called when a pair of collision solids
//               have passed all bounding-volume tests and have been
//               tested for a collision.  The detected value is set
//               true if a collision was detected, false otherwise.
////////////////////////////////////////////////////////////////////
void CollisionRecorder::
collision_tested(const CollisionEntry &entry, bool detected) {
  if (detected) {
    _num_detected++;
  } else {
    _num_missed++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRecorder::end_traversal
//       Access: Public, Virtual
//  Description: This method is called at the end of a
//               CollisionTraverser::traverse() call.  It is provided
//               as a hook for the derived class to finalize its state
//               as appropriate.
////////////////////////////////////////////////////////////////////
void CollisionRecorder::
end_traversal() {
}

#endif  // DO_COLLISION_RECORDING
