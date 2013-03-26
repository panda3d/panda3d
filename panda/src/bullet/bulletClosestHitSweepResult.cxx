// Filename: bulletClosestHitSweepResult.cxx
// Created by:  enn0x (01Dec10)
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

#include "bulletClosestHitSweepResult.h"

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitSweepResult::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
BulletClosestHitSweepResult::
BulletClosestHitSweepResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask) 
 : btCollisionWorld::ClosestConvexResultCallback(from_pos, to_pos), _mask(mask) {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitSweepResult::needsCollision
//       Access: Protected
//  Description: Override default implementation.
////////////////////////////////////////////////////////////////////
bool BulletClosestHitSweepResult::
needsCollision(btBroadphaseProxy* proxy0) const {

  btCollisionObject *obj0 = (btCollisionObject *) proxy0->m_clientObject;
  PandaNode *node0 = (PandaNode *) obj0->getUserPointer();
  CollideMask mask0 = node0->get_into_collide_mask();

  return (_mask & mask0) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitSweepResult::has_hit
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletClosestHitSweepResult::
has_hit() const {

  return hasHit();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitSweepResult::get_hit_fraction
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletClosestHitSweepResult::
get_hit_fraction() const {

  return (PN_stdfloat)m_closestHitFraction;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitSweepResult::get_node
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
const PandaNode *BulletClosestHitSweepResult::
get_node() const {

  const btCollisionObject *objectPtr = m_hitCollisionObject;
  return (objectPtr) ? (const PandaNode *)objectPtr->getUserPointer() : NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitSweepResult::get_hit_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletClosestHitSweepResult::
get_hit_pos() const {

  return btVector3_to_LPoint3(m_hitPointWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitSweepResult::get_hit_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVector3 BulletClosestHitSweepResult::
get_hit_normal() const {

  return btVector3_to_LVector3(m_hitNormalWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitSweepResult::get_from_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletClosestHitSweepResult::
get_from_pos() const {

  return btVector3_to_LPoint3(m_convexFromWorld);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletClosestHitSweepResult::get_to_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletClosestHitSweepResult::
get_to_pos() const {

  return btVector3_to_LPoint3(m_convexToWorld);
}

