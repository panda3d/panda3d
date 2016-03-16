/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMotorDesc.I
 * @author enn0x
 * @date 2009-09-28
 */

/**
 *
 */
INLINE PhysxMotorDesc::
PhysxMotorDesc() {

}

/**
 *
 */
INLINE PhysxMotorDesc::
PhysxMotorDesc(float velTarget, float maxForce, bool freeSpin) {

  _desc.velTarget = velTarget;
  _desc.maxForce = maxForce;
  _desc.freeSpin = freeSpin;
}

/**
 *
 */
INLINE PhysxMotorDesc::
~PhysxMotorDesc() {

}
