// Filename: eggVertex.I
// Created by:  drose (16Jan99)
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
//     Function: EggVertex::get_pool
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggVertexPool *EggVertex::
get_pool() const {
  return _pool;
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::set_pos
//       Access: Public
//  Description: Sets the vertex position.  This variant sets the
//               vertex to a one-dimensional value.
////////////////////////////////////////////////////////////////////
INLINE void EggVertex::
set_pos(double pos) {
  _num_dimensions = 1;
  _pos.set(pos, 0.0, 0.0, 1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::set_pos
//       Access: Public
//  Description: Sets the vertex position.  This variant sets the
//               vertex to a two-dimensional value.
////////////////////////////////////////////////////////////////////
INLINE void EggVertex::
set_pos(const LPoint2d &pos) {
  _num_dimensions = 2;
  _pos.set(pos[0], pos[1], 0.0, 1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::set_pos
//       Access: Public
//  Description: Sets the vertex position.  This variant sets the
//               vertex to a three-dimensional value.
////////////////////////////////////////////////////////////////////
INLINE void EggVertex::
set_pos(const LPoint3d &pos) {
  _num_dimensions = 3;
  _pos.set(pos[0], pos[1], pos[2], 1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::set_pos
//       Access: Public
//  Description: Sets the vertex position.  This variant sets the
//               vertex to a four-dimensional value.
////////////////////////////////////////////////////////////////////
INLINE void EggVertex::
set_pos(const LPoint4d &pos) {
  _num_dimensions = 4;
  _pos = pos;
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::set_pos4
//       Access: Public
//  Description: This special flavor of set_pos() sets the vertex as a
//               four-component value, but does not change the set
//               number of dimensions.  It's handy for retrieving the
//               vertex position via get_pos4, manipulating it, then
//               storing it back again, without worrying about the
//               number of dimensions it actually had.
////////////////////////////////////////////////////////////////////
INLINE void EggVertex::
set_pos4(const LPoint4d &pos) {
  _pos = pos;
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::get_num_dimensions
//       Access: Public
//  Description: Returns the number of dimensions the vertex uses.
//               Usually this will be 3, but it may be 1, 2, 3, or 4.
////////////////////////////////////////////////////////////////////
INLINE int EggVertex::
get_num_dimensions() const {
  return _num_dimensions;
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::get_pos1
//       Access: Public
//  Description: Only valid if get_num_dimensions() returns 1.
//               Returns the position as a one-dimensional value.
////////////////////////////////////////////////////////////////////
INLINE double EggVertex::
get_pos1() const {
  nassertr(_num_dimensions == 1, 0.0);
  return _pos[0];
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::get_pos2
//       Access: Public
//  Description: Only valid if get_num_dimensions() returns 2.
//               Returns the position as a two-dimensional value.
////////////////////////////////////////////////////////////////////
INLINE LPoint2d EggVertex::
get_pos2() const {
  nassertr(_num_dimensions == 2, LPoint2d(0.0, 0.0));
  return LPoint2d(_pos[0], _pos[1]);
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::get_pos3
//       Access: Public
//  Description: Valid if get_num_dimensions() returns 3 or 4.
//               Returns the position as a three-dimensional value.
////////////////////////////////////////////////////////////////////
INLINE Vertexd EggVertex::
get_pos3() const {
  nassertr(_num_dimensions == 3 || _num_dimensions == 4,
           LPoint3d(0.0, 0.0, 0.0));
  return Vertexd(_pos[0] / _pos[3], _pos[1] / _pos[3], _pos[2] / _pos[3]);
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::get_pos4
//       Access: Public
//  Description: This is always valid, regardless of the value of
//               get_num_dimensions.  It returns the position as a
//               four-dimensional value.  If the pos has fewer than
//               four dimensions, this value represents the pos
//               extended into four-dimensional homogenous space,
//               e.g. by adding 1 as the fourth component.
////////////////////////////////////////////////////////////////////
INLINE LPoint4d EggVertex::
get_pos4() const {
  return _pos;
}


////////////////////////////////////////////////////////////////////
//     Function: EggVertex::get_index
//       Access: Public
//  Description: Returns the index number of the vertex within its
//               pool.
////////////////////////////////////////////////////////////////////
INLINE int EggVertex::
get_index() const {
  return _index;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertex::set_external_index
//       Access: Public
//  Description: Sets a special index number that is associated with
//               the EggVertex (but is not written to the egg file).
//               This number is not interpreted by any egg code; it is
//               simply maintained along with the vertex.  It *is*
//               used to differentiate otherwise identical vertices in
//               EggVertexPool::create_unique_vertex(), however.
//
//               The intention of this number is as an aid for file
//               converters, to associate an EggVertex back to the
//               index number of the original source vertex.
////////////////////////////////////////////////////////////////////
INLINE void EggVertex::
set_external_index(int external_index) {
  _external_index = external_index;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertex::get_external_index
//       Access: Public
//  Description: Returns the number set by set_external_index().  See
//               set_external_index().
////////////////////////////////////////////////////////////////////
INLINE int EggVertex::
get_external_index() const {
  return _external_index;
}






////////////////////////////////////////////////////////////////////
//     Function: UniqueEggVertices::Function operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool UniqueEggVertices::
operator ()(const EggVertex *v1, const EggVertex *v2) const {
  return v1->sorts_less_than(*v2);
}

