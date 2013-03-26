// Filename: bulletClosestHitSweepResult.h
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

#ifndef __BULLET_CLOSEST_HIT_SWEEP_RESULT_H__
#define __BULLET_CLOSEST_HIT_SWEEP_RESULT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"
#include "pandaNode.h"
#include "collideMask.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletClosestHitSweepResult
// Description : 
////////////////////////////////////////////////////////////////////
struct EXPCL_PANDABULLET BulletClosestHitSweepResult : public btCollisionWorld::ClosestConvexResultCallback {

PUBLISHED:
  INLINE static BulletClosestHitSweepResult empty();

  LPoint3 get_from_pos() const;
  LPoint3 get_to_pos() const;

  bool has_hit() const;

  const PandaNode *get_node() const;
  LPoint3 get_hit_pos() const;
  LVector3 get_hit_normal() const;
  PN_stdfloat get_hit_fraction() const;

public:
  virtual bool needsCollision(btBroadphaseProxy* proxy0) const;  

private:
  BulletClosestHitSweepResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask);

  CollideMask _mask;

  friend class BulletWorld;
};

#include "bulletClosestHitSweepResult.I"

#endif // __BULLET_CLOSEST_HIT_SWEEP_RESULT_H__
