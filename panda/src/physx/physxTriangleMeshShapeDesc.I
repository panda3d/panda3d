/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxTriangleMeshShapeDesc.I
 * @author enn0x
 * @date 2009-10-14
 */

/**
 *
 */
INLINE PhysxTriangleMeshShapeDesc::
PhysxTriangleMeshShapeDesc() : PhysxShapeDesc() {

  set_to_default();
}

/**
 *
 */
INLINE PhysxTriangleMeshShapeDesc::
~PhysxTriangleMeshShapeDesc() {

}

/**
 * (re)sets the structure to the default.
 */
INLINE void PhysxTriangleMeshShapeDesc::
set_to_default() {

  _desc.setToDefault();
  set_name("");
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxTriangleMeshShapeDesc::
is_valid() const {

  return _desc.isValid();
}
