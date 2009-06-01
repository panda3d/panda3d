// Filename: physxActorDesc.cxx
// Created by:  pratt (Apr 7, 2006)
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

#ifdef HAVE_PHYSX

#include "physxActorDesc.h"
#include "physxShapeDesc.h"
#include "physxBodyDesc.h"

////////////////////////////////////////////////////////////////////
//     Function : PhysxActorDesc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxActorDesc::
PhysxActorDesc() {
}

////////////////////////////////////////////////////////////////////
//     Function : add_shape
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorDesc::
add_shape(PhysxShapeDesc pShapeDesc) {
  nActorDesc.shapes.pushBack(pShapeDesc.nShapeDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : get_body
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
const PhysxBodyDesc * PhysxActorDesc::
get_body() const {
  throw "Not Implemented"; // return nActorDesc.body;
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LMatrix4f PhysxActorDesc::
get_global_pose() const {
  return PhysxManager::nxMat34_to_lMatrix4(nActorDesc.globalPose);
}

////////////////////////////////////////////////////////////////////
//     Function : set_body
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorDesc::
set_body(const PhysxBodyDesc * value) {
  nActorDesc.body = &(value->nBodyDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxActorDesc::
set_global_pose(LMatrix4f value) {
  nActorDesc.globalPose = PhysxManager::lMatrix4_to_nxMat34(value);
}

#endif // HAVE_PHYSX

