/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxConvexMeshDesc.I
 * @author enn0x
 * @date 2009-10-11
 */

/**
 *
 */
INLINE PhysxConvexMeshDesc::
PhysxConvexMeshDesc() {

  _desc.flags = NX_CF_COMPUTE_CONVEX;
  _desc.pointStrideBytes = sizeof(NxVec3);
  _desc.points = nullptr;

  _vertices = nullptr;
}

/**
 *
 */
INLINE PhysxConvexMeshDesc::
~PhysxConvexMeshDesc() {

  if (_vertices) {
    delete [] _vertices;
  }
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxConvexMeshDesc::
is_valid() const {

  return _desc.isValid();
}
