// Filename: threadNsprImpl.h
// Created by:  drose (08Aug02)
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

#ifndef THREADNSPRIMPL_H
#define THREADNSPRIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_NSPR_IMPL

#include "notify.h"
#include "threadPriority.h"
#include "pmutex.h"

#include <prthread.h>
#include <prinit.h>

class Thread;

////////////////////////////////////////////////////////////////////
//       Class : ThreadNsprImpl
// Description : Uses NSPR to implement a thread.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ThreadNsprImpl {
public:
  INLINE ThreadNsprImpl(Thread *parent_obj);
  INLINE ~ThreadNsprImpl();

  bool start(ThreadPriority priority, bool global, bool joinable);
  void interrupt();
  void join();

  INLINE static void prepare_for_exit();

  INLINE static Thread *get_current_thread();
  INLINE static bool is_threading_supported();
  INLINE static void sleep(double seconds);

private:
  static void root_func(void *data);
  static void pt_ptr_destructor(void *data);

  Mutex _mutex;
  Thread *_parent_obj;
  PRThread *_thread;
  bool _joinable;
  static PRUintn _pt_ptr_index;
  static bool _got_pt_ptr_index;
};

#include "threadNsprImpl.I"

#endif  // THREAD_NSPR_IMPL

#endif
