// Filename: nodeChain.I
// Created by:  drose (25Feb02)
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
//     Function: NodeChain::Default Constructor
//       Access: Published
//  Description: This constructs an empty NodeChain with no nodes.  It
//               cannot be extended, since you cannot add nodes without
//               first specifying the top node.  Use the constructor
//               that receives a node if you ever want to do anything
//               with this chain.
////////////////////////////////////////////////////////////////////
INLINE NodeChain::
NodeChain() :
  _error_type(ET_ok)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::Constructor
//       Access: Published
//  Description: This constructs an empty NodeChain with a single node.
//               This chain may now be extended by repeatedly calling
//               push_back() with each node below that node in
//               sequence.
//
//               If the Node pointer is NULL, this quietly creates an
//               empty NodeChain.
////////////////////////////////////////////////////////////////////
INLINE NodeChain::
NodeChain(PandaNode *top_node) :
  _error_type(ET_ok)
{
  if (top_node != (PandaNode *)NULL) {
    _head = top_node->get_generic_component();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NodeChain::
NodeChain(const NodeChain &copy) :
  _head(copy._head),
  _error_type(copy._error_type)
{
  uncollapse_head();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void NodeChain::
operator = (const NodeChain &copy) {
  _head = copy._head;
  _error_type = copy._error_type;
  uncollapse_head();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::not_found named constructor
//       Access: Published, Static
//  Description: Creates a NodeChain with the ET_not_found error type
//               set.
////////////////////////////////////////////////////////////////////
INLINE NodeChain NodeChain::
not_found() {
  NodeChain result;
  result._error_type = ET_not_found;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::removed named constructor
//       Access: Published, Static
//  Description: Creates a NodeChain with the ET_removed error type
//               set.
////////////////////////////////////////////////////////////////////
INLINE NodeChain NodeChain::
removed() {
  NodeChain result;
  result._error_type = ET_removed;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::fail named constructor
//       Access: Published, Static
//  Description: Creates a NodeChain with the ET_fail error type
//               set.
////////////////////////////////////////////////////////////////////
INLINE NodeChain NodeChain::
fail() {
  NodeChain result;
  result._error_type = ET_fail;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::is_empty
//       Access: Published
//  Description: Returns true if the NodeChain contains no nodes.
////////////////////////////////////////////////////////////////////
INLINE bool NodeChain::
is_empty() const {
  return (_head == (NodeChainComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::is_singleton
//       Access: Published
//  Description: Returns true if the NodeChain contains exactly one
//               node.
////////////////////////////////////////////////////////////////////
INLINE bool NodeChain::
is_singleton() const {
  uncollapse_head();
  return (_head != (NodeChainComponent *)NULL && _head->is_top_node());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_error_type
//       Access: Published
//  Description: If is_empty() is true, this returns a code that
//               represents the reason why the NodeChain is empty.
////////////////////////////////////////////////////////////////////
INLINE NodeChain::ErrorType NodeChain::
get_error_type() const {
  return _error_type;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::node
//       Access: Published
//  Description: Returns the bottom node of the path, or NULL if the
//               path is empty.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *NodeChain::
node() const {
  if (is_empty()) {
    return (PandaNode *)NULL;
  }
  return _head->get_node();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_num_children
//       Access: Published
//  Description: Returns the number of children of the bottom node.
////////////////////////////////////////////////////////////////////
INLINE int NodeChain::
get_num_children() const {
  nassertr(!is_empty(), 0);
  return _head->get_node()->get_num_children();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_child
//       Access: Published
//  Description: Returns a NodeChain representing the nth child of the
//               bottom node.
////////////////////////////////////////////////////////////////////
INLINE NodeChain NodeChain::
get_child(int n) const {
  nassertr(n >= 0 && n < get_num_children(), NodeChain());
  NodeChain child;
  child._head = PandaNode::get_component(_head, _head->get_node()->get_child(n));
  return child;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::has_parent
//       Access: Published
//  Description: Returns true if the node at the bottom of the
//               NodeChain has a parent; i.e. the NodeChain contains at
//               least two nodes.
////////////////////////////////////////////////////////////////////
INLINE bool NodeChain::
has_parent() const {
  return !is_empty() && !is_singleton();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_parent
//       Access: Published
//  Description: Returns the NodeChain to the parent of the bottom
//               node: that is, this NodeChain, shortened by one node.
////////////////////////////////////////////////////////////////////
INLINE NodeChain NodeChain::
get_parent() const {
  nassertr(has_parent(), NodeChain::fail());
  NodeChain parent;
  parent._head = _head->get_next();
  return parent;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::attach_new_node
//       Access: Published
//  Description: Creates an ordinary PandaNode and attaches it below
//               the current NodeChain, returning a new NodeChain that
//               references it.
////////////////////////////////////////////////////////////////////
INLINE NodeChain NodeChain::
attach_new_node(const string &name, int sort) const {
  nassertr(verify_complete(), NodeChain::fail());
  nassertr(!is_empty(), *this);

  return attach_new_node(new PandaNode(name), sort);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_net_state
//       Access: Published
//  Description: Returns the net state on this node from the root.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderState) NodeChain::
get_net_state() const {
  uncollapse_head();
  return r_get_net_state(_head);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_net_transform
//       Access: Published
//  Description: Returns the net transform on this node from the root.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) NodeChain::
get_net_transform() const {
  uncollapse_head();
  return r_get_net_transform(_head);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_mat
//       Access: Published
//  Description: Returns the matrix that describes the coordinate
//               space of this node, relative to the other
//               node's coordinate space.  If either NodeChain is
//               empty, it is taken to indicate the top of the graph.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &NodeChain::
get_mat(const NodeChain &other) const {
  CPT(TransformState) transform = get_rel_transform(other);
  // We can safely assume the transform won't go away when the
  // function returns, since its reference count is also held in the
  // cache.  This assumption allows us to return a reference to the
  // matrix, instead of having to return a matrix on the stack.
  nassertr(transform->get_ref_count() > 1, LMatrix4f::ident_mat());
  return transform->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::operator ==
//       Access: Published
//  Description: Returns true if the two chains are equivalent; that
//               is, if they contain the same list of nodes in the same
//               order.
////////////////////////////////////////////////////////////////////
INLINE bool NodeChain::
operator == (const NodeChain &other) const {
  return (compare_to(other) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::operator !=
//       Access: Published
//  Description: Returns true if the two chains are not equivalent.
////////////////////////////////////////////////////////////////////
INLINE bool NodeChain::
operator != (const NodeChain &other) const {
  return (compare_to(other) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::operator <
//       Access: Published
//  Description: Returns true if this NodeChain sorts before the other
//               one, false otherwise.  The sorting order of two
//               nonequivalent NodeChains is consistent but undefined,
//               and is useful only for storing NodeChains in a sorted
//               container like an STL set.
////////////////////////////////////////////////////////////////////
INLINE bool NodeChain::
operator < (const NodeChain &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this NodeChain
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
//
//               Two NodeChains are considered equivalent if they
//               consist of exactly the same list of nodes in the same
//               order.  Otherwise, they are different; different
//               NodeChains will be ranked in a consistent but
//               undefined ordering; the ordering is useful only for
//               placing the NodeChains in a sorted container like an
//               STL set.
////////////////////////////////////////////////////////////////////
INLINE int NodeChain::
compare_to(const NodeChain &other) const {
  return r_compare_to(_head, other._head);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::output
//       Access: Published
//  Description: Writes a sensible description of the NodeChain to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void NodeChain::
output(ostream &out) const {
  if (_head == (NodeChainComponent *)NULL) {
    out << "(empty)";
  } else {
    r_output(out, _head);
  }
}

INLINE ostream &operator << (ostream &out, const NodeChain &node_chain) {
  node_chain.output(out);
  return out;
}

