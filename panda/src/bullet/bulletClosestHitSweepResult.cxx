/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletClosestHitSweepResult.cxx
 * @author enn0x
 * @date 2010-12-01
 */

#include "bulletClosestHitSweepResult.h"

/**
 *
 */
BulletClosestHitSweepResult::
BulletClosestHitSweepResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask)
 : btCollisionWorld::ClosestConvexResultCallback(from_pos, to_pos), _mask(mask) {

}

/**
 * Override default implementation.
 */
bool BulletClosestHitSweepResult::
needsCollision(btBroadphaseProxy* proxy0) const {

  btCollisionObject *obj0 = (btCollisionObject *) proxy0->m_clientObject;
  PandaNode *node0 = (PandaNode *) obj0->getUserPointer();
  CollideMask mask0 = node0->get_into_collide_mask();

  return (_mask & mask0) != 0;
}

/**
 *
 */
bool BulletClosestHitSweepResult::
has_hit() const {

  return hasHit();
}

/**
 *
 */
PN_stdfloat BulletClosestHitSweepResult::
get_hit_fraction() const {

  return (PN_stdfloat)m_closestHitFraction;
}

/**
 *
 */
PandaNode *BulletClosestHitSweepResult::
get_node() const {

  const btCollisionObject *objectPtr = m_hitCollisionObject;
  return (objectPtr) ? (PandaNode *)objectPtr->getUserPointer() : nullptr;
}

/**
 *
 */
LPoint3 BulletClosestHitSweepResult::
get_hit_pos() const {

  return btVector3_to_LPoint3(m_hitPointWorld);
}

/**
 *
 */
LVector3 BulletClosestHitSweepResult::
get_hit_normal() const {

  return btVector3_to_LVector3(m_hitNormalWorld);
}

/**
 *
 */
LPoint3 BulletClosestHitSweepResult::
get_from_pos() const {

  return btVector3_to_LPoint3(m_convexFromWorld);
}

/**
 *
 */
LPoint3 BulletClosestHitSweepResult::
get_to_pos() const {

  return btVector3_to_LPoint3(m_convexToWorld);
}
