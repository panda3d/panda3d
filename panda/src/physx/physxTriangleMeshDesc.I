/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxTriangleMeshDesc.I
 * @author enn0x
 * @date 2009-10-11
 */

/**
 *
 */
INLINE PhysxTriangleMeshDesc::
PhysxTriangleMeshDesc() {

  _desc.flags = 0;
  _desc.pointStrideBytes = sizeof(NxVec3);
  _desc.triangleStrideBytes = 3*sizeof(NxU32);
  _desc.materialIndexStride = sizeof(NxMaterialIndex);
  _desc.points = nullptr;
  _desc.triangles = nullptr;
  _desc.materialIndices = nullptr;

  _vertices = nullptr;
  _triangles = nullptr;
  _materials = nullptr;
}

/**
 *
 */
INLINE PhysxTriangleMeshDesc::
~PhysxTriangleMeshDesc() {

  if (_vertices) {
    delete [] _vertices;
  }

  if (_triangles) {
    delete [] _triangles;
  }

  if (_materials) {
    delete [] _materials;
  }
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxTriangleMeshDesc::
is_valid() const {

  return _desc.isValid();
}
