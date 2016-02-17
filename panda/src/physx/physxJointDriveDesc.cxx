/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJointDriveDesc.cxx
 * @author enn0x
 * @date 2009-10-01
 */

#include "physxJointDriveDesc.h"

/**

 */
void PhysxJointDriveDesc::
set_spring(float spring) {

  _desc.spring = spring;
}

/**

 */
void PhysxJointDriveDesc::
set_damping(float damping) {

  _desc.damping = damping;
}

/**

 */
void PhysxJointDriveDesc::
set_force_limit(float forceLimit) {

  _desc.forceLimit = forceLimit;
}

/**

 */
void PhysxJointDriveDesc::
set_drive_type(PhysxD6JointDriveType driveType) {

  _desc.driveType = (NxD6JointDriveType)driveType;
}

/**

 */
float PhysxJointDriveDesc::
get_spring() const {

  return _desc.spring;
}

/**

 */
float PhysxJointDriveDesc::
get_damping() const {

  return _desc.damping;
}

/**

 */
float PhysxJointDriveDesc::
get_force_limit() const {

  return _desc.forceLimit;
}

/**

 */
PhysxEnums::PhysxD6JointDriveType PhysxJointDriveDesc::
get_drive_type() const {

  return (PhysxD6JointDriveType)_desc.driveType.bitField;
}
