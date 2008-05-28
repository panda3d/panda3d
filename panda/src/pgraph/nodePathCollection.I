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
set_color(float r, float g, float b, float a, int priority) {
  set_color(Colorf(r, g, b, a), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::set_color_scale
//       Access: Published
//  Description: Applies color scales to all NodePaths in the
//               collection.  The existing color scale is replaced.
////////////////////////////////////////////////////////////////////
INLINE void NodePathCollection::
set_color_scale(float r, float g, float b, float a, int priority) {
  set_color_scale(LVecBase4f(r, g, b, a), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::compose_color_scale
//       Access: Published
//  Description: Applies color scales to all NodePaths in the
//               collection.  The existing color scale, if any, is
//               multiplied by the specified color scale.
////////////////////////////////////////////////////////////////////
INLINE void NodePathCollection::
compose_color_scale(float r, float g, float b, float a, int priority) {
  compose_color_scale(LVecBase4f(r, g, b, a), priority);
}
