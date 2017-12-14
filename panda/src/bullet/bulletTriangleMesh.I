/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTriangleMesh.I
 * @author enn0x
 * @date 2010-02-09
 */

/**
 *
 */
INLINE btStridingMeshInterface *BulletTriangleMesh::
ptr() const {
  return (btStridingMeshInterface *)&_mesh;
}

/**
 * Returns the number of vertices in this triangle mesh.
 */
INLINE size_t BulletTriangleMesh::
get_num_vertices() const {
  return _vertices.size();
}

/**
 * Returns the vertex at the given vertex index.
 */
INLINE LPoint3 BulletTriangleMesh::
get_vertex(size_t index) const {
  nassertr(index < _vertices.size(), LPoint3::zero());
  const btVector3 &vertex = _vertices[index];
  return LPoint3(vertex[0], vertex[1], vertex[2]);
}

/**
 * Returns the vertex indices making up the given triangle index.
 */
INLINE LVecBase3i BulletTriangleMesh::
get_triangle(size_t index) const {
  index *= 3;
  nassertr(index + 2 < _indices.size(), LVecBase3i::zero());
  return LVecBase3i(_indices[index], _indices[index + 1], _indices[index + 2]);
}

/**
 *
 */
INLINE ostream &
operator << (ostream &out, const BulletTriangleMesh &obj) {
  obj.output(out);
  return out;
}
