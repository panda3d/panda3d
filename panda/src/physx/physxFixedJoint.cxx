/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxFixedJoint.cxx
 * @author enn0x
 * @date 2009-10-02
 */

#include "physxFixedJoint.h"
#include "physxFixedJointDesc.h"

TypeHandle PhysxFixedJoint::_type_handle;

/**
 *
 */
void PhysxFixedJoint::
link(NxJoint *jointPtr) {

  _ptr = jointPtr->isFixedJoint();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(jointPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.add(this);
}

/**
 *
 */
void PhysxFixedJoint::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.remove(this);
}

/**
 * Saves the state of the joint object to a descriptor.
 */
void PhysxFixedJoint::
save_to_desc(PhysxFixedJointDesc &jointDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(jointDesc._desc);
}

/**
 * Loads the entire state of the joint from a descriptor with a single call.
 */
void PhysxFixedJoint::
load_from_desc(const PhysxFixedJointDesc &jointDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(jointDesc._desc);
}
