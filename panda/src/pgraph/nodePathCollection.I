// Filename: nodePathCollection.I
// Created by:  drose (06Mar02)
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
//     Function: NodePathCollection::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NodePathCollection::
~NodePathCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::operator +=
//       Access: Published
//  Description: Appends the other list onto the end of this one.
////////////////////////////////////////////////////////////////////
INLINE void NodePathCollection::
operator += (const NodePathCollection &other) {
  add_paths_from(other);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::operator +
//       Access: Published
//  Description: Returns a NodePathCollection representing the
//               concatenation of the two lists.
////////////////////////////////////////////////////////////////////
INLINE NodePathCollection NodePathCollection::
operator + (const NodePathCollection &other) const {
  NodePathCollection a(*this);
  a += other;
  return a;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::append
//       Access: Published
//  Description: Adds a new NodePath to the collection.  This method
//               duplicates the add_path() method; it is provided to
//               satisfy Python's naming convention.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
append(const NodePath &node_path) {
  add_path(node_path);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::extend
//       Access: Published
//  Description: Appends the other list onto the end of this one.
//               This method duplicates the += operator; it is
//               provided to satisfy Python's naming convention.
////////////////////////////////////////////////////////////////////
INLINE void NodePathCollection::
extend(const NodePathCollection &other) {
  operator += (other);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::ls
//       Access: Published
//  Description: Lists all the nodes at and below each node in the
//               collection hierarchically.
////////////////////////////////////////////////////////////////////
INLINE void NodePathCollection::
ls() const {
  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::set_color
//       Access: Published
//  Description: Colors all NodePaths in the collection
////////////////////////////////////////////////////////////////////
INLINE void NodePathCollection::
set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a, int priority) {
  set_color(LColor(r, g, b, a), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::set_color_scale
//       Access: Published
//  Description: Applies color scales to all NodePaths in the
//               collection.  The existing color scale is replaced.
////////////////////////////////////////////////////////////////////
INLINE void NodePathCollection::
set_color_scale(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a, int priority) {
  set_color_scale(LVecBase4(r, g, b, a), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::compose_color_scale
//       Access: Published
//  Description: Applies color scales to all NodePaths in the
//               collection.  The existing color scale, if any, is
//               multiplied by the specified color scale.
////////////////////////////////////////////////////////////////////
INLINE void NodePathCollection::
compose_color_scale(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a, int priority) {
  compose_color_scale(LVecBase4(r, g, b, a), priority);
}
