/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxDistanceJointDesc.I
 * @author enn0x
 * @date 2009-09-28
 */

/**
 *
 */
INLINE PhysxDistanceJointDesc::
PhysxDistanceJointDesc() : PhysxJointDesc() {

  set_to_default();
}

/**
 *
 */
INLINE PhysxDistanceJointDesc::
~PhysxDistanceJointDesc() {

}

/**
 * (re)sets the structure to the default.
 */
INLINE void PhysxDistanceJointDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxDistanceJointDesc::
is_valid() const {

  return _desc.isValid();
}
