// Filename: physxBoxControllerDesc.I
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
//     Function: PhysxBoxControllerDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxBoxControllerDesc::
PhysxBoxControllerDesc() : PhysxControllerDesc() {

  _desc.upDirection = (NxHeightFieldAxis)physx_up_axis.get_value();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxControllerDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxBoxControllerDesc::
~PhysxBoxControllerDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxControllerDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxBoxControllerDesc::
set_to_default() {

  _desc.setToDefault();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxControllerDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxBoxControllerDesc::
is_valid() const {

  return _desc.isValid();
}

