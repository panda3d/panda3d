// Filename: physxPlaneShape.cxx
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

#include "physxPlaneShape.h"

#include "luse.h"
#include "physxPlane.h"
#include "physxPlaneShapeDesc.h"

TypeHandle PhysxPlaneShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : get_plane
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxPlane PhysxPlaneShape::
get_plane() const {
//  return *((PhysxPlane *)(nPlaneShape->getPlane().userData));
  throw "Not Implemented";
}

////////////////////////////////////////////////////////////////////
//     Function : save_to_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxPlaneShape::
save_to_desc(PhysxPlaneShapeDesc & desc) const {
  nassertv(nPlaneShape != NULL);

  nPlaneShape->saveToDesc(desc.nPlaneShapeDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : set_plane
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxPlaneShape::
set_plane(const LVecBase3f & normal, float d) {
  nassertv(nPlaneShape != NULL);

  nPlaneShape->setPlane(PhysxManager::lVecBase3_to_nxVec3(normal), d);
}

#endif // HAVE_PHYSX

