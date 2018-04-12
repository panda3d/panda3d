/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTranslationalLimitMotor.h
 * @author enn0x
 * @date 2013-03-03
 */

#ifndef __BULLET_TRANSLATIONAL_LIMIT_MOTOR_H__
#define __BULLET_TRANSLATIONAL_LIMIT_MOTOR_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"

/**
 * Rotation Limit structure for generic joints.
 */
class EXPCL_PANDABULLET BulletTranslationalLimitMotor {

PUBLISHED:
  BulletTranslationalLimitMotor(const BulletTranslationalLimitMotor &copy);
  INLINE ~BulletTranslationalLimitMotor();

  void set_motor_enabled(int axis, bool enable);
  void set_low_limit(const LVecBase3 &limit);
  void set_high_limit(const LVecBase3 &limit);
  void set_target_velocity(const LVecBase3 &velocity);
  void set_max_motor_force(const LVecBase3 &force);
  void set_damping(PN_stdfloat damping);
  void set_softness(PN_stdfloat softness);
  void set_restitution(PN_stdfloat restitution);
  void set_normal_cfm(const LVecBase3 &cfm);
  void set_stop_erp(const LVecBase3 &erp);
  void set_stop_cfm(const LVecBase3 &cfm);

  bool is_limited(int axis) const;
  bool get_motor_enabled(int axis) const;
  int get_current_limit(int axis) const;
  LVector3 get_current_error() const;
  LPoint3 get_current_diff() const;
  LVector3 get_accumulated_impulse() const;

  MAKE_PROPERTY(current_error, get_current_error);
  MAKE_PROPERTY(current_diff, get_current_diff);
  MAKE_PROPERTY(accumulated_impulse, get_accumulated_impulse);

public:
  BulletTranslationalLimitMotor(btTranslationalLimitMotor &motor);

private:
  btTranslationalLimitMotor &_motor;
};

#include "bulletTranslationalLimitMotor.I"

#endif // __BULLET_TRANSLATIONAL_LIMIT_MOTOR_H__
