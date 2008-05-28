// Filename: eventStorePandaNode.I
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


////////////////////////////////////////////////////////////////////
//     Function: EventStorePandaNode::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EventStorePandaNode::
EventStorePandaNode(const PandaNode *value) : 
  _value((PandaNode *)value) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: EventStorePandaNode::set_value
//       Access: Published
//  Description: Changes the value stored in the parameter.  It is
//               dangerous to do this for a parameter already added to
//               an event, since the parameters may be shared.
////////////////////////////////////////////////////////////////////
INLINE void EventStorePandaNode::
set_value(const PandaNode *value) {
  _value = (PandaNode *)value;
}


////////////////////////////////////////////////////////////////////
//     Function: EventStorePandaNode::get_value
//       Access: Published
//  Description: Retrieves the value stored in the parameter.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *EventStorePandaNode::
get_value() const {
  return _value;
}
