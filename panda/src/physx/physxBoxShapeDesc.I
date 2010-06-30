// Filename: physxBoxShapeDesc.I
// Created by:  enn0x (08Sep09)
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
//     Function: PhysxBoxShapeDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxBoxShapeDesc::
PhysxBoxShapeDesc() : PhysxShapeDesc() {

  set_to_default();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxShapeDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxBoxShapeDesc::
~PhysxBoxShapeDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxShapeDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxBoxShapeDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBoxShapeDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxBoxShapeDesc::
is_valid() const {

  return _desc.isValid();
}

