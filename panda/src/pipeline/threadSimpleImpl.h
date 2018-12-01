/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadSimpleImpl.h
 * @author drose
 * @date 2007-06-18
 */

#ifndef THREADSIMPLEIMPL_H
#define THREADSIMPLEIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_SIMPLE_IMPL

#include "pnotify.h"
#include "threadPriority.h"
#include "pvector.h"
#include "contextSwitch.h"

class Thread;
class ThreadSimpleManager;
class MutexSimpleImpl;

/**
 * This is a trivial threading implementation for applications that don't
 * desire full OS-managed threading.  It is a user-space implementation of
 * threads implemented via setjmp/longjmp, and therefore it cannot take
 * advantage of multiple CPU's (the application will always run on a single
 * CPU, regardless of the number of threads you spawn).
 *
 * However, since context switching is entirely cooperative, synchronization
 * primitives like mutexes and condition variables aren't necessary, and the
 * Mutex and ConditionVar classes are compiled into trivial no-op classes,
 * which can reduce overhead substantially compared to a truly threaded
 * application.
 *
 * Be sure that every thread calls Thread::consider_yield() occasionally, or
 * it will starve the rest of the running threads.
 */
class EXPCL_PANDA_PIPELINE ThreadSimpleImpl {
public:
  ThreadSimpleImpl(Thread *parent_obj);
  ~ThreadSimpleImpl();

  void setup_main_thread();
  bool start(ThreadPriority priority, bool joinable);
  void join();
  void preempt();

  std::string get_unique_id() const;

  static void prepare_for_exit();

  INLINE static Thread *get_current_thread();
  INLINE bool is_same_system_thread() const;

  INLINE static void bind_thread(Thread *thread);
  INLINE static bool is_threading_supported();
  static bool is_true_threads();
  INLINE static bool is_simple_threads();
  INLINE static void sleep(double seconds);
  INLINE static void yield();
  INLINE static void consider_yield();

  void sleep_this(double seconds);
  void yield_this(bool volunteer);
  INLINE void consider_yield_this();

  INLINE double get_wake_time() const;

  INLINE static void write_status(std::ostream &out);

private:
  static void st_begin_thread(void *data);
  void begin_thread();

private:
  enum ThreadStatus {
    TS_new,
    TS_running,
    TS_finished,
    TS_killed,
  };

  static int _next_unique_id;
  int _unique_id;
  Thread *_parent_obj;
  bool _joinable;
  ThreadStatus _status;
  ThreadPriority _priority;

  // The relative weight of this thread, relative to other threads, in
  // priority.
  double _priority_weight;

  // The amount of time this thread has run recently.
  unsigned int _run_ticks;

  // This is the time at which the currently-running thread started execution.
  double _start_time;

  // This is the time at which the currently-running thread should yield.
  double _stop_time;

  // This records the time at which a sleeping thread should wake up.
  double _wake_time;

  ThreadContext *_context;
  unsigned char *_stack;
  size_t _stack_size;

#ifdef HAVE_PYTHON
  // If we might be working with Python, we have to manage the Python thread
  // state as we switch contexts.
  PyThreadState *_python_state;
#endif  // HAVE_PYTHON

  // Threads that are waiting for this thread to finish.
  typedef pvector<ThreadSimpleImpl *> JoiningThreads;
  JoiningThreads _joining_threads;

  ThreadSimpleManager *_manager;
  static ThreadSimpleImpl *volatile _st_this;

  // We may not mix-and-match OS threads with Panda's SIMPLE_THREADS. If we
  // ever get a Panda context switch request from a different OS thread than
  // the thread we think we should be in, that's a serious error that may
  // cause major consequences.  For this reason, we store the OS thread's
  // current thread ID here when the thread is constructed, and insist that it
  // never changes during the lifetime of the thread.
#ifdef HAVE_POSIX_THREADS
  pthread_t _posix_system_thread_id;
#endif
#ifdef WIN32
  DWORD _win32_system_thread_id;
#endif

  friend class ThreadSimpleManager;
};

// We include this down here to avoid the circularity problem.
/* okcircular */
#include "threadSimpleManager.h"

#include "threadSimpleImpl.I"

#endif // THREAD_SIMPLE_IMPL

#endif
