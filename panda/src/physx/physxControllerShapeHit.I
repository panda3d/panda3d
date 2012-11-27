// Filename: physxControllerShapeHit.I
// Created by:  enn0x (28Nov12)
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
//     Function: PhysxControllerShapeHit::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxControllerShapeHit::
PhysxControllerShapeHit(const NxControllerShapeHit &hit) :
  _hit(hit) {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerShapeHit::get_controller
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxController *PhysxControllerShapeHit::
get_controller() const {

  if (_hit.controller) {
    return (PhysxController *)(_hit.controller->getUserData());
  }
  else {
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerShapeHit::get_shape
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxShape *PhysxControllerShapeHit::
get_shape() const {

  if (_hit.shape) {
    return (PhysxShape *)(_hit.shape->userData);
  }
  else {
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerShapeHit::get_world_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LPoint3 PhysxControllerShapeHit::
get_world_pos() const {

  return PhysxManager::nxExtVec3_to_point3(_hit.worldPos);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerShapeHit::get_world_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3 PhysxControllerShapeHit::
get_world_normal() const {

  return PhysxManager::nxVec3_to_vec3(_hit.worldNormal);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerShapeHit::get_dir
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LVector3 PhysxControllerShapeHit::
get_dir() const {

  return PhysxManager::nxVec3_to_vec3(_hit.dir);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxControllerShapeHit::get_length
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PhysxControllerShapeHit::
get_length() const {

  return (PN_stdfloat)_hit.length;
}

