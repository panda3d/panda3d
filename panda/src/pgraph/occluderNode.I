// Filename: occluderNode.I
// Created by:  jenes (11Mar11)
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
//     Function: OccluderNode::set_vertices
//       Access: Published
//  Description: Replaces the four vertices of the occluder polygon.
//               The vertices should be defined in a counterclockwise
//               orientation when looking at the face of the occluder.
////////////////////////////////////////////////////////////////////
INLINE void OccluderNode::
set_vertices(const LPoint3f &v0, const LPoint3f &v1,
             const LPoint3f &v2, const LPoint3f &v3) {
  _vertices.clear();
  _vertices.reserve(4);
  _vertices.push_back(v0);
  _vertices.push_back(v1);
  _vertices.push_back(v2);
  _vertices.push_back(v3);
}

////////////////////////////////////////////////////////////////////
//     Function: OccluderNode::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertices in the occluder
//               polygon.  This should always return 4.
////////////////////////////////////////////////////////////////////
INLINE int OccluderNode::
get_num_vertices() const {
  return _vertices.size();
}

////////////////////////////////////////////////////////////////////
//     Function: OccluderNode::get_vertex
//       Access: Published
//  Description: Returns the nth vertex of the occluder polygon.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &OccluderNode::
get_vertex(int n) const {
  nassertr(n >= 0 && n < (int)_vertices.size(), LPoint3f::zero());
  return _vertices[n];
}
