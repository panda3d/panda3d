// Filename: nurbsCurveEvaluator.I
// Created by:  drose (05Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::set_order
//       Access: Published
//  Description: Sets the order of the curve.  This resets the knot
//               vector to the default knot vector for the number of
//               vertices.
//
//               The order must be 1, 2, 3, or 4, and the value is one
//               more than the degree of the curve.
////////////////////////////////////////////////////////////////////
INLINE void NurbsCurveEvaluator::
set_order(int order) {
  _order = order;
  _knots_dirty = true;
  _basis_dirty = true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::get_order
//       Access: Published
//  Description: Returns the order of the curve as set by a previous
//               call to set_order().
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveEvaluator::
get_order() const {
  return _order;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::get_num_vertices
//       Access: Published
//  Description: Returns the number of control vertices in the curve.
//               This is the number passed to the last call to
//               reset().
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveEvaluator::
get_num_vertices() const {
  return (int)_vertices.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::set_vertex
//       Access: Published
//  Description: Sets the nth control vertex of the curve.
////////////////////////////////////////////////////////////////////
INLINE void NurbsCurveEvaluator::
set_vertex(int i, const LVecBase4f &vertex) {
  nassertv(i >= 0 && i < (int)_vertices.size());
  _vertices[i].set_vertex(vertex);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::set_vertex
//       Access: Published
//  Description: Sets the nth control vertex of the curve.
////////////////////////////////////////////////////////////////////
INLINE void NurbsCurveEvaluator::
set_vertex(int i, const LVecBase3f &vertex, float weight) {
  nassertv(i >= 0 && i < (int)_vertices.size());
  _vertices[i].set_vertex(LVecBase4f(vertex[0] * weight, vertex[1] * weight, vertex[2] * weight, weight));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::get_vertex
//       Access: Published
//  Description: Returns the nth control vertex of the curve, relative
//               to its indicated coordinate space.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4f &NurbsCurveEvaluator::
get_vertex(int i) const {
  nassertr(i >= 0 && i < (int)_vertices.size(), LVecBase4f::zero());
  return _vertices[i].get_vertex();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::set_vertex_space
//       Access: Published
//  Description: Sets the coordinate space of the nth control vertex.
//               If this is not specified, or is set to an empty
//               NodePath, the nth control vertex is deemed to be in
//               the coordinate space passed to evaluate().
//
//               This specifies the space as a fixed NodePath, which
//               is always the same NodePath.  Also see setting the
//               space as a path string, which can specify a different
//               NodePath for different instances of the curve.
////////////////////////////////////////////////////////////////////
INLINE void NurbsCurveEvaluator::
set_vertex_space(int i, const NodePath &space) {
  nassertv(i >= 0 && i < (int)_vertices.size());
  _vertices[i].set_space(space);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::set_vertex_space
//       Access: Published
//  Description: Sets the coordinate space of the nth control vertex.
//               If this is not specified, or is set to an empty
//               string, the nth control vertex is deemed to be in
//               the coordinate space passed to evaluate().
//
//               This specifies the space as a string, which describes
//               the path to find the node relative to the rel_to
//               NodePath when the curve is evaluated.
////////////////////////////////////////////////////////////////////
INLINE void NurbsCurveEvaluator::
set_vertex_space(int i, const string &space) {
  nassertv(i >= 0 && i < (int)_vertices.size());
  _vertices[i].set_space(space);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveEvaluator::get_num_knots
//       Access: Published
//  Description: Returns the number of knot values in the curve.  This
//               is based on the number of vertices and the order.
////////////////////////////////////////////////////////////////////
INLINE int NurbsCurveEvaluator::
get_num_knots() const {
  return (int)_vertices.size() + _order;
}
