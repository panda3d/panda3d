/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJointLimitSoftDesc.cxx
 * @author enn0x
 * @date 2009-10-01
 */

#include "physxJointLimitSoftDesc.h"

/**
 *
 */
void PhysxJointLimitSoftDesc::
set_value(float value) {

  _desc.value = value;
}

/**
 *
 */
void PhysxJointLimitSoftDesc::
set_restitution(float restitution) {

  _desc.restitution = restitution;
}

/**
 *
 */
void PhysxJointLimitSoftDesc::
set_spring(float spring) {

  _desc.spring = spring;
}

/**
 *
 */
void PhysxJointLimitSoftDesc::
set_damping(float damping) {

  _desc.damping = damping;
}

/**
 *
 */
float PhysxJointLimitSoftDesc::
get_value() const {

  return _desc.value;
}

/**
 *
 */
float PhysxJointLimitSoftDesc::
get_restitution() const {

  return _desc.restitution;
}

/**
 *
 */
float PhysxJointLimitSoftDesc::
get_spring() const {

  return _desc.spring;
}

/**
 *
 */
float PhysxJointLimitSoftDesc::
get_damping() const {

  return _desc.damping;
}
