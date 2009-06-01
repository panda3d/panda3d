// Filename: physxSphereShape.cxx
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

#include "physxSphereShape.h"

#include "physxSphere.h"
#include "physxSphereShapeDesc.h"

TypeHandle PhysxSphereShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : get_radius
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxSphereShape::
get_radius() const {
  nassertr(nSphereShape != NULL, -1.0f);

  return nSphereShape->getRadius();
}

////////////////////////////////////////////////////////////////////
//     Function : get_world_sphere
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSphereShape::
get_world_sphere(PhysxSphere & world_sphere) const {
  nassertv(nSphereShape != NULL);

  nSphereShape->getWorldSphere(*(world_sphere.nSphere));
}

////////////////////////////////////////////////////////////////////
//     Function : save_to_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSphereShape::
save_to_desc(PhysxSphereShapeDesc & desc) const {
  nassertv(nSphereShape != NULL);

  nSphereShape->saveToDesc(desc.nSphereShapeDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : set_radius
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSphereShape::
set_radius(float radius) {
  nassertv(nSphereShape != NULL);

  nSphereShape->setRadius(radius);
}

#endif // HAVE_PHYSX



