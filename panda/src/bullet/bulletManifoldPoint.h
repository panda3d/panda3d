// Filename: bulletManifoldPoint.h
// Created by:  enn0x (07Mar10)
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

#ifndef __BULLET_MANIFOLD_POINT_H__
#define __BULLET_MANIFOLD_POINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletManifoldPoint
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletManifoldPoint {

PUBLISHED:
  INLINE ~BulletManifoldPoint();

  int get_lift_time() const;
  PN_stdfloat get_distance() const;
  PN_stdfloat get_applied_impulse() const;
  LPoint3 get_position_world_on_a() const;
  LPoint3 get_position_world_on_b() const;
  LPoint3 get_local_point_a() const;
  LPoint3 get_local_point_b() const;

  int get_part_id0() const;
  int get_part_id1() const;
  int get_index0() const;
  int get_index1() const;

public:
  BulletManifoldPoint(btManifoldPoint &pt);

private:
  btManifoldPoint &_pt;
};

#include "bulletManifoldPoint.I"

#endif // __BULLET_MANIFOLD_POINT_H__
