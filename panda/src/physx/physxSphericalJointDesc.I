// Filename: physxSphericalJointDesc.I
// Created by:  enn0x (28Sep09)
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
//     Function: PhysxSphericalJointDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSphericalJointDesc::
PhysxSphericalJointDesc() : PhysxJointDesc() {

  set_to_default();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphericalJointDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSphericalJointDesc::
~PhysxSphericalJointDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphericalJointDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxSphericalJointDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphericalJointDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxSphericalJointDesc::
is_valid() const {

  return _desc.isValid();
}

