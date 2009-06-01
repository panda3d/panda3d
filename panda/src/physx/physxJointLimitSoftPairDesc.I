// Filename: physxJointLimitSoftPairDesc.I
// Created by:  pratt (Jun 20, 2006)
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
//  Description :
////////////////////////////////////////////////////////////////////
INLINE bool PhysxJointLimitSoftPairDesc::
is_valid() const {
  return nJointLimitSoftPairDesc.isValid();
}

////////////////////////////////////////////////////////////////////
//     Function : set_to_default
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxJointLimitSoftPairDesc::
set_to_default() {
  nJointLimitSoftPairDesc.setToDefault();
}

