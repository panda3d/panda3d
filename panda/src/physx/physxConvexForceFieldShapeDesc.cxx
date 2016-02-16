/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxConvexForceFieldShapeDesc.cxx
 * @author enn0x
 * @date 2009-11-06
 */

#include "physxConvexForceFieldShapeDesc.h"
#include "physxConvexMesh.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexForceFieldShapeDesc::set_mesh
//       Access: Published
//  Description: Sets the convex mesh for this force field shape.
////////////////////////////////////////////////////////////////////
void PhysxConvexForceFieldShapeDesc::
set_mesh(PhysxConvexMesh *mesh) {

  _desc.meshData = mesh->ptr();
}


