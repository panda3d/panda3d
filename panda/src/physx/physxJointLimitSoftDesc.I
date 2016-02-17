/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxJointLimitSoftDesc.I
 * @author enn0x
 * @date 2009-10-01
 */

/**

 */
INLINE PhysxJointLimitSoftDesc::
PhysxJointLimitSoftDesc() {

}

/**

 */
INLINE PhysxJointLimitSoftDesc::
PhysxJointLimitSoftDesc(float value, float restitution, float spring, float damping) {

  _desc.value = value;
  _desc.restitution = restitution;
  _desc.spring = spring;
  _desc.damping = damping;
}

/**

 */
INLINE PhysxJointLimitSoftDesc::
~PhysxJointLimitSoftDesc() {

}
