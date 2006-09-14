// Filename: eventStorePandaNode.I
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
