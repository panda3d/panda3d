// Filename: nodeTransitionWrapper.T
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: get_transition_into
//  Description: This external template function is handy for
//               extracting the transition (of a known type) from the
//               wrapper.  If the transition exists, it is
//               automatically downcasted to the correct type and
//               stored in the pointer given in the first parameter,
//               and the return value is true.  If the transition does
//               not exist, the pointer is filled with NULL and the
//               return value is false.
////////////////////////////////////////////////////////////////////
template<class Transition>
INLINE_GRAPH bool 
get_transition_into(Transition *&ptr, const NodeTransitionWrapper &trans) {
  NodeTransition *nt = trans.get_trans();
  if (nt == (NodeTransition *)NULL) {
    ptr = (Transition *)NULL;
    return false;
  }
  DCAST_INTO_R(ptr, nt, false);
  return true;
}
