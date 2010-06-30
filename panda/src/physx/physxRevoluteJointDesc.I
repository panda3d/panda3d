// Filename: physxRevoluteJointDesc.I
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
//     Function: PhysxRevoluteJointDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxRevoluteJointDesc::
PhysxRevoluteJointDesc() : PhysxJointDesc() {

  set_to_default();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRevoluteJointDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxRevoluteJointDesc::
~PhysxRevoluteJointDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRevoluteJointDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxRevoluteJointDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRevoluteJointDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxRevoluteJointDesc::
is_valid() const {

  return _desc.isValid();
}

