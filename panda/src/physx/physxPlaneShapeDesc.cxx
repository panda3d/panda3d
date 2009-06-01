// Filename: physxPlaneShapeDesc.cxx
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

#include "physxPlaneShapeDesc.h"

#include "luse.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxPlaneShapeDesc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxPlaneShapeDesc::
PhysxPlaneShapeDesc()
: PhysxShapeDesc( &nPlaneShapeDesc ) {

}

////////////////////////////////////////////////////////////////////
//     Function : get_normal
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxPlaneShapeDesc::
get_normal() const {
  return PhysxManager::nxVec3_to_lVecBase3(nPlaneShapeDesc.normal);
}

////////////////////////////////////////////////////////////////////
//     Function : set_normal
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxPlaneShapeDesc::
set_normal(LVecBase3f value) {
  nPlaneShapeDesc.normal = PhysxManager::lVecBase3_to_nxVec3(value);
}

#endif // HAVE_PHYSX

