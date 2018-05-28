/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxRaycastHit.cxx
 * @author enn0x
 * @date 2009-10-21
 */

#include "physxRaycastHit.h"
#include "physxManager.h"
#include "physxShape.h"

/**
 *
 */
bool PhysxRaycastHit::
is_empty() const {

  return (_hit.shape == nullptr);
}

/**
 *
 */
PhysxShape *PhysxRaycastHit::
get_shape() const {

  nassertr_always(_hit.shape, nullptr);
  return (PhysxShape *)_hit.shape->userData;
}

/**
 *
 */
LPoint3f PhysxRaycastHit::
get_impact_pos() const {

  return PhysxManager::nxVec3_to_point3(_hit.worldImpact);
}

/**
 *
 */
LVector3f PhysxRaycastHit::
get_impact_normal() const {

  return PhysxManager::nxVec3_to_vec3(_hit.worldNormal);
}

/**
 *
 */
float PhysxRaycastHit::
get_distance() const {

  return _hit.distance;
}
