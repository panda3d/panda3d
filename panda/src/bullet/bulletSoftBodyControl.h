// Filename: bulletSoftBodyControl.h
// Created by:  enn0x (04Mar10)
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

#ifndef __BULLET_SOFT_BODY_CONTROL_H__
#define __BULLET_SOFT_BODY_CONTROL_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletSoftBodyControl
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletSoftBodyControl : public btSoftBody::AJoint::IControl {

PUBLISHED:
  BulletSoftBodyControl();
  virtual ~BulletSoftBodyControl();

  // Motor
  INLINE void set_goal(PN_stdfloat goal);
  INLINE void set_max_torque(PN_stdfloat maxtorque);

  //INLINE PN_stdfloat get_goal() const;
  //INLINE PN_stdfloat get_max_torque() const;

  // Steer
  INLINE void set_angle(PN_stdfloat angle);
  INLINE void set_sign(PN_stdfloat sign);

  //INLINE PN_stdfloat get_angle() const;
  //INLINE PN_stdfloat get_sign() const;

public:
  void Prepare(btSoftBody::AJoint* joint);
  btScalar Speed(btSoftBody::AJoint *joint, btScalar current);

private:
  // Motor
  btScalar _goal;
  btScalar _maxtorque;

  // Steer
  btScalar _angle;
  btScalar _sign;
};

#include "bulletSoftBodyControl.I"

#endif // __BULLET_SOFT_BODY_CONTROL_H__
