// Filename: node.I
// Created by:  drose (26Oct98)
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

#include <notify.h>


////////////////////////////////////////////////////////////////////
//     Function: Node::find_connection
//       Access: Public
//  Description: Returns a const reference to the NodeConnection of
//               the indicated graph type, if one exists, or a const
//               reference to an empty NodeConnection if a connection
//               of the indicated type does not exist.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH const NodeConnection &Node::
find_connection(TypeHandle graph_type) const {
  if (_connections[0].get_graph_type() == graph_type) {
    return _connections[0];
  }
  if (max_node_graphs > 1) {
    return p_find_connection(graph_type);
  } else {
    return _empty_connection;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Node::update_connection
//       Access: Public
//  Description: Returns a non-const pointer to the NodeConnection of
//               the indicated graph type, if one exists.  If a
//               NodeConnection of the indicated type does not already
//               exist, creates one if possible and returns its
//               pointer.  Otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeConnection *Node::
update_connection(TypeHandle graph_type) {
  if (_connections[0].get_graph_type() == graph_type) {
    return &_connections[0];
  }
  if (max_node_graphs > 1) {
    return p_update_connection(graph_type);
  } else {
    return (NodeConnection *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: remove_child
//  Description: Finds the arc that connects the indicated nodes and
//               removes it.  Returns true if the arc was found and
//               removed, false if it did not exist.
////////////////////////////////////////////////////////////////////
EXPCL_PANDA INLINE_GRAPH bool
remove_child(Node *parent, Node *child, TypeHandle graph_type) {
  NodeRelation *arc = find_arc(parent, child, graph_type);
  if (arc != (NodeRelation *)NULL) {
    remove_arc(arc);
    return true;
  }
  return false;
}

EXPCL_PANDA INLINE_GRAPH ostream &
operator << (ostream &out, const Node &node) {
  node.output(out);
  return out;
}
