/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxPulleyJoint.cxx
 * @author enn0x
 * @date 2009-10-02
 */

#include "physxPulleyJoint.h"
#include "physxPulleyJointDesc.h"
#include "physxMotorDesc.h"

TypeHandle PhysxPulleyJoint::_type_handle;

/**
 *
 */
void PhysxPulleyJoint::
link(NxJoint *jointPtr) {

  _ptr = jointPtr->isPulleyJoint();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(jointPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.add(this);
}

/**
 *
 */
void PhysxPulleyJoint::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.remove(this);
}

/**
 * Saves the state of the joint object to a descriptor.
 */
void PhysxPulleyJoint::
save_to_desc(PhysxPulleyJointDesc &jointDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(jointDesc._desc);
}

/**
 * Loads the entire state of the joint from a descriptor with a single call.
 */
void PhysxPulleyJoint::
load_from_desc(const PhysxPulleyJointDesc &jointDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(jointDesc._desc);
}

/**
 * Sets motor parameters for the joint.
 *
 * For a positive velTarget, the motor pulls the first body towards its
 * pulley, for a negative velTarget, the motor pulls the second body towards
 * its pulley.
 *
 * velTarget - the relative velocity the motor is trying to achieve.  The
 * motor will only be able to reach this velocity if the maxForce is
 * sufficiently large.  If the joint is moving faster than this velocity, the
 * motor will actually try to brake.  If you set this to infinity then the
 * motor will keep speeding up, unless there is some sort of resistance on the
 * attached bodies.
 *
 * maxForce - the maximum force the motor can exert.  Zero disables the motor.
 * Default is 0, should be >= 0. Setting this to a very large value if
 * velTarget is also very large may not be a good idea.
 *
 * freeSpin - if this flag is set, and if the joint is moving faster than
 * velTarget, then neither braking nor additional acceleration will result.
 * default: false.
 *
 * This automatically enables the motor.
 */
void PhysxPulleyJoint::
set_motor(const PhysxMotorDesc &motor) {

  nassertv(_error_type == ET_ok);
  _ptr->setMotor(motor._desc);
}

/**
 * Sets or clear a single pulley joint flag.
 */
void PhysxPulleyJoint::
set_flag(PhysxPulleyJointFlag flag, bool value) {

  nassertv(_error_type == ET_ok);
  NxU32 flags = _ptr->getFlags();

  if (value == true) {
    flags |= flag;
  }
  else {
    flags &= ~(flag);
  }

  _ptr->setFlags(flags);
}

/**
 * Retrieves the value of a single PulleyJointFlag.
 */
bool PhysxPulleyJoint::
get_flag(PhysxPulleyJointFlag flag) const {

  nassertr(_error_type == ET_ok, false);
  return (_ptr->getFlags() & flag) ? true : false;
}

/**
 *
 */
PhysxMotorDesc PhysxPulleyJoint::
get_motor() const {

  nassertr(_error_type == ET_ok, false);

  PhysxMotorDesc value;
  _ptr->getMotor(value._desc);
  return value;
}
