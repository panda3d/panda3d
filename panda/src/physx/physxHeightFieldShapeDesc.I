/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxHeightFieldShapeDesc.I
 * @author enn0x
 * @date 2009-10-15
 */

/**
 *
 */
INLINE PhysxHeightFieldShapeDesc::
PhysxHeightFieldShapeDesc() : PhysxShapeDesc() {

  set_to_default();
}

/**
 *
 */
INLINE PhysxHeightFieldShapeDesc::
~PhysxHeightFieldShapeDesc() {

}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxHeightFieldShapeDesc::
is_valid() const {

  return _desc.isValid();
}
