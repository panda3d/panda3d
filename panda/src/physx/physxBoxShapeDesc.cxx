// Filename: physxBoxShapeDesc.cxx
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

#include "physxBoxShapeDesc.h"

#include "luse.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxBoxShapeDesc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxBoxShapeDesc::
PhysxBoxShapeDesc()
: PhysxShapeDesc( &nBoxShapeDesc ) {

}

////////////////////////////////////////////////////////////////////
//     Function : get_dimensions
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxBoxShapeDesc::
get_dimensions() const {
  return PhysxManager::nxVec3_to_lVecBase3(nBoxShapeDesc.dimensions);
}

////////////////////////////////////////////////////////////////////
//     Function : set_dimensions
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBoxShapeDesc::
set_dimensions(LVecBase3f value) {
  nBoxShapeDesc.dimensions = PhysxManager::lVecBase3_to_nxVec3(value);
}

#endif // HAVE_PHYSX

