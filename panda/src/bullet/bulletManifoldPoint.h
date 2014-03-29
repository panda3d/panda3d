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

  int get_life_time() const;
  PN_stdfloat get_distance() const;
  PN_stdfloat get_applied_impulse() const;
  LPoint3 get_position_world_on_a() const;
  LPoint3 get_position_world_on_b() const;
  LPoint3 get_normal_world_on_b() const;
  LPoint3 get_local_point_a() const;
  LPoint3 get_local_point_b() const;

  int get_part_id0() const;
  int get_part_id1() const;
  int get_index0() const;
  int get_index1() const;

  INLINE void set_lateral_friction_initialized(bool value);
  INLINE void set_lateral_friction_dir1(const LVecBase3 &dir);
  INLINE void set_lateral_friction_dir2(const LVecBase3 &dir);
  INLINE void set_contact_motion1(PN_stdfloat value);
  INLINE void set_contact_motion2(PN_stdfloat value);
  INLINE void set_combined_friction(PN_stdfloat value);
  INLINE void set_combined_restitution(PN_stdfloat value);
  INLINE void set_applied_impulse(PN_stdfloat value);
  INLINE void set_applied_impulse_lateral1(PN_stdfloat value);
  INLINE void set_applied_impulse_lateral2(PN_stdfloat value);
  INLINE void set_contact_cfm1(PN_stdfloat value);
  INLINE void set_contact_cfm2(PN_stdfloat value);

  INLINE bool get_lateral_friction_initialized() const;
  INLINE LVector3 get_lateral_friction_dir1() const;
  INLINE LVector3 get_lateral_friction_dir2() const;
  INLINE PN_stdfloat get_contact_motion1() const;
  INLINE PN_stdfloat get_contact_motion2() const;
  INLINE PN_stdfloat get_combined_friction() const;
  INLINE PN_stdfloat get_combined_restitution() const;
  INLINE PN_stdfloat get_applied_impulse_lateral1() const;
  INLINE PN_stdfloat get_applied_impulse_lateral2() const;
  INLINE PN_stdfloat get_contact_cfm1() const;
  INLINE PN_stdfloat get_contact_cfm2() const;

public:
  BulletManifoldPoint(btManifoldPoint &pt);

  BulletManifoldPoint(const BulletManifoldPoint &other);
  BulletManifoldPoint& operator=(const BulletManifoldPoint& other);

private:
  btManifoldPoint &_pt;
};

#include "bulletManifoldPoint.I"

#endif // __BULLET_MANIFOLD_POINT_H__
