// Filename: allTransitionsWrapper.cxx
// Created by:  drose (21Mar00)
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

#include "allTransitionsWrapper.h"
#include "nodeRelation.h"

#include <indent.h>

NodeTransitionCache AllTransitionsWrapper::_empty_cache;

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::set_transition
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
PT(NodeTransition) AllTransitionsWrapper::
set_transition(TypeHandle handle, NodeTransition *trans) {
  if (_cache == (NodeTransitionCache *)NULL) {
    _cache = new NodeTransitionCache;

  } else if (_cache->get_ref_count() != 1) {
    // Copy-on-write.
    _cache = new NodeTransitionCache(*_cache);
  }

  _all_verified.clear();
  return _cache->set_transition(handle, trans);
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::clear_transition
//       Access: Public
//  Description: Removes any transition associated with the indicated
//               handle from the set.
//
//               The return value is a pointer to the previous
//               transition in the set, if any, or NULL if there was
//               none.
////////////////////////////////////////////////////////////////////
PT(NodeTransition) AllTransitionsWrapper::
clear_transition(TypeHandle handle) {
  if (_cache == (NodeTransitionCache *)NULL) {
    return NULL;

  } else if (_cache->get_ref_count() != 1) {
    // Copy-on-write.
    _cache = new NodeTransitionCache(*_cache);
  }

  _all_verified.clear();
  return _cache->clear_transition(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AllTransitionsWrapper::
output(ostream &out) const {
  if (_cache != (NodeTransitionCache *)NULL) {
    out << *_cache;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AllTransitionsWrapper::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AllTransitionsWrapper::
write(ostream &out, int indent_level) const {
  if (_cache != (NodeTransitionCache *)NULL) {
    _cache->write(out, indent_level);
  }
}

