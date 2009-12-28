// Filename: physxWheelShapeDesc.I
// Created by:  enn0x (09Nov09)
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
//     Function: PhysxWheelShapeDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxWheelShapeDesc::
PhysxWheelShapeDesc() : PhysxShapeDesc() {

  _desc.name = "";
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxWheelShapeDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxWheelShapeDesc::
~PhysxWheelShapeDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxWheelShapeDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxWheelShapeDesc::
set_to_default() {

  _desc.setToDefault();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxWheelShapeDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxWheelShapeDesc::
is_valid() const {

  return _desc.isValid();
}

