/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxD6Joint.cxx
 * @author enn0x
 * @date 2009-10-02
 */

#include "physxD6Joint.h"
#include "physxD6JointDesc.h"

TypeHandle PhysxD6Joint::_type_handle;

/**
 *
 */
void PhysxD6Joint::
link(NxJoint *jointPtr) {

  _ptr = jointPtr->isD6Joint();
  _ptr->userData = this;
  _error_type = ET_ok;

  set_name(jointPtr->getName());

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.add(this);
}

/**
 *
 */
void PhysxD6Joint::
unlink() {

  _ptr->userData = nullptr;
  _error_type = ET_released;

  PhysxScene *scene = (PhysxScene *)_ptr->getScene().userData;
  scene->_joints.remove(this);
}

/**
 * Saves the state of the joint object to a descriptor.
 */
void PhysxD6Joint::
save_to_desc(PhysxD6JointDesc &jointDesc) const {

  nassertv(_error_type == ET_ok);
  _ptr->saveToDesc(jointDesc._desc);
}

/**
 * Loads the entire state of the joint from a descriptor with a single call.
 */
void PhysxD6Joint::
load_from_desc(const PhysxD6JointDesc &jointDesc) {

  nassertv(_error_type == ET_ok);
  _ptr->loadFromDesc(jointDesc._desc);
}

/**
 *
 */
void PhysxD6Joint::
set_drive_angular_velocity(const LVector3f &v) {

  nassertv(_error_type == ET_ok);
  _ptr->setDriveAngularVelocity(PhysxManager::vec3_to_nxVec3(v));
}

/**
 *
 */
void PhysxD6Joint::
set_drive_linear_velocity(const LVector3f &v) {

  nassertv(_error_type == ET_ok);
  _ptr->setDriveLinearVelocity(PhysxManager::vec3_to_nxVec3(v));
}

/**
 *
 */
void PhysxD6Joint::
set_drive_orientation(const LQuaternionf &quat) {

  nassertv(_error_type == ET_ok);
  _ptr->setDriveOrientation(PhysxManager::quat_to_nxQuat(quat));
}

/**
 *
 */
void PhysxD6Joint::
set_drive_position(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);
  _ptr->setDrivePosition(PhysxManager::point3_to_nxVec3(pos));
}
