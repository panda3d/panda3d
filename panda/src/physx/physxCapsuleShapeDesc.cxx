/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCapsuleShapeDesc.cxx
 * @author enn0x
 * @date 2009-09-11
 */

#include "physxCapsuleShapeDesc.h"

/**
 * Sets the radius of the capsule's hemispherical ends and its trunk.
 */
void PhysxCapsuleShapeDesc::
set_radius(float radius) {

  _desc.radius = radius;
}

/**
 * Sets the distance between the two hemispherical ends of the capsule.
 */
void PhysxCapsuleShapeDesc::
set_height(float height) {

  _desc.height = height;
}

/**
 * The radius of the capsule's hemispherical ends and its trunk.
 */
float PhysxCapsuleShapeDesc::
get_radius() const {

  return _desc.radius;
}

/**
 * The distance between the two hemispherical ends of the capsule.
 */
float PhysxCapsuleShapeDesc::
get_height() const {

  return _desc.height;
}
