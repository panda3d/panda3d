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

