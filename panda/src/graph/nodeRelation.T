// Filename: nodeRelation.T
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
