// Filename: threadLinuxImpl.h
// Created by:  drose (28Mar06)
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

#ifndef THREADLINUXIMPL_H
#define THREADLINUXIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_LINUX_IMPL

#include "pnotify.h"
#include "threadPriority.h"
#include "mutexLinuxImpl.h"
#include "conditionVarLinuxImpl.h"

class Thread;

////////////////////////////////////////////////////////////////////
//       Class : ThreadLinuxImpl
// Description : Uses low-level Linux-specific calls to implement
//               threads.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ThreadLinuxImpl {
public:
  INLINE ThreadLinuxImpl(Thread *parent_obj);
  ~ThreadLinuxImpl();

  bool start(ThreadPriority priority, bool global, bool joinable);
  void interrupt();
  void join();

  INLINE static void prepare_for_exit();

  static Thread *get_current_thread();
  static void bind_thread(Thread *thread);
  INLINE static bool is_threading_supported();
  INLINE static void sleep(double seconds);

private:
  static int root_func(void *data);

  enum Status {
    S_new,
    S_start_called,
    S_running,
    S_finished
  };

  MutexLinuxImpl _mutex;
  ConditionVarLinuxImpl _cv;
  Thread *_parent_obj;
  int _thread;
  bool _joinable;
  Status _status;
  unsigned char *_stack;

  // per-thread data.
  typedef pmap<pid_t, Thread *> ThreadPointers;
  static ThreadPointers _thread_pointers;
  static MutexLinuxImpl _thread_pointers_lock;
  static bool _got_main_thread_pointer;
};

#include "threadLinuxImpl.I"

#endif  // THREAD_LINUX_IMPL

#endif
