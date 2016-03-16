/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMotorDesc.cxx
 * @author enn0x
 * @date 2009-09-28
 */

#include "physxMotorDesc.h"

/**
 *
 */
void PhysxMotorDesc::
set_vel_target(float velTarget) {

  _desc.velTarget = velTarget;
}

/**
 *
 */
void PhysxMotorDesc::
set_max_force(float maxForce) {

  _desc.maxForce = maxForce;
}

/**
 *
 */
void PhysxMotorDesc::
set_free_spin(bool freeSpin) {

  _desc.freeSpin = (NX_BOOL)freeSpin;
}

/**
 *
 */
float PhysxMotorDesc::
get_vel_target() const {

  return _desc.velTarget;
}

/**
 *
 */
float PhysxMotorDesc::
get_max_force() const {

  return _desc.maxForce;
}

/**
 *
 */
bool PhysxMotorDesc::
get_free_spin() const {

  return (_desc.freeSpin) ? true : false;
}
