// Filename: threadSimpleImpl.h
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

#ifndef THREADSIMPLEIMPL_H
#define THREADSIMPLEIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "pnotify.h"
#include "threadPriority.h"
#include "pvector.h"

#ifdef HAVE_PYTHON

#undef _POSIX_C_SOURCE
#include <Python.h>

#endif  // HAVE_PYTHON

#ifdef HAVE_UCONTEXT_H
// We'd prefer to use getcontext() / setcontext() to portably change
// execution contexts within C code.  That's what these library
// functions are designed for.
#include <ucontext.h>

#else
// Unfortunately, setcontext() is not defined everywhere (even though
// it claims to be adopted by Posix).  So we have to fall back to
// setjmp() / longjmp() in its absence.  This is a hackier solution.
#include <setjmp.h>

// Ideally, setjmp.h would have defined JB_SP, which will tell us
// where in the context structure we can muck with the stack pointer.
// If it didn't define this symbol, we have to guess it.
#ifndef JB_SP

#if defined(IS_OSX) && defined(__i386__)
// We have determined this value empirically, via test_setjmp.cxx in
// this directory.
#define JB_SP 9

#elif defined(WIN32)
// We have determined this value empirically, via test_setjmp.cxx in
// this directory.
#define JB_SP 4

#endif

#endif  // JB_SP

#endif  // HAVE_UCONTEXT_H

class Thread;
class ThreadSimpleManager;
class MutexSimpleImpl;

////////////////////////////////////////////////////////////////////
//       Class : ThreadSimpleImpl
// Description : This is a trivial threading implementation for
//               applications that don't desire full OS-managed
//               threading.  It is a user-space implementation of
//               threads implemented via setjmp/longjmp, and therefore
//               it cannot take advantage of multiple CPU's (the
//               application will always run on a single CPU,
//               regardless of the number of threads you spawn).
//
//               However, since context switching is entirely
//               cooperative, synchronization primitives like mutexes
//               and condition variables aren't necessary, and the
//               Mutex and ConditionVar classes are compiled into
//               trivial no-op classes, which can reduce overhead
//               substantially compared to a truly threaded
//               application.
//
//               Be sure that every thread calls
//               Thread::consider_yield() occasionally, or it will
//               starve the rest of the running threads.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ThreadSimpleImpl {
public:
  ThreadSimpleImpl(Thread *parent_obj);
  ~ThreadSimpleImpl();

  void setup_main_thread();
  bool start(ThreadPriority priority, bool joinable);
  void join();
  void preempt();

  static void prepare_for_exit();

  INLINE static Thread *get_current_thread();
  INLINE static void bind_thread(Thread *thread);
  INLINE static bool is_threading_supported();
  INLINE static void sleep(double seconds);
  INLINE static void yield();
  INLINE static void consider_yield();

  void sleep_this(double seconds);
  void yield_this();
  INLINE void consider_yield_this();

  INLINE double get_start_time() const;

private:
  void setup_context();
  static void *setup_context_2(ThreadSimpleImpl *self);
  void setup_context_3();

private:
  enum Status {
    S_new,
    S_running,
    S_finished,
    S_killed,
  };

  Thread *_parent_obj;
  bool _joinable;
  Status _status;

  // The (approx) amount of time this thread is allowed to run each
  // epoch.
  double _time_per_epoch;

  // This serves both as the time at which the current thread started
  // to run, and also records the time at which a sleeping thread
  // should wake up.
  double _start_time;

#ifdef HAVE_UCONTEXT_H
  ucontext_t _ucontext;
#else
  jmp_buf _jmp_context;
#endif  // HAVE_UCONTEXT_H

  unsigned char *_stack;
  size_t _stack_size;

#ifdef HAVE_PYTHON
  // If we might be working with Python, we have to manage the Python
  // thread state as we switch contexts.
  PyThreadState *_python_state;
#endif  // HAVE_PYTHON

  // Threads that are waiting for this thread to finish.
  typedef pvector<ThreadSimpleImpl *> JoiningThreads;
  JoiningThreads _joining_threads;

  ThreadSimpleManager *_manager;
  static ThreadSimpleImpl *volatile _st_this;

  friend class ThreadSimpleManager;
};

// We include this down here to avoid the circularity problem.
/* okcircular */
#include "threadSimpleManager.h"

#include "threadSimpleImpl.I"

#endif // THREAD_SIMPLE_IMPL

#endif
