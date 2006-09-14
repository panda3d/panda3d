// Filename: eventStorePandaNode.cxx
// Created by:  drose (13Sep06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "eventStorePandaNode.h"

TypeHandle EventStorePandaNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EventStorePandaNode::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
EventStorePandaNode::
~EventStorePandaNode() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventStorePandaNode::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void EventStorePandaNode::
output(ostream &out) const {
  if (_value == (PandaNode *)NULL) {
    out << "(empty)";

  } else {
    out << *_value;
  }
}
