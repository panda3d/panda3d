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
set_vertices(const LPoint3 &v0, const LPoint3 &v1,
             const LPoint3 &v2, const LPoint3 &v3) {
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
INLINE const LPoint3 &OccluderNode::
get_vertex(int n) const {
  nassertr(n >= 0 && n < (int)_vertices.size(), LPoint3::zero());
  return _vertices[n];
}

////////////////////////////////////////////////////////////////////
//     Function: OccluderNode::set_double_sided
//       Access: Published
//  Description: If true, the back-face will also be used to occlude
////////////////////////////////////////////////////////////////////
INLINE void OccluderNode::set_double_sided(bool value) {
  _double_sided = value;
}

////////////////////////////////////////////////////////////////////
//     Function: OccluderNode::is_double_sided
//       Access: Published
//  Description: Is this occluder double-sided
////////////////////////////////////////////////////////////////////
INLINE bool OccluderNode::is_double_sided() {
  return _double_sided;
}

////////////////////////////////////////////////////////////////////
//     Function: OccluderNode::set_min_coverage
//       Access: Published
//  Description: Minimum screen coverage needed before occluder used.
//               Range should be 0 to 1. For example, setting to 0.2
//               would mean that the occluder needs to cover 20% of
//               the screen to be considered.
////////////////////////////////////////////////////////////////////
INLINE void OccluderNode::set_min_coverage(PN_stdfloat value) {
  _min_coverage = value;
}

////////////////////////////////////////////////////////////////////
//     Function: OccluderNode::get_min_coverage
//       Access: Published
//  Description: Returns the minimum screen coverage.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat OccluderNode::get_min_coverage() {
  return _min_coverage;
}
