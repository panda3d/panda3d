// Filename: deferredArcTraverser.cxx
// Created by:  drose (04Jul00)
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
	    NullAttributeWrapper &, NullAttributeWrapper &,
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
