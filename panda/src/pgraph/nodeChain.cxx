// Filename: nodeChain.cxx
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

#include "nodeChain.h"
#include "node.h"
#include "namedNode.h"
#include "config_pgraph.h"
#include "plist.h"

TypeHandle NodeChain::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_num_nodes
//       Access: Published
//  Description: Returns the number of nodes in the path.
////////////////////////////////////////////////////////////////////
int NodeChain::
get_num_nodes() const {
  if (is_empty()) {
    return 0;
  }
  uncollapse_head();
  return _head->get_length();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_node
//       Access: Published
//  Description: Returns the nth node of the path, where 0 is the
//               bottom node and get_num_nodes() - 1 is the top node.
//               This requires iterating through the path.
////////////////////////////////////////////////////////////////////
PandaNode *NodeChain::
get_node(int index) const {
  nassertr(index >= 0 && index < get_num_nodes(), NULL);

  uncollapse_head();
  NodeChainComponent *comp = _head;
  while (index > 0) {
    // If this assertion fails, the index was out of range; the
    // component's length must have been invalid.
    nassertr(comp != (NodeChainComponent *)NULL, NULL);
    comp = comp->get_next();
    index--;
  }

  // If this assertion fails, the index was out of range; the
  // component's length must have been invalid.
  nassertr(comp != (NodeChainComponent *)NULL, NULL);
  return comp->get_node();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_top_node
//       Access: Published
//  Description: Returns the top node of the path, or NULL if the path
//               is empty.  This requires iterating through the path.
////////////////////////////////////////////////////////////////////
PandaNode *NodeChain::
get_top_node() const {
  if (is_empty()) {
    return (PandaNode *)NULL;
  }

  uncollapse_head();
  NodeChainComponent *comp = _head;
  while (!comp->is_top_node()) {
    comp = comp->get_next();
    nassertr(comp != (NodeChainComponent *)NULL, NULL);
  }

  return comp->get_node();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::reparent_to
//       Access: Published
//  Description: Removes the bottom node of the NodeChain from its
//               current parent and attaches it to the bottom node of
//               the indicated NodeChain.
////////////////////////////////////////////////////////////////////
void NodeChain::
reparent_to(const NodeChain &other, int sort) {
  nassertv(other.verify_complete());
  nassertv_always(!is_empty());
  nassertv_always(!other.is_empty());

  uncollapse_head();
  other.uncollapse_head();
  PandaNode::reparent(other._head, _head, sort);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::wrt_reparent_to
//       Access: Published
//  Description: This functions identically to reparent_to(), except
//               the transform on this node is also adjusted so that
//               the node remains in the same place in world
//               coordinates, even if it is reparented into a
//               different coordinate system.
////////////////////////////////////////////////////////////////////
void NodeChain::
wrt_reparent_to(const NodeChain &other, int sort) {
  nassertv(other.verify_complete());
  nassertv_always(!is_empty());
  nassertv_always(!other.is_empty());

  //****
  /*
  LMatrix4f mat = get_mat(other);
  set_mat(mat);
  */

  reparent_to(other, sort);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::instance_to
//       Access: Published
//  Description: Adds the bottom node of the NodeChain as a child of
//               the bottom node of the indicated other NodeChain.  Any
//               other parent-child relations of the node are
//               unchanged; in particular, the node is not removed
//               from its existing parent, if any.
//
//               If the node already had an existing parent, this
//               method will create a new instance of the node within
//               the scene graph.
//
//               This does not change the NodeChain itself, but does
//               return a new NodeChain that reflects the new instance
//               node.
////////////////////////////////////////////////////////////////////
NodeChain NodeChain::
instance_to(const NodeChain &other, int sort) const {
  nassertr(verify_complete(), NodeChain::fail());
  nassertr(!is_empty(), NodeChain::fail());
  nassertr(!other.is_empty(), NodeChain::fail());

  uncollapse_head();
  other.uncollapse_head();

  NodeChain new_instance;
  new_instance._head = PandaNode::attach(other._head, node(), sort);

  return new_instance;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::copy_to
//       Access: Published
//  Description: Functions exactly like instance_to(), except a deep
//               copy is made of the bottom node and all of its
//               descendents, which is then parented to the indicated
//               node.  A NodeChain to the newly created copy is
//               returned.
//
//               Certain kinds of nodes may not be copied; if one of
//               these is encountered, the node will be "copied" as
//               the nearest copyable base class.  For instance, a
//               Camera node in the graph will become a simple
//               NamedNode.
////////////////////////////////////////////////////////////////////
NodeChain NodeChain::
copy_to(const NodeChain &other, int sort) const {
  //*****
  return instance_to(other, sort);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::attach_new_node
//       Access: Published
//  Description: Attaches a new node, with or without existing
//               parents, to the scene graph below the bottom node of
//               this NodeChain.  This is the preferred way to add
//               nodes to the graph.
//
//               This does *not* automatically extend the current
//               NodeChain to reflect the attachment; however, a
//               NodeChain that does reflect this extension is
//               returned.
////////////////////////////////////////////////////////////////////
NodeChain NodeChain::
attach_new_node(PandaNode *node, int sort) const {
  nassertr(verify_complete(), NodeChain::fail());
  nassertr(!is_empty(), NodeChain());
  nassertr(node != (PandaNode *)NULL, NodeChain());

  uncollapse_head();
  NodeChain new_path(*this);
  new_path._head = PandaNode::attach(_head, node, sort);
  return new_path;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::remove_node
//       Access: Published
//  Description: Disconnects the bottom node from the scene graph.
//               This will also delete the node if there are no other
//               pointers to it.
//
//               Normally, this should be called only when you are
//               really done with the node.  If you want to remove a
//               node from the scene graph but keep it around for
//               later, you should probably use reparent_to() and put
//               it under a holding node instead.
//
//               After the node is removed, the NodeChain will have
//               been cleared.
////////////////////////////////////////////////////////////////////
void NodeChain::
remove_node() {
  nassertv(_error_type != ET_not_found);
  if (is_empty()) {
    // If we have no arcs (maybe we were already removed), quietly do
    // nothing except to ensure the NodeChain is clear.
    (*this) = NodeChain::removed();
    return;
  }

  uncollapse_head();
  PandaNode::detach(_head);

  (*this) = NodeChain::removed();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_rel_state
//       Access: Published
//  Description: Returns the state changes that must be made to
//               transition from the render state of this node to the
//               render state of the other node.
////////////////////////////////////////////////////////////////////
CPT(RenderState) NodeChain::
get_rel_state(const NodeChain &other) const {
  if (is_empty()) {
    return other.get_net_state();
  }
  if (other.is_empty()) {
    return get_net_state()->invert_compose(RenderState::make_empty());
  }
    
  nassertr(verify_complete(), RenderState::make_empty());
  nassertr(other.verify_complete(), RenderState::make_empty());

  int a_count, b_count;
  find_common_ancestor(*this, other, a_count, b_count);

  CPT(RenderState) a_state = r_get_partial_state(_head, a_count);
  CPT(RenderState) b_state = r_get_partial_state(other._head, b_count);
  return a_state->invert_compose(b_state);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::get_rel_transform
//       Access: Published
//  Description: Returns the relative transform from this node to the
//               other node; i.e. the transformation of the other node
//               as seen from this node.
////////////////////////////////////////////////////////////////////
CPT(TransformState) NodeChain::
get_rel_transform(const NodeChain &other) const {
  if (is_empty()) {
    return other.get_net_transform();
  }
  if (other.is_empty()) {
    return get_net_transform()->invert_compose(TransformState::make_identity());
  }
    
  nassertr(verify_complete(), TransformState::make_identity());
  nassertr(other.verify_complete(), TransformState::make_identity());

  int a_count, b_count;
  find_common_ancestor(*this, other, a_count, b_count);

  CPT(TransformState) a_transform = r_get_partial_transform(_head, a_count);
  CPT(TransformState) b_transform = r_get_partial_transform(other._head, b_count);
  return a_transform->invert_compose(b_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::verify_complete
//       Access: Published
//  Description: Returns true if all of the nodes described in the
//               NodeChain are connected *and* the top node is the top
//               of the graph, or false otherwise.
////////////////////////////////////////////////////////////////////
bool NodeChain::
verify_complete() const {
  if (is_empty()) {
    return true;
  }

  uncollapse_head();
  const NodeChainComponent *comp = _head;
  nassertr(comp != (const NodeChainComponent *)NULL, false);

  PandaNode *node = comp->get_node();
  nassertr(node != (const PandaNode *)NULL, false);
  int length = comp->get_length();

  comp = comp->get_next();
  length--;
  while (comp != (const NodeChainComponent *)NULL) {
    PandaNode *next_node = comp->get_node();
    nassertr(next_node != (const PandaNode *)NULL, false);

    if (next_node->find_child(node) < 0) {
      return false;
    }

    if (comp->get_length() != length) {
      return false;
    }

    node = next_node;
    comp = comp->get_next();
    length--;
  }

  // Now that we've reached the top, we should have no parents.
  return length == 0 && node->get_num_parents() == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::uncollapse_head
//       Access: Private
//  Description: Quietly and transparently uncollapses the _head
//               pointer if it needs it.
////////////////////////////////////////////////////////////////////
void NodeChain::
uncollapse_head() const {
  if (_head != (NodeChainComponent *)NULL && _head->is_collapsed()) {
    ((NodeChain *)this)->_head = _head->uncollapse();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::find_common_ancestor
//       Access: Private, Static
//  Description: Walks up from both NodeChains to find the first node
//               that both have in common, if any.  Fills a_count and
//               b_count with the number of nodes below the common
//               node in each chain.
////////////////////////////////////////////////////////////////////
void NodeChain::
find_common_ancestor(const NodeChain &a, const NodeChain &b,
                     int &a_count, int &b_count) {
  nassertv(!a.is_empty() && !b.is_empty());
  a.uncollapse_head();
  b.uncollapse_head();

  NodeChainComponent *ac = a._head;
  NodeChainComponent *bc = b._head;
  a_count = 0;
  b_count = 0;

  // Shorten up the longer one until they are the same length.
  while (ac->get_length() > bc->get_length()) {
    nassertv(ac != (NodeChainComponent *)NULL);
    ac = ac->get_next();
    a_count++;
  }
  while (bc->get_length() > ac->get_length()) {
    nassertv(bc != (NodeChainComponent *)NULL);
    bc = bc->get_next();
    b_count++;
  }

  // Now shorten them both up until we reach the same component.
  while (ac != bc) {
    // These shouldn't go to NULL unless they both go there together. 
    nassertv(ac != (NodeChainComponent *)NULL);
    nassertv(bc != (NodeChainComponent *)NULL);
    ac = ac->get_next();
    a_count++;
    bc = bc->get_next();
    b_count++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::r_get_net_state
//       Access: Private
//  Description: Recursively determines the net state chnages to the
//               indicated component node from the root of the graph.
////////////////////////////////////////////////////////////////////
CPT(RenderState) NodeChain::
r_get_net_state(NodeChainComponent *comp) const {
  if (comp == (NodeChainComponent *)NULL) {
    return RenderState::make_empty();
  } else {
    CPT(RenderState) state = comp->get_node()->get_state();
    return r_get_net_state(comp->get_next())->compose(state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::r_get_partial_state
//       Access: Private
//  Description: Recursively determines the net state changes to the
//               indicated component node from the nth node above it.
////////////////////////////////////////////////////////////////////
CPT(RenderState) NodeChain::
r_get_partial_state(NodeChainComponent *comp, int n) const {
  nassertr(comp != (NodeChainComponent *)NULL, RenderState::make_empty());
  if (n == 0) {
    return RenderState::make_empty();
  } else {
    CPT(RenderState) state = comp->get_node()->get_state();
    return r_get_partial_state(comp->get_next(), n - 1)->compose(state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::r_get_net_transform
//       Access: Private
//  Description: Recursively determines the net transform to the
//               indicated component node from the root of the graph.
////////////////////////////////////////////////////////////////////
CPT(TransformState) NodeChain::
r_get_net_transform(NodeChainComponent *comp) const {
  if (comp == (NodeChainComponent *)NULL) {
    return TransformState::make_identity();
  } else {
    CPT(TransformState) transform = comp->get_node()->get_transform();
    return r_get_net_transform(comp->get_next())->compose(transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::r_get_partial_transform
//       Access: Private
//  Description: Recursively determines the net transform to the
//               indicated component node from the nth node above it.
////////////////////////////////////////////////////////////////////
CPT(TransformState) NodeChain::
r_get_partial_transform(NodeChainComponent *comp, int n) const {
  nassertr(comp != (NodeChainComponent *)NULL, TransformState::make_identity());
  if (n == 0) {
    return TransformState::make_identity();
  } else {
    CPT(TransformState) transform = comp->get_node()->get_transform();
    return r_get_partial_transform(comp->get_next(), n - 1)->compose(transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::r_output
//       Access: Private
//  Description: The recursive implementation of output(), this writes
//               the names of each node component in order from
//               beginning to end, by first walking to the end of the
//               linked list and then outputting from there.
////////////////////////////////////////////////////////////////////
void NodeChain::
r_output(ostream &out, NodeChainComponent *comp) const {
  NodeChainComponent *next = comp->get_next();
  if (next != (NodeChainComponent *)NULL) {
    // This is not the head of the list; keep going up.
    r_output(out, next);
    out << "/";
  }

  // Now output this component.
  PandaNode *node = comp->get_node();
  if (node->has_name()) {
    out << node->get_name();
  } else {
    out << "+" << node->get_type();
  }
  out << "[" << comp->get_length() << "]";
}

////////////////////////////////////////////////////////////////////
//     Function: NodeChain::r_compare_to
//       Access: Private, Static
//  Description: The recursive implementation of compare_to().  Returns
//               < 0 if a sorts before b, > 0 if b sorts before a, or
//               == 0 if they are equivalent.
////////////////////////////////////////////////////////////////////
int NodeChain::
r_compare_to(const NodeChainComponent *a, const NodeChainComponent *b) {
  if (a == b) {
    return 0;

  } else if (a == (const NodeChainComponent *)NULL) {
    return -1;

  } else if (b == (const NodeChainComponent *)NULL) {
    return 1;

  } else if (a->get_node() != b->get_node()) {
    return a->get_node() - b->get_node();

  } else {
    return r_compare_to(a->get_next(), b->get_next());
  }
}
