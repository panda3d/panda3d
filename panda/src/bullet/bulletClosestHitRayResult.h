/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletClosestHitRayResult.h
 * @author enn0x
 * @date 2010-02-21
 */

#ifndef __BULLET_CLOSEST_HIT_RAY_RESULT_H__
#define __BULLET_CLOSEST_HIT_RAY_RESULT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"
#include "pandaNode.h"
#include "collideMask.h"

/**
 *
 */
struct EXPCL_PANDABULLET BulletClosestHitRayResult : public btCollisionWorld::ClosestRayResultCallback {

PUBLISHED:
  INLINE static BulletClosestHitRayResult empty();

  LPoint3 get_from_pos() const;
  LPoint3 get_to_pos() const;

  bool has_hit() const;

  PandaNode *get_node() const;
  LPoint3 get_hit_pos() const;
  LVector3 get_hit_normal() const;
  PN_stdfloat get_hit_fraction() const;

  int get_shape_part() const;
  int get_triangle_index() const;

  MAKE_PROPERTY(from_pos, get_from_pos);
  MAKE_PROPERTY(to_pos, get_to_pos);
  MAKE_PROPERTY(node, get_node);
  MAKE_PROPERTY(hit_pos, get_hit_pos);
  MAKE_PROPERTY(hit_normal, get_hit_normal);
  MAKE_PROPERTY(hit_fraction, get_hit_fraction);
  MAKE_PROPERTY(shape_part, get_shape_part);
  MAKE_PROPERTY(triangle_index, get_triangle_index);

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
