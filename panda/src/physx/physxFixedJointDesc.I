// Filename: physxFixedJointDesc.I
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
//     Function: PhysxFixedJointDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxFixedJointDesc::
PhysxFixedJointDesc() : PhysxJointDesc() {

  set_to_default();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFixedJointDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxFixedJointDesc::
~PhysxFixedJointDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFixedJointDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxFixedJointDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxFixedJointDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxFixedJointDesc::
is_valid() const {

  return _desc.isValid();
}

