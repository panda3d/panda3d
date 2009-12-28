// Filename: physxCapsuleControllerDesc.I
// Created by:  enn0x (22Sep09)
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
//     Function: PhysxCapsuleControllerDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCapsuleControllerDesc::
PhysxCapsuleControllerDesc() : PhysxControllerDesc() {

  _desc.upDirection = (NxHeightFieldAxis)physx_up_axis.get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleControllerDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCapsuleControllerDesc::
~PhysxCapsuleControllerDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleControllerDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxCapsuleControllerDesc::
set_to_default() {

  _desc.setToDefault();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleControllerDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxCapsuleControllerDesc::
is_valid() const {

  return _desc.isValid();
}

