// Filename: nodeTransitions.cxx
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nodeTransitions.h"
#include "config_graph.h"
#include "setTransitionHelpers.h"
#include "nodeRelation.h"

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransitions::
NodeTransitions() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransitions::
NodeTransitions(const NodeTransitions &copy) :
  _transitions(copy._transitions)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeTransitions::
operator = (const NodeTransitions &copy) {
  _transitions = copy._transitions;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NodeTransitions::
~NodeTransitions() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::is_empty
//       Access: Public
//  Description: Returns true if there are no Transitions stored in
//               the set, or false if there are any (even identity)
//               Transitions.
////////////////////////////////////////////////////////////////////
bool NodeTransitions::
is_empty() const {
  return _transitions.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::set_transition
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
PT(NodeTransition) NodeTransitions::
set_transition(TypeHandle handle, NodeTransition *trans) {
  if (trans == (NodeTransition *)NULL) {
    return clear_transition(handle);

  } else {
    Transitions::iterator ti;
    ti = _transitions.find(handle);
    if (ti != _transitions.end()) {
      PT(NodeTransition) result = (*ti).second;
      (*ti).second = trans;
      return result;
    }

    _transitions.insert(Transitions::value_type(handle, trans));
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::clear_transition
//       Access: Public
//  Description: Removes any transition associated with the indicated
//               handle from the set.
//
//               The return value is a pointer to the previous
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
PT(NodeTransition) NodeTransitions::
clear_transition(TypeHandle handle) {
  nassertr(handle != TypeHandle::none(), NULL);

  Transitions::iterator ti;
  ti = _transitions.find(handle);
  if (ti != _transitions.end()) {
    PT(NodeTransition) result = (*ti).second;
    _transitions.erase(ti);
    return result;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::has_transition
//       Access: Public
//  Description: Returns true if a transition associated with the
//               indicated handle has been stored in the set (even if
//               it is the identity transition), or false otherwise.
////////////////////////////////////////////////////////////////////
bool NodeTransitions::
has_transition(TypeHandle handle) const {
  nassertr(handle != TypeHandle::none(), false);
  return _transitions.count(handle) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::get_transition
//       Access: Public
//  Description: Returns the transition associated with the indicated
//               handle, or NULL if no such transition has been stored
//               in the set.
////////////////////////////////////////////////////////////////////
NodeTransition *NodeTransitions::
get_transition(TypeHandle handle) const {
  nassertr(handle != TypeHandle::none(), NULL);
  Transitions::const_iterator ti;
  ti = _transitions.find(handle);
  if (ti != _transitions.end()) {
    return (*ti).second;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::clear
//       Access: Public
//  Description: Removes all transitions from the set.
////////////////////////////////////////////////////////////////////
void NodeTransitions::
clear() {
  _transitions.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::copy_transitions_from
//       Access: Public
//  Description: Copies all of the transitions stored in the other set
//               to this set.  Any existing transitions in this set,
//               for which there was not a corresponding transition of
//               the same type in the other set, are left undisturbed.
////////////////////////////////////////////////////////////////////
void NodeTransitions::
copy_transitions_from(const NodeTransitions &other) {
  Transitions temp;
  tmap_union(_transitions.begin(), _transitions.end(),
	     other._transitions.begin(), other._transitions.end(),
	     inserter(temp, temp.begin()));
  _transitions.swap(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::copy_transitions_from
//       Access: Public
//  Description: As above, but updates the transitions on the way to
//               indicated they are being attached to (or removed
//               from) the indicated arc.  This is intended only to be
//               called from NodeRelation::copy_transitions(); don't
//               call it directly.
////////////////////////////////////////////////////////////////////
void NodeTransitions::
copy_transitions_from(const NodeTransitions &other,
		      NodeRelation *to_arc) {
  Transitions temp;
  tmap_arc_union(_transitions.begin(), _transitions.end(),
		 other._transitions.begin(), other._transitions.end(),
		 to_arc, inserter(temp, temp.begin()));
  _transitions.swap(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::compose_transitions_from
//       Access: Public
//  Description: Similar to copy_transitions_from(), except that if
//               the same type of transition exists on both arcs, the
//               composition of the two is stored.  The result
//               represents the same set of transitions that would
//               result from composing the two individual sets of
//               transitions.
////////////////////////////////////////////////////////////////////
void NodeTransitions::
compose_transitions_from(const NodeTransitions &other,
			 NodeRelation *to_arc) {
  Transitions temp;
  tmap_arc_compose(_transitions.begin(), _transitions.end(),
		   other._transitions.begin(), other._transitions.end(),
		   to_arc, inserter(temp, temp.begin()));
  _transitions.swap(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::compare_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int NodeTransitions::
compare_to(const NodeTransitions &other) const {
  return tmap_compare_trans(_transitions.begin(), _transitions.end(),
			    other._transitions.begin(), other._transitions.end());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::adjust_all_proriorities
//       Access: Public
//  Description: Adds the indicated adjustment amount (which may be
//               negative) to the priority for all transitions on the
//               arc.  If the priority would drop below zero, it is
//               set to zero.
////////////////////////////////////////////////////////////////////
void NodeTransitions::
adjust_all_priorities(int adjustment, NodeRelation *arc) {
  Transitions::iterator ti;
  for (ti = _transitions.begin(); ti != _transitions.end(); ++ti) {
    nassertv((*ti).second != (NodeTransition *)NULL);
    PT(NodeTransition) &trans = (*ti).second;
    int new_priority = max(trans->_priority + adjustment, 0);

    if (trans->_priority != new_priority) {
      if (trans->get_ref_count() > 1) {
	// Copy-on-write.
	trans->removed_from_arc(arc);
	trans = trans->make_copy();
	trans->added_to_arc(arc);
      }
      trans->_priority = new_priority;
      arc->changed_transition((*ti).first);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::remove_all_from_arc
//       Access: Public
//  Description: Marks all of the transitions in the set as removed
//               from the indicated arc.  This is intended to be
//               called only by the arc's destructor.
////////////////////////////////////////////////////////////////////
void NodeTransitions::
remove_all_from_arc(NodeRelation *arc) {
  Transitions::iterator ti;
  for (ti = _transitions.begin(); ti != _transitions.end(); ++ti) {
    nassertv((*ti).second != (NodeTransition *)NULL);
    (*ti).second->removed_from_arc(arc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeTransitions::
output(ostream &out) const {
  bool written_any = false;
  Transitions::const_iterator ti;
  for (ti = _transitions.begin(); ti != _transitions.end(); ++ti) {
    if ((*ti).second != (NodeTransition *)NULL) {
      if (written_any) {
	out << " ";
      }
      out << *(*ti).second;
      written_any = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeTransitions::
write(ostream &out, int indent_level) const {
  Transitions::const_iterator ti;
  for (ti = _transitions.begin(); ti != _transitions.end(); ++ti) {
    if ((*ti).second != (NodeTransition *)NULL) {
      indent(out, indent_level)
	<< (*ti).first << " = " << *(*ti).second << "\n";
      //      (*ti).second->write(out, indent_level + 2);
    }
  }
}
