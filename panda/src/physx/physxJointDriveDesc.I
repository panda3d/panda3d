/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJointDriveDesc.I
 * @author enn0x
 * @date 2009-10-01
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxJointDriveDesc::
PhysxJointDriveDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxJointDriveDesc::
PhysxJointDriveDesc(float spring, float damping, float forceLimit) {

  _desc.spring = spring;
  _desc.damping = damping;
  _desc.forceLimit = forceLimit;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointDriveDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxJointDriveDesc::
~PhysxJointDriveDesc() {

}

