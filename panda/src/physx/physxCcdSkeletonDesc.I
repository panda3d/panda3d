/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCcdSkeletonDesc.I
 * @author enn0x
 * @date 2012-05-01
 */

/**
 *
 */
INLINE PhysxCcdSkeletonDesc::
PhysxCcdSkeletonDesc() {

  _desc.flags = 0;
  _desc.pointStrideBytes = sizeof(NxVec3);
  _desc.triangleStrideBytes = 3*sizeof(NxU32);
  _desc.points = nullptr;
  _desc.triangles = nullptr;

  _vertices = nullptr;
  _triangles = nullptr;
}

/**
 *
 */
INLINE PhysxCcdSkeletonDesc::
~PhysxCcdSkeletonDesc() {

  if (_vertices) {
    delete [] _vertices;
  }

  if (_triangles) {
    delete [] _triangles;
  }
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxCcdSkeletonDesc::
is_valid() const {

  return _desc.isValid();
}
