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

#include "circBuffer.h"

#ifdef OLD_HAVE_IPC
#include <ipc_mutex.h>
#endif

////////////////////////////////////////////////////////////////////
//       Class : EventQueue
// Description : A queue of pending events.  As events are thrown,
//               they are added to this queue; eventually, they will
//               be extracted out again by an EventHandler and
//               processed.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA EventQueue {
public:
  enum { max_events = 500 };

PUBLISHED:
  EventQueue();
  ~EventQueue();

  void queue_event(CPT_Event event);
  void clear();

  bool is_queue_empty() const;
  bool is_queue_full() const;
  CPT_Event dequeue_event();

  INLINE static EventQueue *get_global_event_queue();

protected:
  CircBuffer<CPT_Event, max_events> _queue;

  static void make_global_event_queue();
  static EventQueue *_global_event_queue;

#ifdef OLD_HAVE_IPC
  mutex _lock;
#endif
};

#include "eventQueue.I"

#endif
