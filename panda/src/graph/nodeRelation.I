// Filename: nodeRelation.I
// Created by:  drose (30Sep99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::Constructor
//       Access: Public
//  Description: Creates a new arc of the scene graph, and immediately
//               attaches it.
////////////////////////////////////////////////////////////////////
INLINE NodeRelation::
NodeRelation(Node *parent, Node *to, int sort, TypeHandle type) :
  _parent(parent), _child(to), _sort(sort), 
  _type(type), _num_transitions(0)
{
  _top_subtree = NULL;
  _attached = false;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::Constructor
//       Access: Protected
//  Description: Creates a new, unattached arc.  This constructor is
//               only intended for passing through the factory to
//               create an arc based on a particular type using
//               create_typed_arc(), below.  You shouldn't, in
//               general, attempt to create an unattached arc.
////////////////////////////////////////////////////////////////////
INLINE NodeRelation::
NodeRelation(TypeHandle type) :
  _type(type)
{
  _parent = NULL;
  _child = NULL;
  _sort = 0;
  _top_subtree = NULL;
  _attached = false;
  _num_transitions = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::Constructor
//       Access: Protected
//  Description: Creates a new, unattached arc.  This constructor is
//               only intended for being called in make_NodeRelation. 
//               You shouldn't, in general, attempt to create an 
//               unattached arc.
////////////////////////////////////////////////////////////////////
INLINE NodeRelation::
NodeRelation(void)
{
  _parent = NULL;
  _child = NULL;
  _sort = 0;
  _top_subtree = NULL;
  _attached = false;
  _type = get_class_type();
  _num_transitions = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: remove_arc
//  Description: Removes the arc from the graph, which incidentally
//               will also delete the arc unless there are some
//               additional outstanding pointers to it.
////////////////////////////////////////////////////////////////////
INLINE void
remove_arc(NodeRelation *arc) {
  PT(NodeRelation) hold_arc = arc->detach();
  arc->_parent = NULL;
  arc->_child = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::output_transitions
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodeRelation::
output_transitions(ostream &out) const {
  out << _transitions;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::write_transitions
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodeRelation::
write_transitions(ostream &out, int indent_level) const {
  _transitions.write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_parent
//       Access: Public
//  Description: Returns the node above the arc in the scene graph.
////////////////////////////////////////////////////////////////////
INLINE Node *NodeRelation::
get_parent() const {
  return _parent;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_child
//       Access: Public
//  Description: Returns the node below the arc in the scene graph.
////////////////////////////////////////////////////////////////////
INLINE Node *NodeRelation::
get_child() const {
  return _child;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::get_sort
//       Access: Public
//  Description: Returns the sorting index of the arc.  This affects
//               its apparent position among its list of siblings.
////////////////////////////////////////////////////////////////////
INLINE int NodeRelation::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::change_parent
//       Access: Public
//  Description: Changes the parent of the arc to a different node.
////////////////////////////////////////////////////////////////////
INLINE void NodeRelation::
change_parent(Node *parent) {
  PT(NodeRelation) hold_arc = detach();
  _parent = parent;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::change_parent
//       Access: Public
//  Description: Changes the parent of the arc to a different node,
//               simultaneously changing the sort index.
////////////////////////////////////////////////////////////////////
INLINE void NodeRelation::
change_parent(Node *parent, int sort) {
  PT(NodeRelation) hold_arc = detach();
  _parent = parent;
  _sort = sort;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::change_child
//       Access: Public
//  Description: Changes the child of the arc to a different node.
////////////////////////////////////////////////////////////////////
INLINE void NodeRelation::
change_child(Node *child) {
  PT(NodeRelation) hold_arc = detach();
  _child = child;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::change_parent_and_child
//       Access: Public
//  Description: Simultaneously moves both the parent and child of the
//               arc to different nodes.
////////////////////////////////////////////////////////////////////
INLINE void NodeRelation::
change_parent_and_child(Node *parent, Node *child) {
  PT(NodeRelation) hold_arc = detach();
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
INLINE void NodeRelation::
set_sort(int sort) {
  PT(NodeRelation) hold_arc = detach();
  _sort = sort;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::set_transition
//       Access: Public
//  Description: This flavor of set_transition() accepts a specific
//               TypeHandle, indicating the type of transition that we
//               are setting, and a NodeTransition pointer indicating
//               the value of the transition.  The NodeTransition may
//               be NULL indicating that the transition should be
//               cleared.  If the NodeTransition is not NULL, it must
//               match the type indicated by the TypeHandle.
//
//               The return value is a pointer to the *previous*
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
INLINE PT(NodeTransition) NodeRelation::
set_transition(TypeHandle handle, NodeTransition *trans) {
  PT(NodeTransition) old_trans =
    _transitions.set_transition(handle, trans);

  if (old_trans != (NodeTransition *)NULL) {
    old_trans->removed_from_arc(this);
  }
  if (trans != (NodeTransition *)NULL) {
    trans->added_to_arc(this);
  }

  changed_transition(handle);
  return old_trans;
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
INLINE PT(NodeTransition) NodeRelation::
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
INLINE PT(NodeTransition) NodeRelation::
set_transition(NodeTransition *trans, int priority) {
  nassertr(trans != (NodeTransition *)NULL, NULL);
  nassertr(trans->get_handle() != TypeHandle::none(), NULL);
  trans->set_priority(priority);
  return set_transition(trans->get_handle(), trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::clear_transition
//       Access: Public
//  Description: Removes any transition associated with the indicated
//               handle from the arc.
//
//               The return value is a pointer to the previous
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
INLINE PT(NodeTransition) NodeRelation::
clear_transition(TypeHandle handle) {
  PT(NodeTransition) old_trans =
    _transitions.clear_transition(handle);
  if (old_trans != (NodeTransition *)NULL) {
    old_trans->removed_from_arc(this);
  }

  changed_transition(handle);
  return old_trans;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRelation::has_transition
//       Access: Public
//  Description: Returns true if a transition associated with the
//               indicated handle has been stored on the arc (even if
//               it is the identity transition), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NodeRelation::
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
INLINE bool NodeRelation::
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
INLINE NodeTransition *NodeRelation::
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
INLINE int NodeRelation::
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
INLINE UpdateSeq NodeRelation::
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
INLINE NodeRelation *NodeRelation::
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
INLINE void NodeRelation::
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
INLINE Factory<NodeRelation> &NodeRelation::
get_factory() {
  if (_factory == (Factory<NodeRelation> *)NULL) {
    _factory = new Factory<NodeRelation>;
  }
  return *_factory;
}

////////////////////////////////////////////////////////////////////
//     Function: get_transition_into
//  Description: This external template function is handy for
//               extracting a transition of a particular type from the
//               arc.  If the transition exists, it is automatically
//               downcasted to the correct type and stored in the
//               pointer given in the first parameter, and the return
//               value is true.  If the transition does not exist, the
//               pointer is filled with NULL and the return value is
//               false.
////////////////////////////////////////////////////////////////////
template<class Transition>
INLINE bool 
get_transition_into(Transition *&ptr, const NodeRelation *arc,
		    TypeHandle transition_type) {
  NodeTransition *nt = arc->get_transition(transition_type);
  if (nt == (NodeTransition *)NULL) {
    ptr = (Transition *)NULL;
    return false;
  }
  DCAST_INTO_R(ptr, nt, false);
  return true;
}

template<class Transition>
INLINE bool 
get_transition_into(Transition *&ptr, const NodeRelation *arc) {
  return get_transition_into(ptr, arc, Transition::get_class_type());
}
