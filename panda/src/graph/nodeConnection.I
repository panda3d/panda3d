// Filename: nodeConnection.I
// Created by:  drose (07May01)
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
//     Function: NodeConnection::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeConnection::
NodeConnection(TypeHandle graph_type) : _graph_type(graph_type) {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeConnection::
~NodeConnection() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::Copy Constructor
//       Access: Private
//  Description: The copy constructor is private.  Copying
//               NodeConnections is not a good idea.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeConnection::
NodeConnection(const NodeConnection &copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::Copy Assignment Operator
//       Access: Private
//  Description: The assignment constructor is private.  Copying
//               NodeConnections is not a good idea.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeConnection::
operator = (const NodeConnection &copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::is_empty
//       Access: Public
//  Description: Returns true if the NodeConnection has no arcs, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeConnection::
is_empty() const {
  return _up.empty() && _down.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::get_graph_type
//       Access: Public
//  Description: Returns the type of graph represented by this
//               NodeConnection.  A node can coexist simultaneously in
//               several different graphs of different types; each
//               type requires a NodeConnection object to be stored
//               with the node.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH TypeHandle NodeConnection::
get_graph_type() const {
  return _graph_type;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::set_graph_type
//       Access: Public
//  Description: Changes the type of graph represented by this
//               NodeConnection.  It is legal to do this only when the
//               NodeConnection is empty, i.e. is_empty() returns
//               true.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeConnection::
set_graph_type(TypeHandle graph_type) {
  nassertv(is_empty());
  _graph_type = graph_type;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::get_up
//       Access: Public
//  Description: Returns the list of arcs pointing to the node's
//               parent(s).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH UpRelationPointers &NodeConnection::
get_up() {
  return _up;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::get_up
//       Access: Public
//  Description: Returns the list of arcs pointing to the node's
//               parent(s).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH const UpRelationPointers &NodeConnection::
get_up() const {
  return _up;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::get_down
//       Access: Public
//  Description: Returns the list of arcs pointing to the node's
//               child(ren).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH DownRelationPointers &NodeConnection::
get_down() {
  return _down;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeConnection::get_down
//       Access: Public
//  Description: Returns the list of arcs pointing to the node's
//               child(ren).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH const DownRelationPointers &NodeConnection::
get_down() const {
  return _down;
}
