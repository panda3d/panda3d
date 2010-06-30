// Filename: physxPointInPlaneJointDesc.I
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
//     Function: PhysxPointInPlaneJointDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxPointInPlaneJointDesc::
PhysxPointInPlaneJointDesc() : PhysxJointDesc() {

  set_to_default();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPointInPlaneJointDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxPointInPlaneJointDesc::
~PhysxPointInPlaneJointDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPointInPlaneJointDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxPointInPlaneJointDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxPointInPlaneJointDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxPointInPlaneJointDesc::
is_valid() const {

  return _desc.isValid();
}

