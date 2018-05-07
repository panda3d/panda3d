/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletManifoldPoint.h
 * @author enn0x
 * @date 2010-03-07
 */

#ifndef __BULLET_MANIFOLD_POINT_H__
#define __BULLET_MANIFOLD_POINT_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletManifoldPoint {

PUBLISHED:
  INLINE ~BulletManifoldPoint();

  int get_life_time() const;
  PN_stdfloat get_distance() const;
  PN_stdfloat get_applied_impulse() const;
  LPoint3 get_position_world_on_a() const;
  LPoint3 get_position_world_on_b() const;
  LVector3 get_normal_world_on_b() const;
  LPoint3 get_local_point_a() const;
  LPoint3 get_local_point_b() const;

  int get_part_id0() const;
  int get_part_id1() const;
  int get_index0() const;
  int get_index1() const;

  void set_lateral_friction_initialized(bool value);
  void set_lateral_friction_dir1(const LVecBase3 &dir);
  void set_lateral_friction_dir2(const LVecBase3 &dir);
  void set_contact_motion1(PN_stdfloat value);
  void set_contact_motion2(PN_stdfloat value);
  void set_combined_friction(PN_stdfloat value);
  void set_combined_restitution(PN_stdfloat value);
  void set_applied_impulse(PN_stdfloat value);
  void set_applied_impulse_lateral1(PN_stdfloat value);
  void set_applied_impulse_lateral2(PN_stdfloat value);
  void set_contact_cfm1(PN_stdfloat value);
  void set_contact_cfm2(PN_stdfloat value);

  bool get_lateral_friction_initialized() const;
  LVector3 get_lateral_friction_dir1() const;
  LVector3 get_lateral_friction_dir2() const;
  PN_stdfloat get_contact_motion1() const;
  PN_stdfloat get_contact_motion2() const;
  PN_stdfloat get_combined_friction() const;
  PN_stdfloat get_combined_restitution() const;
  PN_stdfloat get_applied_impulse_lateral1() const;
  PN_stdfloat get_applied_impulse_lateral2() const;
  PN_stdfloat get_contact_cfm1() const;
  PN_stdfloat get_contact_cfm2() const;

  MAKE_PROPERTY(life_time, get_life_time);
  MAKE_PROPERTY(distance, get_distance);
  MAKE_PROPERTY(applied_impulse, get_applied_impulse, set_applied_impulse);
  MAKE_PROPERTY(position_world_on_a, get_position_world_on_a);
  MAKE_PROPERTY(position_world_on_b, get_position_world_on_b);
  MAKE_PROPERTY(normal_world_on_b, get_normal_world_on_b);
  MAKE_PROPERTY(local_point_a, get_local_point_a);
  MAKE_PROPERTY(local_point_b, get_local_point_b);
  MAKE_PROPERTY(part_id0, get_part_id0);
  MAKE_PROPERTY(part_id1, get_part_id1);
  MAKE_PROPERTY(index0, get_index0);
  MAKE_PROPERTY(index1, get_index1);
  MAKE_PROPERTY(lateral_friction_initialized, get_lateral_friction_initialized, set_lateral_friction_initialized);
  MAKE_PROPERTY(lateral_friction_dir1, get_lateral_friction_dir1, set_lateral_friction_dir1);
  MAKE_PROPERTY(lateral_friction_dir2, get_lateral_friction_dir2, set_lateral_friction_dir2);
  MAKE_PROPERTY(contact_motion1, get_contact_motion1, set_contact_motion1);
  MAKE_PROPERTY(contact_motion2, get_contact_motion2, set_contact_motion2);
  MAKE_PROPERTY(combined_friction, get_combined_friction, set_combined_friction);
  MAKE_PROPERTY(combined_restitution, get_combined_restitution, set_combined_restitution);
  MAKE_PROPERTY(applied_impulse_lateral1, get_applied_impulse_lateral1, set_applied_impulse_lateral1);
  MAKE_PROPERTY(applied_impulse_lateral2, get_applied_impulse_lateral2, set_applied_impulse_lateral2);
  MAKE_PROPERTY(contact_cfm1, get_contact_cfm1, set_contact_cfm1);
  MAKE_PROPERTY(contact_cfm2, get_contact_cfm2, set_contact_cfm2);

public:
  BulletManifoldPoint(btManifoldPoint &pt);

  BulletManifoldPoint(const BulletManifoldPoint &other);
  BulletManifoldPoint& operator=(const BulletManifoldPoint& other);

private:
  btManifoldPoint &_pt;
};

#include "bulletManifoldPoint.I"

#endif // __BULLET_MANIFOLD_POINT_H__
