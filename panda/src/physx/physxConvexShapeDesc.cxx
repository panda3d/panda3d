/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxConvexShapeDesc.cxx
 * @author enn0x
 * @date 2009-10-14
 */

#include "physxConvexShapeDesc.h"
#include "physxConvexMesh.h"

/**

 */
void PhysxConvexShapeDesc::
set_mesh(PhysxConvexMesh *mesh) {

  _desc.meshData = mesh->ptr();
}
