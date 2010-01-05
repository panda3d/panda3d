// Filename: physxRaycastHit.cxx
// Created by:  enn0x (21Oct09)
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

#include "physxRaycastHit.h"
#include "physxManager.h"
#include "physxShape.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastHit::is_empty
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxRaycastHit::
is_empty() const {

  return (_hit.shape == NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastHit::get_shape
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxShape *PhysxRaycastHit::
get_shape() const {

  nassertr_always(_hit.shape, NULL);
  return (PhysxShape *)_hit.shape->userData;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastHit::get_impact_pos
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3f PhysxRaycastHit::
get_impact_pos() const {

  return PhysxManager::nxVec3_to_point3(_hit.worldImpact);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastHit::get_impact_normal
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
LVector3f PhysxRaycastHit::
get_impact_normal() const {

  return PhysxManager::nxVec3_to_vec3(_hit.worldNormal);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastHit::get_distance
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxRaycastHit::
get_distance() const {

  return _hit.distance;
}

