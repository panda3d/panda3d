// Filename: cullStateLookup.I
// Created by:  drose (13Apr00)
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
