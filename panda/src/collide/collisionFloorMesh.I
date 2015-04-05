//: collisionPlane.I
// Created by:  drose (25Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::Default Constructor
//       Access: Protected
//  Description: This is only for the convenience of CollisionPolygon.
//               Normally, you should not attempt to create an
//               uninitialized CollisionPlane.
////////////////////////////////////////////////////////////////////
INLINE CollisionFloorMesh::
CollisionFloorMesh() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionFloorMesh::
CollisionFloorMesh(const CollisionFloorMesh &copy) :
  CollisionSolid(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::flush_level
//       Access: Public, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void CollisionFloorMesh::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::add_vertex
//       Access: Published
//  Description: store away a vertex to index against
////////////////////////////////////////////////////////////////////
INLINE void CollisionFloorMesh::
add_vertex(const LPoint3 &vert) {
  _vertices.push_back(vert);
}

INLINE unsigned int  CollisionFloorMesh::
get_num_vertices() const {
  return _vertices.size();
}

INLINE const LPoint3 &CollisionFloorMesh::
get_vertex(unsigned int index) const {
  return _vertices[index];
}

INLINE unsigned int CollisionFloorMesh::
get_num_triangles() const {
  return _triangles.size();
}

INLINE LPoint3i CollisionFloorMesh::
get_triangle(unsigned int index) const {
  CollisionFloorMesh::TriangleIndices tri = _triangles[index];
  return LPoint3i(tri.p1, tri.p2, tri.p3);
}
