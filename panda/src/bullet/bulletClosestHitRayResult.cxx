// Filename: bulletClosestHitRayResult.cxx
// Created by:  enn0x (21Feb10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "bulletClosestHitRayResult.h"

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
BulletClosestHitRayResult::
BulletClosestHitRayResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask) 
 : btCollisionWorld::ClosestRayResultCallback(from_pos, to_pos), _mask(mask) {

  _shapePart = -1;
  _triangleIndex = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::needsCollision
//       Access: Protected
//  Description: Override default implementation.
////////////////////////////////////////////////////////////////////
bool BulletClosestHitRayResult::
needsCollision(btBroadphaseProxy* proxy0) const {

  btCollisionObject *obj0 = (btCollisionObject *) proxy0->m_clientObject;
  PandaNode *node0 = (PandaNode *) obj0->getUserPointer();
  CollideMask mask0 = node0->get_into_collide_mask();

  return (_mask & mask0) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletAllHitsRayResult::addSingleResult
//       Access: Protected
//  Description: Override default implementation.
////////////////////////////////////////////////////////////////////
btScalar BulletClosestHitRayResult::
addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) {

  // Store part/index information
  if (rayResult.m_localShapeInfo) {
    _shapePart = rayResult.m_localShapeInfo->m_shapePart;
    _triangleIndex = rayResult.m_localShapeInfo->m_triangleIndex;
  }

  // Call the default implementation
  return btCollisionWorld::ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
};

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::has_hit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletClosestHitRayResult::
has_hit() const {

  return hasHit();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::get_hit_fraction
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletClosestHitRayResult::
get_hit_fraction() const {

  return (PN_stdfloat)m_closestHitFraction;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::get_node
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
const PandaNode *BulletClosestHitRayResult::
get_node() const {

  const btCollisionObject *objectPtr = m_collisionObject;
  return (objectPtr) ? (const PandaNode *)objectPtr->getUserPointer() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::get_hit_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletClosestHitRayResult::
get_hit_pos() const {

  return btVector3_to_LPoint3(m_hitPointWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::get_hit_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3 BulletClosestHitRayResult::
get_hit_normal() const {

  return btVector3_to_LVector3(m_hitNormalWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::get_from_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletClosestHitRayResult::
get_from_pos() const {

  return btVector3_to_LPoint3(m_rayFromWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::get_to_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletClosestHitRayResult::
get_to_pos() const {

  return btVector3_to_LPoint3(m_rayToWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::get_shape_part
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletClosestHitRayResult::
get_shape_part() const {

  return _shapePart;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitRayResult::get_triangle_index
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletClosestHitRayResult::
get_triangle_index() const {

  return _triangleIndex;
}

