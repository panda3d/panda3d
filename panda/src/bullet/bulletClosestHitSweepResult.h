/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletClosestHitSweepResult.h
 * @author enn0x
 * @date 2010-12-01
 */

#ifndef BULLETCLOSESTHITSWEEPRESULT_H
#define BULLETCLOSESTHITSWEEPRESULT_H

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"
#include "memoryBase.h"
#include "pandaNode.h"
#include "collideMask.h"

/**
 *
 */
struct EXPCL_PANDABULLET BulletClosestHitSweepResult : public btCollisionWorld::ClosestConvexResultCallback, public MemoryBase {

PUBLISHED:
  INLINE static BulletClosestHitSweepResult empty();

  LPoint3 get_from_pos() const;
  LPoint3 get_to_pos() const;

  bool has_hit() const;

  PandaNode *get_node() const;
  LPoint3 get_hit_pos() const;
  LVector3 get_hit_normal() const;
  PN_stdfloat get_hit_fraction() const;

  MAKE_PROPERTY(from_pos, get_from_pos);
  MAKE_PROPERTY(to_pos, get_to_pos);
  MAKE_PROPERTY(node, get_node);
  MAKE_PROPERTY(hit_pos, get_hit_pos);
  MAKE_PROPERTY(hit_normal, get_hit_normal);
  MAKE_PROPERTY(hit_fraction, get_hit_fraction);

public:
  virtual bool needsCollision(btBroadphaseProxy* proxy0) const;

private:
  BulletClosestHitSweepResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask);

  CollideMask _mask;

  friend class BulletWorld;
};

#include "bulletClosestHitSweepResult.I"

#endif // BULLETCLOSESTHITSWEEPRESULT_H
