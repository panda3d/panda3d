// Filename: triangulator.I
// Created by:  drose (18Jan07)
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
//     Function: Triangulator::add_vertex
//       Access: Published
//  Description: Adds a new vertex to the vertex pool.  Returns the
//               vertex index number.
////////////////////////////////////////////////////////////////////
INLINE int Triangulator::
add_vertex(double x, double y) {
  return add_vertex(LPoint2d(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: Triangulator::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertices in the pool.  Note
//               that the Triangulator might append new vertices, in
//               addition to those added by the user, if any of the
//               polygon is self-intersecting, or if any of the holes
//               intersect some part of the polygon edges.
////////////////////////////////////////////////////////////////////
INLINE int Triangulator::
get_num_vertices() const {
  return _vertices.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Triangulator::get_vertex
//       Access: Published
//  Description: Returns the nth vertex.
////////////////////////////////////////////////////////////////////
INLINE const LPoint2d &Triangulator::
get_vertex(int n) const {
  nassertr(n >= 0 && n < (int)_vertices.size(), LPoint2d::zero());
  return _vertices[n];
}

////////////////////////////////////////////////////////////////////
//     Function: Triangulator::is_left_winding
//       Access: Published
//  Description: Returns true if the polygon vertices are listed in
//               counterclockwise order, or false if they appear to be
//               listed in clockwise order.
////////////////////////////////////////////////////////////////////
INLINE bool Triangulator::
is_left_winding() const {
  return check_left_winding(_polygon);
}

////////////////////////////////////////////////////////////////////
//     Function: Triangulator::Triangle::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Triangulator::Triangle::
Triangle(Triangulator *t, int v0, int v1, int v2) :
  _v0(t->vert[v0].user_i), 
  _v1(t->vert[v1].user_i),
  _v2(t->vert[v2].user_i)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Triangulator::segment_t::Default Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Triangulator::segment_t::
segment_t() {
}

////////////////////////////////////////////////////////////////////
//     Function: Triangulator::segment_t::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Triangulator::segment_t::
segment_t(Triangulator *t, int v0_i, int v1_i, int prev, int next) :
  is_inserted(false),
  root0(0), root1(0),
  next(next),
  prev(prev),
  v0_i(v0_i)
{
  v0.x = t->_vertices[v0_i][0];
  v0.y = t->_vertices[v0_i][1];

  v1.x = t->_vertices[v1_i][0];
  v1.y = t->_vertices[v1_i][1];
}
