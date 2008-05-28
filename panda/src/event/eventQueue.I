// Filename: eventQueue.I
// Created by:  drose (05May00)
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
//     Function: EventQueue::get_global_event_queue
//       Access: Public
//  Description: Returns a pointer to the one global EventQueue
//               object.  If the global object has not yet been
//               created, this will create it.
////////////////////////////////////////////////////////////////////
INLINE EventQueue *EventQueue::
get_global_event_queue() {
  if (_global_event_queue == NULL) {
    make_global_event_queue();
  }
  return _global_event_queue;
}
