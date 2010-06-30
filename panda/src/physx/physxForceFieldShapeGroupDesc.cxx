// Filename: physxForceFieldShapeGroupDesc.cxx
// Created by:  enn0x (11Nov09)
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

#include "physxForceFieldShapeGroupDesc.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroupDesc::add_shape
//       Access: Published
//  Description: Adds a shape to the list of force field shapes
//               composing this shape group.
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShapeGroupDesc::
add_shape(PhysxForceFieldShapeDesc &desc) {

  _desc.shapes.push_back(desc.ptr());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroupDesc::set_name
//       Access: Published
//  Description: Sets the optional debug name for the force field
//               shape group.
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShapeGroupDesc::
set_name(const char *name) {

  _name = name ? name : "";
  _desc.name = _name.c_str();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroupDesc::get_name
//       Access: Published
//  Description: Returns the optional debug name for this force
//               field shape group.
////////////////////////////////////////////////////////////////////
const char *PhysxForceFieldShapeGroupDesc::
get_name() const {

  return _desc.name;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroupDesc::set_flag
//       Access: Published
//  Description: Raise or lower individual force field shape group
//               flags.
////////////////////////////////////////////////////////////////////
void PhysxForceFieldShapeGroupDesc::
set_flag(const PhysxForceFieldShapeGroupFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxForceFieldShapeGroupDesc::get_flag
//       Access: Published
//  Description: Returns the specified force field shape group
//               flag.
////////////////////////////////////////////////////////////////////
bool PhysxForceFieldShapeGroupDesc::
get_flag(const PhysxForceFieldShapeGroupFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

