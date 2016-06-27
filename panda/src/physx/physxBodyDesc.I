/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBodyDesc.I
 * @author enn0x
 * @date 2009-09-05
 */

/**
 *
 */
INLINE PhysxBodyDesc::
PhysxBodyDesc() {

}

/**
 *
 */
INLINE PhysxBodyDesc::
~PhysxBodyDesc() {

}

/**
 * (re)sets the structure to the default.
 */
INLINE void PhysxBodyDesc::
set_to_default() {

  _desc.setToDefault();
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxBodyDesc::
is_valid() const {

  return _desc.isValid();
}
