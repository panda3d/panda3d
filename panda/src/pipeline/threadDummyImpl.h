/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadDummyImpl.h
 * @author drose
 * @date 2002-08-09
 */

#ifndef THREADDUMMYIMPL_H
#define THREADDUMMYIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_DUMMY_IMPL

#include "pnotify.h"
#include "threadPriority.h"

class Thread;

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>  // For Sleep().
#endif

/**
 * A fake thread implementation for single-threaded applications.  This simply
 * fails whenever you try to start a thread.
 */
class EXPCL_PANDA_PIPELINE ThreadDummyImpl {
public:
  INLINE ThreadDummyImpl(Thread *parent_obj);
  INLINE ~ThreadDummyImpl();

  INLINE void setup_main_thread();
  INLINE bool start(ThreadPriority priority, bool joinable);
  INLINE void join();
  INLINE void preempt();

  std::string get_unique_id() const;

  INLINE static void prepare_for_exit();

  static Thread *get_current_thread();
  INLINE static void bind_thread(Thread *thread);
  INLINE static bool is_threading_supported();
  INLINE static bool is_true_threads();
  INLINE static bool is_simple_threads();
  INLINE static void sleep(double seconds);
  INLINE static void yield();
  INLINE static void consider_yield();
};

#include "threadDummyImpl.I"

#endif // THREAD_DUMMY_IMPL

#endif
