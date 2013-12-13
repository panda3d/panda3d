// Filename: bulletClosestHitRayResult.h
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

#ifndef __BULLET_CLOSEST_HIT_RAY_RESULT_H__
#define __BULLET_CLOSEST_HIT_RAY_RESULT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"
#include "pandaNode.h"
#include "collideMask.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletClosestHitRayResult
// Description : 
////////////////////////////////////////////////////////////////////
struct EXPCL_PANDABULLET BulletClosestHitRayResult : public btCollisionWorld::ClosestRayResultCallback {

PUBLISHED:
  INLINE static BulletClosestHitRayResult empty();

  LPoint3 get_from_pos() const;
  LPoint3 get_to_pos() const;

  bool has_hit() const;

  const PandaNode *get_node() const;
  LPoint3 get_hit_pos() const;
  LVector3 get_hit_normal() const;
  PN_stdfloat get_hit_fraction() const;

  int get_shape_part() const;
  int get_triangle_index() const;

public:
  virtual bool needsCollision(btBroadphaseProxy* proxy0) const;
  virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace);

private:
  BulletClosestHitRayResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask);

  CollideMask _mask;

  int _shapePart;
  int _triangleIndex;

  friend class BulletWorld;
};

#include "bulletClosestHitRayResult.I"

#endif // __BULLET_CLOSEST_HIT_RAY_RESULT_H__
