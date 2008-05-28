// Filename: eventStorePandaNode.cxx
// Created by:  drose (13Sep06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
