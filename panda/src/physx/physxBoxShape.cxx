// Filename: physxBoxShape.cxx
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

#include "physxBoxShape.h"

#include "luse.h"
#include "physxBox.h"
#include "physxBoxShapeDesc.h"

TypeHandle PhysxBoxShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : get_dimensions
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxBoxShape::
get_dimensions() const {
  nassertr(nBoxShape != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nBoxShape->getDimensions());
}

////////////////////////////////////////////////////////////////////
//     Function : get_world_obb
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBoxShape::
get_world_obb(PhysxBox & obb) const {
  nassertv(nBoxShape != NULL);

  nBoxShape->getWorldOBB(*(obb.nBox));
}

////////////////////////////////////////////////////////////////////
//     Function : save_to_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBoxShape::
save_to_desc(PhysxBoxShapeDesc & desc) const {
  nassertv(nBoxShape != NULL);

  nBoxShape->saveToDesc(desc.nBoxShapeDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : set_dimensions
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxBoxShape::
set_dimensions(const LVecBase3f & vec) {
  nassertv(nBoxShape != NULL);

  nBoxShape->setDimensions(PhysxManager::lVecBase3_to_nxVec3(vec));
}

#endif // HAVE_PHYSX

