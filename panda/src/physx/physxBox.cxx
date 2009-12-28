// Filename: physxBox.cxx
// Created by:  enn0x (31Oct09)
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

#include "physxBox.h"
#include "physxManager.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PhysxBox::
PhysxBox(const LPoint3f &center, const LVector3f &extents, const LMatrix3f &rot) {

  _box = NxBox(PhysxManager::point3_to_nxVec3(center),
               PhysxManager::vec3_to_nxVec3(extents),
               PhysxManager::mat3_to_nxMat33(rot));
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::is_valid
//       Access: Published
//  Description: Returns TRUE if the box is valid.
////////////////////////////////////////////////////////////////////
bool PhysxBox::
is_valid() const {

  return _box.isValid();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::rotate
//       Access: Published
//  Description: Recomputes the box after an arbitrary transform by
//               a 4x4 matrix.
////////////////////////////////////////////////////////////////////
void PhysxBox::
rotate(const LMatrix4f &m, PhysxBox &obb) const {

  nassertv(!m.is_nan());

  _box.rotate(PhysxManager::mat4_to_nxMat34(m), obb._box);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::set_empty
//       Access: Published
//  Description: Setups an empty box.
////////////////////////////////////////////////////////////////////
void PhysxBox::
set_empty() {

  _box.setEmpty();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::get_center
//       Access: Published
//  Description: Return center of the box.
////////////////////////////////////////////////////////////////////
LPoint3f PhysxBox::
get_center() const {

  return PhysxManager::nxVec3_to_point3(_box.GetCenter());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::get_extents
//       Access: Published
//  Description: Returns the extents (radii) of the box.
////////////////////////////////////////////////////////////////////
LVector3f PhysxBox::
get_extents() const {

  return PhysxManager::nxVec3_to_vec3(_box.GetExtents());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::get_rot
//       Access: Published
//  Description: Return the rotation of the box.
////////////////////////////////////////////////////////////////////
LMatrix3f PhysxBox::
get_rot() const {

  return PhysxManager::nxMat33_to_mat3(_box.GetRot());
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::set_center
//       Access: Published
//  Description: Sets the center of the box.
////////////////////////////////////////////////////////////////////
void PhysxBox::
set_center(LPoint3f center) {

  nassertv(!center.is_nan());

  _box.center = PhysxManager::vec3_to_nxVec3(center);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::set_extents
//       Access: Published
//  Description: Sets the extents of the box.
////////////////////////////////////////////////////////////////////
void PhysxBox::
set_extents(LVector3f extents) {

  nassertv(!extents.is_nan());

  _box.extents = PhysxManager::vec3_to_nxVec3(extents);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxBox::set_rot
//       Access: Published
//  Description: Sets the rotation of the box.
////////////////////////////////////////////////////////////////////
void PhysxBox::
set_rot(LMatrix3f rot) {

  nassertv(!rot.is_nan());

  _box.rot = PhysxManager::mat3_to_nxMat33(rot);
}

