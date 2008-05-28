// Filename: workingNodePath.I
// Created by:  drose (16Mar02)
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
//     Function: WorkingNodePath::Constructor
//       Access: Public
//  Description: Creates a WorkingNodePath that is the same as the
//               indicated NodePath.  This is generally used to begin
//               the traversal of a scene graph with the root
//               NodePath.
////////////////////////////////////////////////////////////////////
INLINE WorkingNodePath::
WorkingNodePath(const NodePath &start) {
  nassertv(!start.is_empty());
  _next = (WorkingNodePath *)NULL;
  _start = start._head;
  _node = start.node();
}

////////////////////////////////////////////////////////////////////
//     Function: WorkingNodePath::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE WorkingNodePath::
WorkingNodePath(const WorkingNodePath &copy) :
  _next(copy._next),
  _start(copy._start),
  _node(copy._node)
{
  nassertv(_next != (WorkingNodePath *)NULL ||
           _start != (NodePathComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: WorkingNodePath::Constructor
//       Access: Public
//  Description: Creates a WorkingNodePath that is the same as the
//               indicated WorkingNodePath, plus one node.  This is
//               generally used to continue the traversal to the next
//               node.
////////////////////////////////////////////////////////////////////
INLINE WorkingNodePath::
WorkingNodePath(const WorkingNodePath &parent, PandaNode *child) {
  _next = &parent;
  _start = (NodePathComponent *)NULL;
  _node = child;
  nassertv(_node != _next->_node);
}

////////////////////////////////////////////////////////////////////
//     Function: WorkingNodePath::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE WorkingNodePath::
~WorkingNodePath() {
}

////////////////////////////////////////////////////////////////////
//     Function: WorkingNodePath::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void WorkingNodePath::
operator = (const WorkingNodePath &copy) {
  _next = copy._next;
  _start = copy._start;
  _node = copy._node;

  nassertv(_next != (WorkingNodePath *)NULL ||
           _start != (NodePathComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: WorkingNodePath::get_node_path
//       Access: Public
//  Description: Constructs and returns an actual NodePath that
//               represents the same path we have just traversed.
////////////////////////////////////////////////////////////////////
INLINE NodePath WorkingNodePath::
get_node_path() const {
  NodePath result;
  result._head = r_get_node_path();
  nassertr(result._head != (NodePathComponent *)NULL, NodePath::fail());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: WorkingNodePath::node
//       Access: Public
//  Description: Returns the node traversed to so far.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *WorkingNodePath::
node() const {
  return _node;
}

INLINE ostream &
operator << (ostream &out, const WorkingNodePath &node_path) {
  node_path.output(out);
  return out;
}
