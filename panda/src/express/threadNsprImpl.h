// Filename: threadNsprImpl.h
// Created by:  drose (08Aug02)
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

#ifndef THREADNSPRIMPL_H
#define THREADNSPRIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_NSPR_IMPL

#include "notify.h"
#include "threadPriority.h"
#include "mutexNsprImpl.h"

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
  ~ThreadNsprImpl();

  bool start(ThreadPriority priority, bool global, bool joinable);
  void interrupt();
  void join();

  INLINE static void prepare_for_exit();

  INLINE static Thread *get_current_thread();
  INLINE static void bind_thread(Thread *thread);
  INLINE static bool is_threading_supported();
  INLINE static void sleep(double seconds);

private:
  static void root_func(void *data);
  static void init_pt_ptr_index();

  MutexNsprImpl _mutex;
  Thread *_parent_obj;
  PRThread *_thread;
  bool _joinable;
  static PRUintn _pt_ptr_index;
  static bool _got_pt_ptr_index;
};

#include "threadNsprImpl.I"

#endif  // THREAD_NSPR_IMPL

#endif
