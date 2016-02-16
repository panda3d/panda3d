/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxPointInPlaneJointDesc.I
 * @author enn0x
 * @date 2009-09-28
 */

/**

 */
INLINE PhysxPointInPlaneJointDesc::
PhysxPointInPlaneJointDesc() : PhysxJointDesc() {

  set_to_default();
}

/**

 */
INLINE PhysxPointInPlaneJointDesc::
~PhysxPointInPlaneJointDesc() {

}

/**
 * (re)sets the structure to the default.
 */
INLINE void PhysxPointInPlaneJointDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxPointInPlaneJointDesc::
is_valid() const {

  return _desc.isValid();
}
