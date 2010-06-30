// Filename: physxCapsuleShapeDesc.I
// Created by:  enn0x (11Sep09)
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
//     Function: PhysxCapsuleShapeDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCapsuleShapeDesc::
PhysxCapsuleShapeDesc() : PhysxShapeDesc() {

  set_to_default();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleShapeDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxCapsuleShapeDesc::
~PhysxCapsuleShapeDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleShapeDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxCapsuleShapeDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxCapsuleShapeDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxCapsuleShapeDesc::
is_valid() const {

  return _desc.isValid();
}

