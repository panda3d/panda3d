// Filename: nodeRelation.I
// Created by:  drose (30Sep99)
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
//     Function: remove_arc
//  Description: Removes the arc from the graph, which incidentally
//               will also delete the arc unless there are some
//               additional outstanding pointers to it.
////////////////////////////////////////////////////////////////////
EXPCL_PANDA INLINE_GRAPH void
remove_arc(NodeRelation *arc) {
  PT(TypedWritableReferenceCount) hold_ptr = arc->detach();
  arc->_parent = NULL;
  arc->_child = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::output_transitions
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeRelation::
output_transitions(ostream &out) const {
  out << _transitions;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::write_transitions
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeRelation::
write_transitions(ostream &out, int indent_level) const {
  _transitions.write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_parent
//       Access: Public
//  Description: Returns the node above the arc in the scene graph.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH Node *NodeRelation::
get_parent() const {
  return _parent;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_child
//       Access: Public
//  Description: Returns the node below the arc in the scene graph.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH Node *NodeRelation::
get_child() const {
  return _child;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_sort
//       Access: Public
//  Description: Returns the sorting index of the arc.  This affects
//               its apparent position among its list of siblings.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int NodeRelation::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_graph_type
//       Access: Public
//  Description: Returns the type of graph this arc represents.  This
//               is normally the same value as get_type(), i.e. the
//               class type of the arc, but it may be set to any type.
//               A node may simultaneously have many arcs of different
//               types.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH TypeHandle NodeRelation::
get_graph_type() const {
  return _graph_type;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::is_attached
//       Access: Public
//  Description: Returns true if this arc is attached to the scene
//               graph, false otherwise.  Normally, an arc should
//               always be attached; generally you will only see an
//               unattached arc if the arc (or some parent of the arc)
//               has been disconnected from all nodes via
//               remove_arc().
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeRelation::
is_attached() const {
  return _attached;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::change_parent
//       Access: Public
//  Description: Changes the parent of the arc to a different node.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeRelation::
change_parent(Node *parent) {
  PT(TypedWritableReferenceCount) hold_ptr = detach();
  _parent = parent;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::change_parent
//       Access: Public
//  Description: Changes the parent of the arc to a different node,
//               simultaneously changing the sort index.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeRelation::
change_parent(Node *parent, int sort) {
  PT(TypedWritableReferenceCount) hold_ptr = detach();
  _parent = parent;
  _sort = sort;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::change_child
//       Access: Public
//  Description: Changes the child of the arc to a different node.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeRelation::
change_child(Node *child) {
  PT(TypedWritableReferenceCount) hold_ptr = detach();
  _child = child;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::change_parent_and_child
//       Access: Public
//  Description: Simultaneously moves both the parent and child of the
//               arc to different nodes.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeRelation::
change_parent_and_child(Node *parent, Node *child) {
  PT(TypedWritableReferenceCount) hold_ptr = detach();
  _parent = parent;
  _child = child;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::set_sort
//       Access: Public
//  Description: Changes the sorting index of the arc.  This affects
//               its apparent position among its list of siblings.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeRelation::
set_sort(int sort) {
  PT(TypedWritableReferenceCount) hold_ptr = detach();
  _sort = sort;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::set_graph_type
//       Access: Public
//  Description: Changes the graph type of the arc.  This moves the
//               arc into a totally different graph; it may now be
//               detected only by a special traversal.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeRelation::
set_graph_type(TypeHandle graph_type) {
  PT(TypedWritableReferenceCount) hold_ptr = detach();
  _graph_type = graph_type;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::set_transition
//       Access: Public
//  Description: This flavor of set_transition() accepts a pointer to
//               a NodeTransition only.  It infers the type of the
//               NodeTransition from the pointer.  However, it is not
//               valid to pass a NULL pointer to this flavor of
//               set_transition; if the pointer might be NULL, use the
//               above flavor instead (or just call clear_transition).
//
//               The return value is a pointer to the *previous*
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeTransition) NodeRelation::
set_transition(NodeTransition *trans) {
  nassertr(trans != (NodeTransition *)NULL, NULL);
  nassertr(trans->get_handle() != TypeHandle::none(), NULL);
  return set_transition(trans->get_handle(), trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::set_transition
//       Access: Public
//  Description: This is a convenience flavor.  It automatically
//               applies the indicated priority to the given
//               transition before setting the transition.  Generally,
//               this will be used for creating override transitions
//               on-the-fly.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeTransition) NodeRelation::
set_transition(NodeTransition *trans, int priority) {
  nassertr(trans != (NodeTransition *)NULL, NULL);
  nassertr(trans->get_handle() != TypeHandle::none(), NULL);
  trans->set_priority(priority);
  return set_transition(trans->get_handle(), trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::has_transition
//       Access: Public
//  Description: Returns true if a transition associated with the
//               indicated handle has been stored on the arc (even if
//               it is the identity transition), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeRelation::
has_transition(TypeHandle handle) const {
  return _transitions.has_transition(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::has_any_transition
//       Access: Public
//  Description: Returns true if a any transition has been stored on
//               the arc (even if it is the identity transition), or
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeRelation::
has_any_transition() const {
  return !_transitions.is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_transition
//       Access: Public
//  Description: Returns the transition associated with the indicated
//               handle, or NULL if no such transition has been stored
//               on the arc.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransition *NodeRelation::
get_transition(TypeHandle handle) const {
  return _transitions.get_transition(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::compare_transitions_to
//       Access: Public
//  Description: Returns a negative number if the transitions on the
//               other arc sort before those on this arc, positive if
//               they sort after, or zero if the sets of transitions
//               are equivalent.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int NodeRelation::
compare_transitions_to(const NodeRelation *arc) const {
  return _transitions.compare_to(arc->_transitions);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_last_update
//       Access: Public
//  Description: Returns the sequence number associated with the last
//               time this arc was changed, for instance to change its
//               state or to reparent it.  The only sensible thing you
//               can do with this is store it and note whether it
//               increases.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH UpdateSeq NodeRelation::
get_last_update() const {
  return _last_update;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::create_typed_arc
//       Access: Public, Static
//  Description: Creates a new arc of the scene graph of the indicated
//               type, and immediately attaches it.  Returns NULL if
//               the type is unknown.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeRelation *NodeRelation::
create_typed_arc(TypeHandle type, Node *parent, Node *child, int sort) {
  NodeRelation *arc = get_factory().make_instance(type);
  if (arc != (NodeRelation *)NULL) {
    arc->_parent = parent;
    arc->_child = child;
    arc->_sort = sort;
    arc->attach();
  }
  return arc;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::register_with_factory
//       Access: Public, Static
//  Description: Registers the type NodeRelation with the factory.
//               This is only intended to be called at initialization
//               time; don't call it directly.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeRelation::
register_with_factory() {
  get_factory().register_factory(get_class_type(), make_arc);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_factory
//       Access: Protected, Static
//  Description: Returns a reference to the NodeRelation factory.
//               Creates one if this is the first time this function
//               has been called.  This is necessary to guarantee
//               ordering at static init time.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH Factory<NodeRelation> &NodeRelation::
get_factory() {
  if (_factory == (Factory<NodeRelation> *)NULL) {
    _factory = new Factory<NodeRelation>;
  }
  return *_factory;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_class_type
//       Access: Public, Static
//  Description: Returns the TypeHandle associated with this type.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH TypeHandle NodeRelation::
get_class_type() {
  return _type_handle;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_stashed_type
//       Access: Public, Static
//  Description: Returns a special TypeHandle that, by convention,
//               arcs that wish to remove themselves from normal
//               consideration during any traversal (including
//               rendering and collision traversals), as well as
//               removing themselves from the bounding volumes of
//               their parents, should set their graph_type to.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH TypeHandle NodeRelation::
get_stashed_type() {
  return _stashed_type_handle;
}

EXPCL_PANDA INLINE_GRAPH ostream &
operator << (ostream &out, const NodeRelation &arc) {
  arc.output(out);
  return out;
}

