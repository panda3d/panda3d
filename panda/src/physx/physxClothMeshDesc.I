/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxClothMeshDesc.I
 * @author enn0x
 * @date 2010-03-28
 */

/**
 *
 */
INLINE PhysxClothMeshDesc::
PhysxClothMeshDesc() {

  _desc.flags = 0;
  _desc.pointStrideBytes = sizeof(NxVec3);
  _desc.triangleStrideBytes = 3*sizeof(NxU32);
  _desc.points = nullptr;
  _desc.triangles = nullptr;

  _points = nullptr;
  _triangles = nullptr;
  _texcoords = nullptr;
}

/**
 *
 */
INLINE PhysxClothMeshDesc::
~PhysxClothMeshDesc() {

  if (_points) {
    delete [] _points;
  }

  if (_triangles) {
    delete [] _triangles;
  }

  if (_texcoords) {
    delete [] _texcoords;
  }
}

/**
 * Returns true if the descriptor is valid.
 */
INLINE bool PhysxClothMeshDesc::
is_valid() const {

  return _desc.isValid();
}

/**
 *
 */
INLINE const NxClothMeshDesc &PhysxClothMeshDesc::
get_desc() const {

  return _desc;
}

/**
 *
 */
INLINE const plist<LPoint2f> PhysxClothMeshDesc::
get_texcoords() const {

  plist<LPoint2f> texcoords;

  for (unsigned int i=0; i < _desc.numVertices; i++) {
    LPoint2f uv = _texcoords[i];
    texcoords.push_back(uv);
  }

  return texcoords;
}
