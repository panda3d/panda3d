// Filename: threadPosixImpl.h
// Created by:  drose (09Feb06)
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

#ifndef THREADPOSIXIMPL_H
#define THREADPOSIXIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_POSIX_IMPL

#include "pnotify.h"
#include "threadPriority.h"
#include "mutexPosixImpl.h"

#include <pthread.h>

class Thread;

////////////////////////////////////////////////////////////////////
//       Class : ThreadPosixImpl
// Description : Uses Posix threads to implement a thread.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ThreadPosixImpl {
public:
  INLINE ThreadPosixImpl(Thread *parent_obj);
  ~ThreadPosixImpl();

  bool start(ThreadPriority priority, bool joinable);
  void interrupt();
  void join();

  INLINE static void prepare_for_exit();

  INLINE static Thread *get_current_thread();
  INLINE static void bind_thread(Thread *thread);
  INLINE static bool is_threading_supported();
  INLINE static void sleep(double seconds);

private:
  static void *root_func(void *data);
  static void init_pt_ptr_index();

  enum Status {
    S_new,
    S_start_called,
    S_running,
    S_finished,
  };

  MutexPosixImpl _mutex;
  Thread *_parent_obj;
  pthread_t _thread;
  bool _joinable;
  bool _detached;
  Status _status;

  static pthread_key_t _pt_ptr_index;
  static bool _got_pt_ptr_index;
};

#include "threadPosixImpl.I"

#endif  // THREAD_POSIX_IMPL

#endif
