// Filename: threadDummyImpl.h
// Created by:  drose (09Aug02)
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

#ifndef THREADDUMMYIMPL_H
#define THREADDUMMYIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_DUMMY_IMPL

#include "pnotify.h"
#include "threadPriority.h"

class Thread;

////////////////////////////////////////////////////////////////////
//       Class : ThreadDummyImpl
// Description : A fake thread implementation for single-threaded
//               applications.  This simply fails whenever you try to
//               start a thread.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ThreadDummyImpl {
public:
  INLINE ThreadDummyImpl(Thread *parent_obj);
  INLINE ~ThreadDummyImpl();

  INLINE void setup_main_thread();
  INLINE bool start(ThreadPriority priority, bool joinable);
  INLINE void join();
  INLINE void preempt();

  INLINE static void prepare_for_exit();

  static Thread *get_current_thread();
  INLINE static void bind_thread(Thread *thread);
  INLINE static bool is_threading_supported();
  INLINE static void sleep(double seconds);
  INLINE static void yield();
  INLINE static void consider_yield();
};

#include "threadDummyImpl.I"

#endif // THREAD_DUMMY_IMPL

#endif
