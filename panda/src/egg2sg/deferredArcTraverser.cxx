// Filename: deferredArcTraverser.cxx
// Created by:  drose (04Jul00)
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

#include "deferredArcTraverser.h"

////////////////////////////////////////////////////////////////////
//     Function: DeferredArcTraverser::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DeferredArcTraverser::
DeferredArcTraverser(const DeferredArcs &deferred_arcs) :
  _deferred_arcs(deferred_arcs)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DeferredArcTraverser::forward_arc
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool DeferredArcTraverser::
forward_arc(NodeRelation *arc, NullTransitionWrapper &,
            NullTransitionWrapper &, NullTransitionWrapper &,
            DeferredArcProperty &level_state) {

  // Do we have a DeferredArcProperty associated with this arc?
  DeferredArcs::const_iterator dai;
  dai = _deferred_arcs.find(arc);

  if (dai != _deferred_arcs.end()) {
    const DeferredArcProperty &def = (*dai).second;
    level_state.compose(def);
  }

  // Now apply the accumulated state to both the arc and its node.
  level_state.apply_to_arc(arc);
  level_state.apply_to_node(arc->get_child());

  return true;
}
