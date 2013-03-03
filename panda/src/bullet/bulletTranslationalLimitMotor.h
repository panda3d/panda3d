// Filename: bulletTranslationalLimitMotor.h
// Created by:  enn0x (03Mar13)
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

#ifndef __BULLET_TRANSLATIONAL_LIMIT_MOTOR_H__
#define __BULLET_TRANSLATIONAL_LIMIT_MOTOR_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletTranslationalLimitMotor
// Description : Rotation Limit structure for generic joints.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletTranslationalLimitMotor {

PUBLISHED:
  BulletTranslationalLimitMotor(const BulletTranslationalLimitMotor &copy);
  ~BulletTranslationalLimitMotor();

  INLINE void set_motor_enabled(int axis, bool enable);
  INLINE void set_low_limit(const LVecBase3 &limit);
  INLINE void set_high_limit(const LVecBase3 & limit);
  INLINE void set_target_velocity(const LVecBase3&velocity);
  INLINE void set_max_motor_force(const LVecBase3 &force);
  INLINE void set_damping(PN_stdfloat damping);
  INLINE void set_softness(PN_stdfloat softness);
  INLINE void set_restitution(PN_stdfloat restitution);
  INLINE void set_normal_cfm(const LVecBase3 &cfm);
  INLINE void set_stop_erp(const LVecBase3 &erp);
  INLINE void set_stop_cfm(const LVecBase3 &cfm);

  INLINE bool is_limited(int axis) const;
  INLINE bool get_motor_enabled(int axis) const;
  INLINE int get_current_limit(int axis) const;
  INLINE LVector3 get_current_error() const;
  INLINE LPoint3 get_current_diff() const;
  INLINE LVector3 get_accumulated_impulse() const;

public:
  BulletTranslationalLimitMotor(btTranslationalLimitMotor &motor);

private:
  btTranslationalLimitMotor &_motor;
};

#include "bulletTranslationalLimitMotor.I"

#endif // __BULLET_TRANSLATIONAL_LIMIT_MOTOR_H__
