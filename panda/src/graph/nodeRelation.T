// Filename: nodeRelation.T
// Created by:  drose (30Sep99)
// 
////////////////////////////////////////////////////////////////////

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
INLINE_GRAPH bool 
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
INLINE_GRAPH bool 
get_transition_into(Transition *&ptr, const NodeRelation *arc) {
  return get_transition_into(ptr, arc, Transition::get_class_type());
}
