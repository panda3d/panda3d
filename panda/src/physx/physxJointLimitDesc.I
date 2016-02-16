/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJointLimitDesc.I
 * @author enn0x
 * @date 2009-09-28
 */

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointLimitDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxJointLimitDesc::
PhysxJointLimitDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointLimitDesc::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxJointLimitDesc::
PhysxJointLimitDesc(float value, float restitution, float hardness) {

  _desc.value = value;
  _desc.restitution = restitution;
  _desc.hardness = hardness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxJointLimitDesc::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxJointLimitDesc::
~PhysxJointLimitDesc() {

}

