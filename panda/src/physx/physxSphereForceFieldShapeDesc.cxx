/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSphereForceFieldShapeDesc.cxx
 * @author enn0x
 * @date 2009-11-06
 */

#include "physxSphereForceFieldShapeDesc.h"

/**
 * Radius of shape.  Must be positive.
 */
void PhysxSphereForceFieldShapeDesc::
set_radius(float radius) {

  _desc.radius = radius;
}

/**
 * Radius of shape.
 */
float PhysxSphereForceFieldShapeDesc::
get_radius() const {

  return _desc.radius;
}
