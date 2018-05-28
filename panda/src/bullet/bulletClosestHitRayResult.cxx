/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletClosestHitRayResult.cxx
 * @author enn0x
 * @date 2010-02-21
 */

#include "bulletClosestHitRayResult.h"

/**
 *
 */
BulletClosestHitRayResult::
BulletClosestHitRayResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask)
 : btCollisionWorld::ClosestRayResultCallback(from_pos, to_pos), _mask(mask) {

  _shapePart = -1;
  _triangleIndex = -1;
}

/**
 * Override default implementation.
 */
bool BulletClosestHitRayResult::
needsCollision(btBroadphaseProxy* proxy0) const {

  btCollisionObject *obj0 = (btCollisionObject *) proxy0->m_clientObject;
  PandaNode *node0 = (PandaNode *) obj0->getUserPointer();
  CollideMask mask0 = node0->get_into_collide_mask();

  return (_mask & mask0) != 0;
}

/**
 * Override default implementation.
 */
btScalar BulletClosestHitRayResult::
addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) {

  // Store partindex information
  if (rayResult.m_localShapeInfo) {
    _shapePart = rayResult.m_localShapeInfo->m_shapePart;
    _triangleIndex = rayResult.m_localShapeInfo->m_triangleIndex;
  }

  // Call the default implementation
  return btCollisionWorld::ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
};

/**
 *
 */
bool BulletClosestHitRayResult::
has_hit() const {

  return hasHit();
}

/**
 *
 */
PN_stdfloat BulletClosestHitRayResult::
get_hit_fraction() const {

  return (PN_stdfloat)m_closestHitFraction;
}

/**
 *
 */
PandaNode *BulletClosestHitRayResult::
get_node() const {

  const btCollisionObject *objectPtr = m_collisionObject;
  return (objectPtr) ? (PandaNode *)objectPtr->getUserPointer() : nullptr;
}

/**
 *
 */
LPoint3 BulletClosestHitRayResult::
get_hit_pos() const {

  return btVector3_to_LPoint3(m_hitPointWorld);
}

/**
 *
 */
LVector3 BulletClosestHitRayResult::
get_hit_normal() const {

  return btVector3_to_LVector3(m_hitNormalWorld);
}

/**
 *
 */
LPoint3 BulletClosestHitRayResult::
get_from_pos() const {

  return btVector3_to_LPoint3(m_rayFromWorld);
}

/**
 *
 */
LPoint3 BulletClosestHitRayResult::
get_to_pos() const {

  return btVector3_to_LPoint3(m_rayToWorld);
}

/**
 *
 */
int BulletClosestHitRayResult::
get_shape_part() const {

  return _shapePart;
}

/**
 *
 */
int BulletClosestHitRayResult::
get_triangle_index() const {

  return _triangleIndex;
}
