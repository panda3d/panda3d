// Filename: eventQueue.cxx
// Created by:  drose (08Feb99)
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

#include "eventQueue.h"
#include "config_event.h"

EventQueue *EventQueue::_global_event_queue = NULL;


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EventQueue::
EventQueue() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EventQueue::
~EventQueue() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::queue_event
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EventQueue::
queue_event(CPT_Event event) {
#ifdef OLD_HAVE_IPC
  mutex_lock lock(_lock);
#endif
  if (_queue.full()) {
    event_cat.error()
      << "Ignoring event " << *event << "; event queue full.\n";
  } else {
    _queue.push_back(event);
    if (event_cat.is_spam() || event_cat.is_debug()) {
      if (event->get_name() == "NewFrame") {
        // Don't bother us with this particularly spammy event.
        event_cat.spam()
          << "Throwing event " << *event << "\n";
      } else {
        event_cat.debug()
          << "Throwing event " << *event << "\n";
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::clear
//       Access: Public
//  Description: Empties all events on the queue, throwing them on the
//               floor.
////////////////////////////////////////////////////////////////////
void EventQueue::
clear() {
  while (!_queue.empty()) {
    _queue.pop_front();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::is_queue_empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool EventQueue::
is_queue_empty() const {
  return _queue.empty();
}

bool EventQueue::
is_queue_full() const {
  return _queue.full();
}


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::dequeue_event
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPT_Event EventQueue::
dequeue_event() {
  // We need no mutex protection here, as long as there is only one
  // thread extracting events.  The magic of circular buffers.
  CPT_Event result = _queue.front();
  _queue.pop_front();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::make_global_event_queue
//       Access: Protected, Static
//  Description:
////////////////////////////////////////////////////////////////////
void EventQueue::
make_global_event_queue() {
  _global_event_queue = new EventQueue;
}
