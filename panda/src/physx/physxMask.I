/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMask.I
 * @author enn0x
 * @date 2009-10-21
 */

/**
 *
 */
INLINE PhysxMask::
PhysxMask() {

  _mask = 0xffffffff;
}

/**
 *
 */
INLINE PhysxMask::
~PhysxMask() {

}

/**
 *
 */
INLINE NxU32 PhysxMask::
get_mask() const {

  return _mask;
}
