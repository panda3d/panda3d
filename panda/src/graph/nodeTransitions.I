// Filename: nodeTransitions.I
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::set_transition
//       Access: Public
//  Description: This flavor of set_transition() accepts a pointer to
//               a NodeTransition only.  It infers the type of the
//               NodeTransition from the pointer.  However, it is not
//               valid to pass a NULL pointer to this flavor of
//               set_transition; if the pointer might be NULL, use the
//               above flavor instead (or just call clear_transition).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeTransition) NodeTransitions::
set_transition(NodeTransition *trans) {
  nassertr(trans != (NodeTransition *)NULL, NULL);
  nassertr(trans->get_handle() != TypeHandle::none(), NULL);
  return set_transition(trans->get_handle(), trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::size
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitions::size_type NodeTransitions::
size() const {
  return _transitions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::begin
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitions::const_iterator NodeTransitions::
begin() const {
  return _transitions.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::end
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitions::const_iterator NodeTransitions::
end() const {
  return _transitions.end();
}

////////////////////////////////////////////////////////////////////
//     Function: get_transition_into
//  Description: This external template function is handy for
//               extracting a transition of a particular type from the
//               set.  If the transition exists, it is automatically
//               downcasted to the correct type and stored in the
//               pointer given in the first parameter, and the return
//               value is true.  If the transition does not exist, the
//               pointer is filled with NULL and the return value is
//               false.
////////////////////////////////////////////////////////////////////
template<class Transition>
INLINE_GRAPH bool 
get_transition_into(Transition *&ptr, const NodeTransitions &trans,
                    TypeHandle transition_type) {
  NodeTransition *nt = trans.get_transition(transition_type);
  if (nt == (NodeTransition *)NULL) {
    ptr = (Transition *)NULL;
    return false;
  }
  DCAST_INTO_R(ptr, nt, false);
  return true;
}

template<class Transition>
INLINE_GRAPH bool 
get_transition_into(Transition *&ptr, const NodeTransitions &trans) {
  return get_transition_into(ptr, trans, Transition::get_class_type());
}

INLINE_GRAPH ostream &operator << (ostream &out, const NodeTransitions &nts) {
  nts.output(out);
  return out;
}
