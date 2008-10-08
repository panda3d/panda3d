// Filename: eventQueue.cxx
// Created by:  drose (08Feb99)
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

#include "eventQueue.h"
#include "config_event.h"
#include "lightMutexHolder.h"

EventQueue *EventQueue::_global_event_queue = NULL;


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EventQueue::
EventQueue() : _lock("EventQueue::_lock") {
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EventQueue::
~EventQueue() {
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::queue_event
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void EventQueue::
queue_event(CPT_Event event) {
  nassertv(!event.is_null());
  if (event->get_name().empty()) {
    // Never mind.
    return;
  }

  LightMutexHolder holder(_lock);

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

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::clear
//       Access: Published
//  Description: Empties all events on the queue, throwing them on the
//               floor.
////////////////////////////////////////////////////////////////////
void EventQueue::
clear() {
  LightMutexHolder holder(_lock);

  _queue.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::is_queue_empty
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool EventQueue::
is_queue_empty() const {
  LightMutexHolder holder(_lock);
  return _queue.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EventQueue::is_queue_full
//       Access: Published
//  Description: This function is deprecated--the queue is never full
//               these days.
////////////////////////////////////////////////////////////////////
bool EventQueue::
is_queue_full() const {
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::dequeue_event
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT_Event EventQueue::
dequeue_event() {
  LightMutexHolder holder(_lock);

  CPT_Event result = _queue.front();
  _queue.pop_front();

  nassertr(!result.is_null(), result);
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
