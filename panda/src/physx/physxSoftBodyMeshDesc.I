/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSoftBodyMeshDesc.I
 * @author enn0x
 * @date 2010-09-12
 */

/**
 *
 */
INLINE PhysxSoftBodyMeshDesc::
PhysxSoftBodyMeshDesc() {

  _desc.flags = 0;
  _desc.vertexStrideBytes = sizeof(NxVec3);
  _desc.tetrahedronStrideBytes = 4*sizeof(NxU32);
  _desc.vertices = nullptr;
  _desc.tetrahedra = nullptr;

  _vertices = nullptr;
  _tetrahedra = nullptr;
}

/**
 *
 */
INLINE PhysxSoftBodyMeshDesc::
~PhysxSoftBodyMeshDesc() {

  if (_vertices) {
    delete [] _vertices;
  }

  if (_tetrahedra) {
    delete [] _tetrahedra;
  }
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxSoftBodyMeshDesc::
is_valid() const {

  return _desc.isValid();
}

/**
 *
 */
INLINE const NxSoftBodyMeshDesc &PhysxSoftBodyMeshDesc::
get_desc() const {

  return _desc;
}
