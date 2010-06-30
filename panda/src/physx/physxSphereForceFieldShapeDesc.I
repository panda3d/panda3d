// Filename: physxSphereForceFieldShapeDesc.I
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
//     Function: PhysxSphereForceFieldShapeDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSphereForceFieldShapeDesc::
PhysxSphereForceFieldShapeDesc() : PhysxForceFieldShapeDesc() {

  set_to_default();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphereForceFieldShapeDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxSphereForceFieldShapeDesc::
~PhysxSphereForceFieldShapeDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphereForceFieldShapeDesc::set_to_default
//       Access: Published
//  Description: (re)sets the structure to the default.
////////////////////////////////////////////////////////////////////
INLINE void PhysxSphereForceFieldShapeDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSphereForceFieldShapeDesc::is_valid
//       Access: Published
//  Description: Returns true if the descriptor is valid.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxSphereForceFieldShapeDesc::
is_valid() const {

  return _desc.isValid();
}

