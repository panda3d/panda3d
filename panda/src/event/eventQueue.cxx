// Filename: eventQueue.cxx
// Created by:  drose (08Feb99)
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

#include "eventQueue.h"
#include "config_event.h"
#include "mutexHolder.h"

EventQueue *EventQueue::_global_event_queue = NULL;


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EventQueue::
EventQueue() {
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
  MutexHolder holder(_lock);

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
  MutexHolder holder(_lock);

  _queue.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: EventQueue::is_queue_empty
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool EventQueue::
is_queue_empty() const {
  MutexHolder holder(_lock);
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
  MutexHolder holder(_lock);

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
