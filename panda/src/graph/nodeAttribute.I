// Filename: nodeAttribute.I
// Created by:  drose (20Mar00)
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
//     Function: NodeAttribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttribute::
NodeAttribute() {
  _priority = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttribute::
NodeAttribute(const NodeAttribute &copy) :
  TypedReferenceCount(copy),
  _priority(copy._priority)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeAttribute::
operator = (const NodeAttribute &copy) {
  TypedReferenceCount::operator = (copy);
  _priority = copy._priority;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::Equality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeAttribute::
operator == (const NodeAttribute &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeAttribute::
operator != (const NodeAttribute &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeAttribute::
operator < (const NodeAttribute &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeAttribute::
operator <= (const NodeAttribute &other) const {
  return compare_to(other) <= 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeAttribute::
operator > (const NodeAttribute &other) const {
  return compare_to(other) > 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool NodeAttribute::
operator >= (const NodeAttribute &other) const {
  return compare_to(other) >= 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::compare_to
//       Access: Public
//  Description: This function works like strcmp(): it compares the
//               two attributes and returns a number less than zero
//               if this attribute sorts before the other one, equal
//               to zero if they are equivalent, or greater than zero
//               if this attribute sorts after the other one.
//
//               This imposes an arbitrary sorting order across all
//               attributes, whose sole purpose is to allow grouping
//               of equivalent attributes together in STL structures
//               like maps and sets.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int NodeAttribute::
compare_to(const NodeAttribute &other) const {
  TypeHandle my_handle = get_handle();
  TypeHandle other_handle = other.get_handle();

  if (my_handle == other_handle) {
    return internal_compare_to(&other);

  } else {
    return
      (my_handle < other_handle) ? -1 : 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::set_priority
//       Access: Public
//  Description: Changes the priority associated with this attribute.
//               The attribute will not be affected by transitions
//               with a lower priority.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeAttribute::
set_priority(int priority) {
  _priority = priority;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttribute::get_priority
//       Access: Public
//  Description: Returns the priority associated with this attribute.
//               Normally this is of limited value; the priority is
//               meaningful primarily on the transitions.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int NodeAttribute::
get_priority() const {
  return _priority;
}

INLINE_GRAPH ostream &operator << (ostream &out, const NodeAttribute &nab) {
  nab.output(out);
  return out;
}
