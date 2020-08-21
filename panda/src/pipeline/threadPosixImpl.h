/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadPosixImpl.h
 * @author drose
 * @date 2006-02-09
 */

#ifndef THREADPOSIXIMPL_H
#define THREADPOSIXIMPL_H

#include "pandabase.h"
#include "selectThreadImpl.h"

#ifdef THREAD_POSIX_IMPL

#include "pnotify.h"
#include "threadPriority.h"
#include "mutexPosixImpl.h"

#include <pthread.h>

#ifdef ANDROID
typedef struct _JNIEnv JNIEnv;
#endif

class Thread;

/**
 * Uses Posix threads to implement a thread.
 */
class EXPCL_PANDA_PIPELINE ThreadPosixImpl {
public:
  INLINE ThreadPosixImpl(Thread *parent_obj);
  ~ThreadPosixImpl();

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

#ifdef ANDROID
  INLINE JNIEnv *get_jni_env() const;
  bool attach_java_vm();
  static void bind_java_thread();
#endif

private:
  static void *root_func(void *data);
  static void init_pt_ptr_index();

  // There appears to be a name collision with the word "Status".
  enum PStatus {
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
  PStatus _status;

#ifdef ANDROID
  JNIEnv *_jni_env;
#endif

  static pthread_key_t _pt_ptr_index;
  static bool _got_pt_ptr_index;
};

#include "threadPosixImpl.I"

#endif  // THREAD_POSIX_IMPL

#endif
