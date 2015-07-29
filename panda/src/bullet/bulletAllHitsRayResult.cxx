// Filename: bulletAllHitsRayResult.cxx
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

#include "bulletAllHitsRayResult.h"

////////////////////////////////////////////////////////////////////
//     Function: BulletAllHitsRayResult::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
BulletAllHitsRayResult::
BulletAllHitsRayResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask) 
 : btCollisionWorld::AllHitsRayResultCallback(from_pos, to_pos), _mask(mask) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletAllHitsRayResult::needsCollision
//       Access: Protected
//  Description: Override default implementation.
////////////////////////////////////////////////////////////////////
bool BulletAllHitsRayResult::
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
btScalar BulletAllHitsRayResult::
addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) {

  // Store part/index information
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

////////////////////////////////////////////////////////////////////
//     Function: BulletAllHitsRayResult::get_from_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletAllHitsRayResult::
get_from_pos() const {

  return btVector3_to_LPoint3(m_rayFromWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletAllHitsRayResult::get_to_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletAllHitsRayResult::
get_to_pos() const {

  return btVector3_to_LPoint3(m_rayToWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletAllHitsRayResult::has_hits
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletAllHitsRayResult::
has_hits() const {

  return hasHit();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletAllHitsRayResult::get_closest_hit_fraction
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletAllHitsRayResult::
get_closest_hit_fraction() const {

  return (PN_stdfloat)m_closestHitFraction;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletAllHitsRayResult::get_num_hits
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletAllHitsRayResult::
get_num_hits() const {

  return m_collisionObjects.size();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletAllHitsRayResult::get_hit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: BulletRayHit::get_hit_fraction
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletRayHit::
get_hit_fraction() const {

  return (PN_stdfloat)_fraction;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRayHit::get_node
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
const PandaNode *BulletRayHit::
get_node() const {

  return (_object) ? (PandaNode *)_object->getUserPointer() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRayHit::get_hit_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletRayHit::
get_hit_pos() const {

  return btVector3_to_LPoint3(_pos);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRayHit::get_hit_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3 BulletRayHit::
get_hit_normal() const {

  return btVector3_to_LVector3(_normal);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRayHit::get_shape_part
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletRayHit::
get_shape_part() const {

  return _shapePart;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletRayHit::get_triangle_index
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletRayHit::
get_triangle_index() const {

  return _triangleIndex;
}

