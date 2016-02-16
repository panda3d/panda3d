/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBoxControllerDesc.I
 * @author enn0x
 * @date 2009-09-22
 */

/**

 */
INLINE PhysxBoxControllerDesc::
PhysxBoxControllerDesc() : PhysxControllerDesc() {

  _desc.upDirection = (NxHeightFieldAxis)physx_up_axis.get_value();
}

/**

 */
INLINE PhysxBoxControllerDesc::
~PhysxBoxControllerDesc() {

}

/**
 * (re)sets the structure to the default.
 */
INLINE void PhysxBoxControllerDesc::
set_to_default() {

  _desc.setToDefault();
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxBoxControllerDesc::
is_valid() const {

  return _desc.isValid();
}
