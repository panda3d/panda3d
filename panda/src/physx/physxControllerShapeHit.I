/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxControllerShapeHit.I
 * @author enn0x
 * @date 2012-11-28
 */

/**
 *
 */
INLINE PhysxControllerShapeHit::
PhysxControllerShapeHit(const NxControllerShapeHit &hit) :
  _hit(hit) {

}

/**
 *
 */
INLINE PhysxController *PhysxControllerShapeHit::
get_controller() const {

  if (_hit.controller) {
    return (PhysxController *)(_hit.controller->getUserData());
  }
  else {
    return nullptr;
  }
}

/**
 *
 */
INLINE PhysxShape *PhysxControllerShapeHit::
get_shape() const {

  if (_hit.shape) {
    return (PhysxShape *)(_hit.shape->userData);
  }
  else {
    return nullptr;
  }
}

/**
 *
 */
INLINE LPoint3 PhysxControllerShapeHit::
get_world_pos() const {

  return PhysxManager::nxExtVec3_to_point3(_hit.worldPos);
}

/**
 *
 */
INLINE LVector3 PhysxControllerShapeHit::
get_world_normal() const {

  return PhysxManager::nxVec3_to_vec3(_hit.worldNormal);
}

/**
 *
 */
INLINE LVector3 PhysxControllerShapeHit::
get_dir() const {

  return PhysxManager::nxVec3_to_vec3(_hit.dir);
}

/**
 *
 */
INLINE PN_stdfloat PhysxControllerShapeHit::
get_length() const {

  return (PN_stdfloat)_hit.length;
}
