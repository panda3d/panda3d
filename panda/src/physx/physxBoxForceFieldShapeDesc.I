// Filename: physxBoxForceFieldShapeDesc.I
// Created by:  enn0x (06Nov09)
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
//     Function: PhysxBoxForceFieldShapeDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxBoxForceFieldShapeDesc::
PhysxBoxForceFieldShapeDesc() : PhysxForceFieldShapeDesc() {

  set_to_default();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxForceFieldShapeDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxBoxForceFieldShapeDesc::
~PhysxBoxForceFieldShapeDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxForceFieldShapeDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxBoxForceFieldShapeDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxForceFieldShapeDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxBoxForceFieldShapeDesc::
is_valid() const {

  return _desc.isValid();
}

