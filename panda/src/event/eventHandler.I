// Filename: eventHandler.I
// Created by:  skyler (27Jan04)
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


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::get_global_event_handler
//       Access: Public
//  Description: Returns a pointer to the one global EventHandler
//               object.  If the global object has not yet been
//               created, this will create it.
////////////////////////////////////////////////////////////////////
INLINE EventHandler *EventHandler::
get_global_event_handler(EventQueue *queue) {
  if (_global_event_handler == 0) {
    assert(queue);
    make_global_event_handler(queue);
  }
  return _global_event_handler;
}
