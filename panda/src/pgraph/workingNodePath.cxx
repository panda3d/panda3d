// Filename: workingNodePath.cxx
// Created by:  drose (16Mar02)
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

#include "workingNodePath.h"


////////////////////////////////////////////////////////////////////
//     Function: WorkingNodePath::r_get_node_path
//       Access: Private
//  Description: The private, recursive implementation of
//               get_node_path(), this returns the NodePathComponent
//               representing the NodePath.
////////////////////////////////////////////////////////////////////
PT(qpNodePathComponent) WorkingNodePath::
r_get_node_path() const {
  if (_next == (WorkingNodePath *)NULL) {
    return _start;
  }

  nassertr(_start == (qpNodePathComponent *)NULL, NULL);
  nassertr(_node != (PandaNode *)NULL, NULL);

  PT(qpNodePathComponent) comp = _next->r_get_node_path();
  nassertr(comp != (qpNodePathComponent *)NULL, NULL);
  return PandaNode::get_component(comp, _node);
}
