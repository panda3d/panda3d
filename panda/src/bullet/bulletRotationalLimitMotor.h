/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletRotationalLimitMotor.h
 * @author enn0x
 * @date 2013-03-03
 */

#ifndef __BULLET_ROTATIONAL_LIMIT_MOTOR_H__
#define __BULLET_ROTATIONAL_LIMIT_MOTOR_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"

/**
 * Rotation Limit structure for generic joints.
 */
class EXPCL_PANDABULLET BulletRotationalLimitMotor {

PUBLISHED:
  BulletRotationalLimitMotor(const BulletRotationalLimitMotor &copy);
  ~BulletRotationalLimitMotor();

  INLINE void set_motor_enabled(bool enable);
  INLINE void set_low_limit(PN_stdfloat limit);
  INLINE void set_high_limit(PN_stdfloat limit);
  INLINE void set_target_velocity(PN_stdfloat velocity);
  INLINE void set_max_motor_force(PN_stdfloat force);
  INLINE void set_max_limit_force(PN_stdfloat force);
  INLINE void set_damping(PN_stdfloat damping);
  INLINE void set_softness(PN_stdfloat softness);
  INLINE void set_bounce(PN_stdfloat bounce);
  INLINE void set_normal_cfm(PN_stdfloat cfm);
  INLINE void set_stop_cfm(PN_stdfloat cfm);
  INLINE void set_stop_erp(PN_stdfloat erp);

  INLINE bool is_limited() const;
  INLINE bool get_motor_enabled() const;
  INLINE int get_current_limit() const;
  INLINE PN_stdfloat get_current_error() const;
  INLINE PN_stdfloat get_current_position() const;
  INLINE PN_stdfloat get_accumulated_impulse() const;

  MAKE_PROPERTY(limited, is_limited);
  MAKE_PROPERTY(motor_enabled, get_motor_enabled, set_motor_enabled);
  MAKE_PROPERTY(current_limit, get_current_limit);
  MAKE_PROPERTY(current_error, get_current_error);
  MAKE_PROPERTY(current_position, get_current_position);
  MAKE_PROPERTY(accumulated_impulse, get_accumulated_impulse);

public:
  BulletRotationalLimitMotor(btRotationalLimitMotor &motor);

private:
  btRotationalLimitMotor &_motor;
};

#include "bulletRotationalLimitMotor.I"

#endif // __BULLET_ROTATIONAL_LIMIT_MOTOR_H__
