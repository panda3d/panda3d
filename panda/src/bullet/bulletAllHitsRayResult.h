/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletAllHitsRayResult.h
 * @author enn0x
 * @date 2010-02-21
 */

#ifndef __BULLET_ALL_HITS_RAY_RESULT_H__
#define __BULLET_ALL_HITS_RAY_RESULT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"
#include "pandaNode.h"
#include "collideMask.h"

/**
 *
 */
struct EXPCL_PANDABULLET BulletRayHit {

PUBLISHED:
  INLINE static BulletRayHit empty();

  PandaNode *get_node() const;
  LPoint3 get_hit_pos() const;
  LVector3 get_hit_normal() const;
  PN_stdfloat get_hit_fraction() const;

  int get_shape_part() const;
  int get_triangle_index() const;

  MAKE_PROPERTY(node, get_node);
  MAKE_PROPERTY(hit_pos, get_hit_pos);
  MAKE_PROPERTY(hit_normal, get_hit_normal);
  MAKE_PROPERTY(hit_fraction, get_hit_fraction);
  MAKE_PROPERTY(shape_part, get_shape_part);
  MAKE_PROPERTY(triangle_index, get_triangle_index);

private:
  const btCollisionObject *_object;
  btVector3 _normal;
  btVector3 _pos;
  btScalar _fraction;

  int _shapePart;
  int _triangleIndex;

  friend struct BulletAllHitsRayResult;
};

/**
 *
 */
struct EXPCL_PANDABULLET BulletAllHitsRayResult : public btCollisionWorld::AllHitsRayResultCallback {

PUBLISHED:
  INLINE static BulletAllHitsRayResult empty();

  LPoint3 get_from_pos() const;
  LPoint3 get_to_pos() const;

  bool has_hits() const;
  PN_stdfloat get_closest_hit_fraction() const;

  int get_num_hits() const;
  const BulletRayHit get_hit(int idx) const;
  MAKE_SEQ(get_hits, get_num_hits, get_hit);

  MAKE_PROPERTY(from_pos, get_from_pos);
  MAKE_PROPERTY(to_pos, get_to_pos);
  MAKE_PROPERTY(closest_hit_fraction, get_closest_hit_fraction);
  MAKE_SEQ_PROPERTY(hits, get_num_hits, get_hit);

public:
  virtual bool needsCollision(btBroadphaseProxy* proxy0) const;
  virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace);

private:
  BulletAllHitsRayResult(const btVector3 &from_pos, const btVector3 &to_pos, const CollideMask &mask);

  CollideMask _mask;

  btAlignedObjectArray<int> _shapePart;
  btAlignedObjectArray<int> _triangleIndex;

  friend class BulletWorld;
};

#include "bulletAllHitsRayResult.I"

#endif // __BULLET_ALL_HITS_RAY_RESULT_H__
