// Filename: nodeVertexTransform.I
// Created by:  drose (22Feb07)
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
