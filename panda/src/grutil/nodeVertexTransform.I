// Filename: nodeVertexTransform.I
// Created by:  drose (22eb07)
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
//     Function: NodeVertexTransform::get_node
//       Access: Published
//  Description: Returns the PandaNode whose transform supplies this
//               object.
////////////////////////////////////////////////////////////////////
INLINE const PandaNode *NodeVertexTransform::
get_node() const {
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeVertexTransform::get_prev
//       Access: Published
//  Description: Returns the VertexTransform object whose matrix will
//               be composed with the result of this node's transform.
////////////////////////////////////////////////////////////////////
INLINE const VertexTransform *NodeVertexTransform::
get_prev() const {
  return _prev;
}
