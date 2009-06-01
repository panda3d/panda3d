// Filename: physxJoint.I
// Created by:  pratt (Jun 16, 2006)
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
//     Function : is_valid
//       Access : Published
//  Description : Returns true if the NxJoint associated with this
//                Joint is not NULL.  The NxJoint will be NULL if
//                the Joint has been released.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxJoint::
is_valid() {
  return nJoint != NULL;
}

////////////////////////////////////////////////////////////////////
//     Function : get_app_data
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void * PhysxJoint::
get_app_data() const {
  nassertr(nJoint != NULL, NULL);

  return nJoint->appData;
}

////////////////////////////////////////////////////////////////////
//     Function : set_app_data
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxJoint::
set_app_data(void * value) {
  nassertv(nJoint != NULL);

  nJoint->appData = value;
}

