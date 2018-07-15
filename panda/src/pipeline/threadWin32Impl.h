/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadWin32Impl.h
 * @author drose
 * @date 2006-02-07
 */

#ifndef THREADWIN32IMPL_H
#define THREADWIN32IMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_WIN32_IMPL

#include "pnotify.h"
#include "threadPriority.h"
#include "mutexWin32Impl.h"
#include "conditionVarWin32Impl.h"

class Thread;

/**
 * Uses native Windows calls to implement a thread.
 */
class EXPCL_PANDA_PIPELINE ThreadWin32Impl {
public:
  INLINE ThreadWin32Impl(Thread *parent_obj);
  ~ThreadWin32Impl();

  void setup_main_thread();
  bool start(ThreadPriority priority, bool joinable);
  void join();
  INLINE void preempt();

  std::string get_unique_id() const;

  INLINE static void prepare_for_exit();

  INLINE static Thread *get_current_thread();
  INLINE static void bind_thread(Thread *thread);
  INLINE static bool is_threading_supported();
  INLINE static bool is_true_threads();
  INLINE static bool is_simple_threads();
  INLINE static void sleep(double seconds);
  INLINE static void yield();
  INLINE static void consider_yield();

private:
  static DWORD WINAPI root_func(LPVOID data);
  static void init_pt_ptr_index();

  enum Status {
    S_new,
    S_start_called,
    S_running,
    S_finished
  };

  MutexWin32Impl _mutex;
  ConditionVarWin32Impl _cv;
  Thread *_parent_obj;
  HANDLE _thread;
  DWORD _thread_id;
  bool _joinable;
  Status _status;

  static DWORD _pt_ptr_index;
  static bool _got_pt_ptr_index;
};

#include "threadWin32Impl.I"

#endif  // THREAD_WIN32_IMPL

#endif
