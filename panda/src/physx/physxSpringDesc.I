/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSpringDesc.I
 * @author enn0x
 * @date 2009-09-28
 */

/**
 *
 */
INLINE PhysxSpringDesc::
PhysxSpringDesc() {

}

/**
 *
 */
INLINE PhysxSpringDesc::
PhysxSpringDesc(float spring, float damper, float target) {

  _desc.spring = spring;
  _desc.damper = damper;
  _desc.targetValue = target;
}

/**
 *
 */
INLINE PhysxSpringDesc::
~PhysxSpringDesc() {

}
