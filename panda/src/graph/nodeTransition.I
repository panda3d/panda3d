// Filename: nodeTransition.I
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeTransition::
NodeTransition() {
  _priority = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeTransition::
NodeTransition(const NodeTransition &copy) :
  TypedWriteableReferenceCount(copy),
  _priority(copy._priority)
{
  // We specifically do *not* copy the _arcs list.
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodeTransition::
operator = (const NodeTransition &copy) {
  TypedWriteableReferenceCount::operator = (copy);
  _priority = copy._priority;
  // We specifically do *not* copy the _arcs list.
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodeTransition::
operator == (const NodeTransition &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodeTransition::
operator != (const NodeTransition &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodeTransition::
operator < (const NodeTransition &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodeTransition::
operator <= (const NodeTransition &other) const {
  return compare_to(other) <= 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodeTransition::
operator > (const NodeTransition &other) const {
  return compare_to(other) > 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodeTransition::
operator >= (const NodeTransition &other) const {
  return compare_to(other) >= 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::compare_to
//       Access: Public
//  Description: This function works like strcmp(): it compares the
//               two transitions and returns a number less than zero
//               if this transition sorts before the other one, equal
//               to zero if they are equivalent, or greater than zero
//               if this transition sorts after the other one.
//
//               This imposes an arbitrary sorting order across all
//               transitions, whose sole purpose is to allow grouping
//               of equivalent transitions together in STL structures
//               like maps and sets.
////////////////////////////////////////////////////////////////////
INLINE int NodeTransition::
compare_to(const NodeTransition &other) const {
  TypeHandle my_handle = get_handle();
  TypeHandle other_handle = other.get_handle();

  if (my_handle != other_handle) {
    return 
      (my_handle < other_handle) ? -1 : 1;

  } else if (_priority != other._priority) {
    return _priority - other._priority;

  } else {
    return internal_compare_to(&other);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::set_priority
//       Access: Public
//  Description: Changes the priority associated with this transition.
//               The transition will always override transitions with
//               a lower priority.
////////////////////////////////////////////////////////////////////
INLINE void NodeTransition::
set_priority(int priority) {
  _priority = priority;
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::get_priority
//       Access: Public
//  Description: Returns the priority associated with this transition.
////////////////////////////////////////////////////////////////////
INLINE int NodeTransition::
get_priority() const {
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::add_to_arc
//       Access: Protected
//  Description: Intended to be called only by the arc's
//               set_transition() method to indicate that the
//               transition has been assigned to the arc.  Further
//               state changes on the transition will now notify the
//               arc.
////////////////////////////////////////////////////////////////////
INLINE void NodeTransition::
added_to_arc(NodeRelation *arc) {
  bool inserted = _arcs.insert(arc).second;
  nassertv(inserted);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransition::add_to_arc
//       Access: Protected
//  Description: Intended to be called by the arc's
//               set_transition() or clear_transition() method (or by
//               the arc's destructor) to indicate that the transition
//               is no longer assigned to the arc.  Further state
//               changes on the transition will *not* notify the arc.
////////////////////////////////////////////////////////////////////
INLINE void NodeTransition::
removed_from_arc(NodeRelation *arc) {
  size_t num_erased = _arcs.erase(arc);
  nassertv(num_erased == 1);
}
