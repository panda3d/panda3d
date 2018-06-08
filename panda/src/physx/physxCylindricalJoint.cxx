/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCylindricalJoint.cxx
 * @author enn0x
 * @date 2009-10-02
 */

#include "physxCylindricalJoint.h"
#include "physxCylindricalJointDesc.h"

TypeHandle PhysxCylindricalJoint::_type_handle;

/**
 *
 */
void PhysxCylindricalJoint::
link(NxJoint *jointPtr) {

  _ptr = jointPtr->isCylindricalJoint();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(jointPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.add(this);
}

/**
 *
 */
void PhysxCylindricalJoint::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.remove(this);
}

/**
 * Saves the state of the joint object to a descriptor.
 */
void PhysxCylindricalJoint::
save_to_desc(PhysxCylindricalJointDesc &jointDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(jointDesc._desc);
}

/**
 * Loads the entire state of the joint from a descriptor with a single call.
 */
void PhysxCylindricalJoint::
load_from_desc(const PhysxCylindricalJointDesc &jointDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(jointDesc._desc);
}
