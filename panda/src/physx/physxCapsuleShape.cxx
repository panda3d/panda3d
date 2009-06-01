// Filename: physxCapsuleShape.cxx
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

#include "physxCapsuleShape.h"

#include "physxCapsule.h"
#include "physxCapsuleShapeDesc.h"

TypeHandle PhysxCapsuleShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : get_height
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxCapsuleShape::
get_height() const {
  nassertr(nCapsuleShape != NULL, -1.0f);

  return nCapsuleShape->getHeight();
}

////////////////////////////////////////////////////////////////////
//     Function : get_radius
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
float PhysxCapsuleShape::
get_radius() const {
  nassertr(nCapsuleShape != NULL, -1.0f);

  return nCapsuleShape->getRadius();
}

////////////////////////////////////////////////////////////////////
//     Function : get_world_capsule
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
get_world_capsule(PhysxCapsule & world_capsule) const {
  nassertv(nCapsuleShape != NULL);

  nCapsuleShape->getWorldCapsule(*(world_capsule.nCapsule));
}

////////////////////////////////////////////////////////////////////
//     Function : save_to_desc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
save_to_desc(PhysxCapsuleShapeDesc & desc) const {
  nassertv(nCapsuleShape != NULL);

  nCapsuleShape->saveToDesc(desc.nCapsuleShapeDesc);
}

////////////////////////////////////////////////////////////////////
//     Function : set_dimensions
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
set_dimensions(float radius, float height) {
  nassertv(nCapsuleShape != NULL);

  nCapsuleShape->setDimensions(radius, height);
}

////////////////////////////////////////////////////////////////////
//     Function : set_height
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
set_height(float height) {
  nassertv(nCapsuleShape != NULL);

  nCapsuleShape->setHeight(height);
}

////////////////////////////////////////////////////////////////////
//     Function : set_radius
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxCapsuleShape::
set_radius(float radius) {
  nassertv(nCapsuleShape != NULL);

  nCapsuleShape->setRadius(radius);
}

#endif // HAVE_PHYSX

