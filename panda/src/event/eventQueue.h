// Filename: eventQueue.h
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

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "pandabase.h"

#include "event.h"
#include "pt_Event.h"
#include "pmutex.h"
#include "pdeque.h"

////////////////////////////////////////////////////////////////////
//       Class : EventQueue
// Description : A queue of pending events.  As events are thrown,
//               they are added to this queue; eventually, they will
//               be extracted out again by an EventHandler and
//               processed.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA EventQueue {
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

  Mutex _lock;
};

#include "eventQueue.I"

#endif
