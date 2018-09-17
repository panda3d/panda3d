/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSphericalJoint.cxx
 * @author enn0x
 * @date 2009-10-02
 */

#include "physxSphericalJoint.h"
#include "physxSphericalJointDesc.h"

TypeHandle PhysxSphericalJoint::_type_handle;

/**
 *
 */
void PhysxSphericalJoint::
link(NxJoint *jointPtr) {

  _ptr = jointPtr->isSphericalJoint();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(jointPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.add(this);
}

/**
 *
 */
void PhysxSphericalJoint::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.remove(this);
}

/**
 * Saves the state of the joint object to a descriptor.
 */
void PhysxSphericalJoint::
save_to_desc(PhysxSphericalJointDesc &jointDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(jointDesc._desc);
}

/**
 * Loads the entire state of the joint from a descriptor with a single call.
 */
void PhysxSphericalJoint::
load_from_desc(const PhysxSphericalJointDesc &jointDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(jointDesc._desc);
}

/**
 * Sets the joint projection mode.
 */
void PhysxSphericalJoint::
set_projection_mode(PhysxProjectionMode mode) {

  nassertv(_error_type == ET_ok);
  _ptr->setProjectionMode((NxJointProjectionMode)mode);
}

/**
 * Returns the current projection mode settings.
 */
PhysxEnums::PhysxProjectionMode PhysxSphericalJoint::
get_projection_mode() const {

  nassertr(_error_type == ET_ok, PM_none);
  return (PhysxProjectionMode)_ptr->getProjectionMode();
}

/**
 * Sets or clears a single SphericalJointFlag.
 */
void PhysxSphericalJoint::
set_flag(PhysxSphericalJointFlag flag, bool value) {

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
 * Returns the value of a single SphericalJointFlag.
 */
bool PhysxSphericalJoint::
get_flag(PhysxSphericalJointFlag flag) const {

  nassertr(_error_type == ET_ok, false);
  return (_ptr->getFlags() & flag) ? true : false;
}
