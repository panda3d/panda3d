/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxRay.cxx
 * @author enn0x
 * @date 2009-10-21
 */

#include "physxRay.h"
#include "physxManager.h"

/**
 * Sets the ray origin.
 */
void PhysxRay::
set_origin(const LPoint3f &origin) {

  nassertv_always(!origin.is_nan());
  _ray.orig = PhysxManager::point3_to_nxVec3(origin);
}

/**
 * Returns the ray origin
 */
LPoint3f PhysxRay::
get_origin() const {

  return PhysxManager::nxVec3_to_point3(_ray.orig);
}

/**
 * Set the ray direction.  It is not required to pass a normalized vector.
 */
void PhysxRay::
set_direction(const LVector3f &direction) {

  nassertv_always(!direction.is_nan());

  _ray.dir = PhysxManager::vec3_to_nxVec3(direction);
  _ray.dir.normalize();
}

/**
 * Returns the ray direction.
 */
LVector3f PhysxRay::
get_direction() const {

  return PhysxManager::nxVec3_to_vec3(_ray.dir);
}

/**
 * Sets the ray length.  If no length is set then the ray will be virtually
 * infinite (the maximum floating point number will be used, e.g.
 * 3.40282346639e+038).
 */
void PhysxRay::
set_length(float length) {

  nassertv_always(length > 0.0f);
  _length = length;
}

/**
 * Returns the ray length.
 */
float PhysxRay::
get_length() const {

  return _length;
}
