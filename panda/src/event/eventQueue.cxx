/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventQueue.cxx
 * @author drose
 * @date 1999-02-08
 */

#include "eventQueue.h"
#include "config_event.h"
#include "lightMutexHolder.h"

EventQueue *EventQueue::_global_event_queue = nullptr;


/**
 *
 */
EventQueue::
EventQueue() : _lock("EventQueue::_lock") {
}

/**
 *
 */
EventQueue::
~EventQueue() {
}

/**
 *
 */
void EventQueue::
queue_event(CPT_Event event) {
  nassertv(!event.is_null());
  if (event->get_name().empty()) {
    // Never mind.
    return;
  }

  LightMutexHolder holder(_lock);

  _queue.push_back(event);
  if (event_cat.is_debug()) {
    if (event->get_name() == "NewFrame") {
      // Don't bother us with this particularly spammy event.
      if (event_cat.is_spam()) {
        event_cat.spam()
          << "Throwing event " << *event << "\n";
      }
    } else {
      event_cat.debug()
        << "Throwing event " << *event << "\n";
    }
  }
}

/**
 * Empties all events on the queue, throwing them on the floor.
 */
void EventQueue::
clear() {
  LightMutexHolder holder(_lock);

  _queue.clear();
}


/**
 *
 */
bool EventQueue::
is_queue_empty() const {
  LightMutexHolder holder(_lock);
  return _queue.empty();
}

/**
 * @deprecated Always returns false; the queue can never be full.
 */
bool EventQueue::
is_queue_full() const {
  return false;
}


/**
 *
 */
CPT_Event EventQueue::
dequeue_event() {
  LightMutexHolder holder(_lock);

  CPT_Event result = _queue.front();
  _queue.pop_front();

  nassertr(!result.is_null(), result);
  return result;
}

/**
 *
 */
void EventQueue::
make_global_event_queue() {
  init_memory_hook();
  _global_event_queue = new EventQueue;
}
