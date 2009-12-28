// Filename: physxContactPoint.cxx
// Created by:  enn0x (20Dec09)
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

#include "physxContactPoint.h"
#include "physxManager.h"
#include "physxShape.h"

TypeHandle PhysxContactPoint::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxContactPoint::set
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxContactPoint::
set(NxContactStreamIterator it) {

  _point = it.getPoint();
  _normal = it.getPatchNormal();
  _normal_force = it.getPointNormalForce();
  _separation = it.getSeparation();
  _feature_index0 = it.getFeatureIndex0();
  _feature_index1 = it.getFeatureIndex1();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxContactPoint::empty
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxContactPoint PhysxContactPoint::
empty() {

  return PhysxContactPoint();
}


////////////////////////////////////////////////////////////////////
//     Function: PhysxContactPoint::get_point
//       Access: Published
//  Description: Returns the contact point position.
////////////////////////////////////////////////////////////////////
LPoint3f PhysxContactPoint::
get_point() const {

  return PhysxManager::nxVec3_to_point3(_point);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxContactPoint::get_normal
//       Access: Published
//  Description: Retrieves the patch normal.
////////////////////////////////////////////////////////////////////
LVector3f PhysxContactPoint::
get_normal() const {

  return PhysxManager::nxVec3_to_vec3(_normal);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxContactPoint::get_normal_force
//       Access: Published
//  Description: Retrieves the point normal force.
////////////////////////////////////////////////////////////////////
float PhysxContactPoint::
get_normal_force() const {

  return _normal_force;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxContactPoint::get_separation
//       Access: Published
//  Description: Return the separation for the contact point.
////////////////////////////////////////////////////////////////////
float PhysxContactPoint::
get_separation() const {

  return _separation;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxContactPoint::get_feature_index0
//       Access: Published
//  Description: Retrieves the feature index.
////////////////////////////////////////////////////////////////////
unsigned int PhysxContactPoint::
get_feature_index0() const {

  return _feature_index0;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxContactPoint::get_feature_index1
//       Access: Published
//  Description: Retrieves the feature index.
////////////////////////////////////////////////////////////////////
unsigned int PhysxContactPoint::
get_feature_index1() const {

  return _feature_index1;
}

