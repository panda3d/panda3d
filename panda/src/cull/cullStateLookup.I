// Filename: cullStateLookup.I
// Created by:  drose (13Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "cullState.h"

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CullStateLookup::
CullStateLookup() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::is_completely_empty
//       Access: Public
//  Description: Returns true if the lookup is completely empty; that
//               is, it represents no Nodes, and no subtrees.
////////////////////////////////////////////////////////////////////
INLINE bool CullStateLookup::
is_completely_empty() const {
  return _cull_states.empty() && _subtrees.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: CullStateLookup::record_node
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CullStateLookup::
record_node(Node *node, CullState *cs, UpdateSeq now) {
  _cull_states[node] = cs;
  cs->mark_verified(node, now);
}
