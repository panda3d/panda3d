/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletAllHitsRayResult.cxx
 * @author enn0x
 * @date 2010-02-21
 */

#include "bulletAllHitsRayResult.h"

/**
 *
 */
BulletAllHitsRayResult::
BulletAllHitsRayResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask)
 : btCollisionWorld::AllHitsRayResultCallback(from_pos, to_pos), _mask(mask) {

}

/**
 * Override default implementation.
 */
bool BulletAllHitsRayResult::
needsCollision(btBroadphaseProxy* proxy0) const {

  btCollisionObject *obj0 = (btCollisionObject *) proxy0->m_clientObject;
  PandaNode *node0 = (PandaNode *) obj0->getUserPointer();
  CollideMask mask0 = node0->get_into_collide_mask();

  return (_mask & mask0) != 0;
}

/**
 * Override default implementation.
 */
btScalar BulletAllHitsRayResult::
addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) {

  // Store partindex information
  if (rayResult.m_localShapeInfo) {
    _shapePart.push_back(rayResult.m_localShapeInfo->m_shapePart);
    _triangleIndex.push_back(rayResult.m_localShapeInfo->m_triangleIndex);
  }
  else {
    _shapePart.push_back(-1);
    _triangleIndex.push_back(-1);
  }

  // Call the default implementation
  return btCollisionWorld::AllHitsRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
};

/**
 *
 */
LPoint3 BulletAllHitsRayResult::
get_from_pos() const {

  return btVector3_to_LPoint3(m_rayFromWorld);
}

/**
 *
 */
LPoint3 BulletAllHitsRayResult::
get_to_pos() const {

  return btVector3_to_LPoint3(m_rayToWorld);
}

/**
 *
 */
bool BulletAllHitsRayResult::
has_hits() const {

  return hasHit();
}

/**
 *
 */
PN_stdfloat BulletAllHitsRayResult::
get_closest_hit_fraction() const {

  return (PN_stdfloat)m_closestHitFraction;
}

/**
 *
 */
int BulletAllHitsRayResult::
get_num_hits() const {

  return m_collisionObjects.size();
}

/**
 *
 */
const BulletRayHit BulletAllHitsRayResult::
get_hit(int idx) const {

  nassertr(idx >= 0 && idx < get_num_hits(), BulletRayHit::empty());

  BulletRayHit hit;

  hit._object = m_collisionObjects[idx];
  hit._normal = m_hitNormalWorld[idx];
  hit._pos = m_hitPointWorld[idx];
  hit._fraction = m_hitFractions[idx];

  hit._shapePart = _shapePart[idx];
  hit._triangleIndex = _triangleIndex[idx];

  return hit;
}

/**
 *
 */
PN_stdfloat BulletRayHit::
get_hit_fraction() const {

  return (PN_stdfloat)_fraction;
}

/**
 *
 */
PandaNode *BulletRayHit::
get_node() const {

  return (_object) ? (PandaNode *)_object->getUserPointer() : nullptr;
}

/**
 *
 */
LPoint3 BulletRayHit::
get_hit_pos() const {

  return btVector3_to_LPoint3(_pos);
}

/**
 *
 */
LVector3 BulletRayHit::
get_hit_normal() const {

  return btVector3_to_LVector3(_normal);
}

/**
 *
 */
int BulletRayHit::
get_shape_part() const {

  return _shapePart;
}

/**
 *
 */
int BulletRayHit::
get_triangle_index() const {

  return _triangleIndex;
}
