// Filename: nodeTransitionCacheEntry.cxx
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

#include "nodeTransitionCacheEntry.h"

#include <indent.h>


////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void NodeTransitionCacheEntry::
output(ostream &out) const {
  if (_trans != (NodeTransition *)NULL) {
    out << *_trans;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitionCacheEntry::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void NodeTransitionCacheEntry::
write(ostream &out, int indent_level) const {
  if (_trans != (NodeTransition *)NULL) {
    _trans->write(out, indent_level);
  }
}
