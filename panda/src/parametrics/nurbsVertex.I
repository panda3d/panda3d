// Filename: nurbsVertex.I
// Created by:  drose (04Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NurbsVertex::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NurbsVertex::
NurbsVertex() {
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsVertex::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NurbsVertex::
NurbsVertex(const NurbsVertex &copy) :
  _vertex(copy._vertex),
  _space(copy._space),
  _space_path(copy._space_path)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsVertex::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NurbsVertex::
operator = (const NurbsVertex &copy) {
  _vertex = copy._vertex;
  _space = copy._space;
  _space_path = copy._space_path;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsVertex::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NurbsVertex::
~NurbsVertex() {
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsVertex::set_vertex
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NurbsVertex::
set_vertex(const LVecBase4f &vertex) {
  _vertex = vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsVertex::get_vertex
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4f &NurbsVertex::
get_vertex() const {
  return _vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsVertex::set_space
//       Access: Public
//  Description: Sets the space of this vertex as a fixed NodePath.
////////////////////////////////////////////////////////////////////
INLINE void NurbsVertex::
set_space(const NodePath &space) {
  _space = space;
  _space_path = string();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsVertex::set_space
//       Access: Public
//  Description: Sets the space of this vertex as a relative path from
//               the rel_to node.
////////////////////////////////////////////////////////////////////
INLINE void NurbsVertex::
set_space(const string &space) {
  _space = NodePath();
  _space_path = space;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsVertex::get_space
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NodePath NurbsVertex::
get_space(const NodePath &rel_to) const {
  if (_space_path.empty()) {
    return _space;
  } else {
    return rel_to.find(_space_path);
  }
}
