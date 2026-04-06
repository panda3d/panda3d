/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventQueue.h
 * @author drose
 * @date 1999-02-08
 */

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "pandabase.h"

#include "event.h"
#include "pt_Event.h"
#include "lightMutex.h"
#include "pdeque.h"

/**
 * A queue of pending events.  As events are thrown, they are added to this
 * queue; eventually, they will be extracted out again by an EventHandler and
 * processed.
 */
class EXPCL_PANDA_EVENT EventQueue {
PUBLISHED:
  EventQueue();
  ~EventQueue();

  void queue_event(CPT_Event event);
  void clear();

  bool is_queue_empty() const;
  bool is_queue_full() const;
  CPT_Event dequeue_event();

  INLINE static EventQueue *get_global_event_queue();

private:
  static void make_global_event_queue();
  static EventQueue *_global_event_queue;

  typedef pdeque<CPT_Event> Events;
  Events _queue;

  LightMutex _lock;
};

#include "eventQueue.I"

#endif
