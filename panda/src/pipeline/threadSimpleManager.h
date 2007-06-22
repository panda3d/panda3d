// Filename: threadSimpleManager.h
// Created by:  drose (18Jun07)
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

#ifndef THREADSIMPLEMANAGER_H
#define THREADSIMPLEMANAGER_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "pdeque.h"
#include "pmap.h"
#include "pvector.h"
#include "trueClock.h"
#include <algorithm>

class Thread;
class ThreadSimpleImpl;
class BlockerSimple;

////////////////////////////////////////////////////////////////////
//       Class : ThreadSimpleManager
// Description : This is the global object that selects the
//               currently-active thread of the various
//               ThreadSimpleImpl objects running, when the
//               currently-active thread yields.
//
//               This class only exists when we are using the
//               ThreadSimple implementation, which is to say, we are
//               not using "real" threads.
//
//               Generally, you shouldn't be calling these methods
//               directly.  Call the interfaces on Thread instead.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ThreadSimpleManager {
private:
  ThreadSimpleManager();

public:
  void enqueue_ready(ThreadSimpleImpl *thread);
  void enqueue_sleep(ThreadSimpleImpl *thread, double seconds);
  void enqueue_block(ThreadSimpleImpl *thread, BlockerSimple *blocker);
  void unblock_one(BlockerSimple *blocker);
  void unblock_all(BlockerSimple *blocker);
  void enqueue_finished(ThreadSimpleImpl *thread);
  void preempt(ThreadSimpleImpl *thread);
  void next_context();

  void prepare_for_exit();

  INLINE ThreadSimpleImpl *get_current_thread();
  void set_current_thread(ThreadSimpleImpl *current_thread);

  INLINE double get_current_time() const;
  INLINE static ThreadSimpleManager *get_global_ptr();

private:
  static void init_pointers();

  static void st_choose_next_context(void *data);
  void choose_next_context();
  void wake_sleepers(double now);
  static void system_sleep(double seconds);
  void report_deadlock();

  // STL function object to sort the priority queue of sleeping threads.
  class CompareStartTime {
  public:
    INLINE bool operator ()(ThreadSimpleImpl *a, ThreadSimpleImpl *b) const;
  };

  typedef pdeque<ThreadSimpleImpl *> FifoThreads;
  typedef pvector<ThreadSimpleImpl *> Sleeping;

  void kill_non_joinable(FifoThreads &threads);
  void kill_non_joinable(Sleeping &threads);

private:
  ThreadSimpleImpl *volatile _current_thread;

  // FIFO list of ready threads.
  FifoThreads _ready;

  typedef pmap<BlockerSimple *, FifoThreads> Blocked;
  Blocked _blocked;

  // Priority queue (partially-ordered heap) based on wakeup time.
  Sleeping _sleeping;

  FifoThreads _finished;

  ThreadSimpleImpl *_waiting_for_exit;

  TrueClock *_clock;

  static bool _pointers_initialized;
  static ThreadSimpleManager *_global_ptr;
};

// We include this down here to avoid the circularity problem.
/* okcircular */
#include "threadSimpleImpl.h"

#include "threadSimpleManager.I"

#endif // THREAD_SIMPLE_IMPL

#endif
