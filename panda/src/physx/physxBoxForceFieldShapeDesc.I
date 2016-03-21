/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxBoxForceFieldShapeDesc.I
 * @author enn0x
 * @date 2009-11-06
 */

/**
 *
 */
INLINE PhysxBoxForceFieldShapeDesc::
PhysxBoxForceFieldShapeDesc() : PhysxForceFieldShapeDesc() {

  set_to_default();
}

/**
 *
 */
INLINE PhysxBoxForceFieldShapeDesc::
~PhysxBoxForceFieldShapeDesc() {

}

/**
 * (re)sets the structure to the default.
 */
INLINE void PhysxBoxForceFieldShapeDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxBoxForceFieldShapeDesc::
is_valid() const {

  return _desc.isValid();
}
