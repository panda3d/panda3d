/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletWheel.I
 * @author enn0x
 * @date 2010-02-17
 */

/**
 *
 */
INLINE BulletWheel::
~BulletWheel() {

}

/**
 *
 */
INLINE BulletWheelRaycastInfo::
~BulletWheelRaycastInfo() {

}

/**
 * Named constructor intended to be used for asserts with have to return a
 * concrete value.
 */
INLINE BulletWheel BulletWheel::
empty() {

  btWheelInfoConstructionInfo ci {};
  btWheelInfo info(ci);

  return BulletWheel(info);
}

