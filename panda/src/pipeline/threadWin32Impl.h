// Filename: threadWin32Impl.h
// Created by:  drose (07Feb06)
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

////////////////////////////////////////////////////////////////////
//       Class : ThreadWin32Impl
// Description : Uses native Windows calls to implement a thread.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ThreadWin32Impl {
public:
  INLINE ThreadWin32Impl(Thread *parent_obj);
  ~ThreadWin32Impl();

  bool start(ThreadPriority priority, bool global, bool joinable);
  void interrupt();
  void join();

  INLINE static void prepare_for_exit();

  INLINE static Thread *get_current_thread();
  INLINE static void bind_thread(Thread *thread);
  INLINE static bool is_threading_supported();
  INLINE static void sleep(double seconds);

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
