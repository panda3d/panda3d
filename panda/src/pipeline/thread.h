/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file thread.h
 * @author cary
 * @date 1998-09-16
 */

#ifndef THREAD_H
#define THREAD_H

#include "pandabase.h"
#include "namable.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "threadPriority.h"
#include "threadImpl.h"
#include "pnotify.h"
#include "config_pipeline.h"

#ifdef ANDROID
typedef struct _JNIEnv JNIEnv;
#endif

class Mutex;
class ReMutex;
class MutexDebug;
class ConditionVarDebug;
class AsyncTask;

/**
 * A thread; that is, a lightweight process.  This is an abstract base class;
 * to use it, you must subclass from it and redefine thread_main().
 *
 * The thread itself will keep a reference count on the Thread object while it
 * is running; when the thread returns from its root function, the Thread
 * object will automatically be destructed if no other pointers are
 * referencing it.
 */
class EXPCL_PANDA_PIPELINE Thread : public TypedReferenceCount, public Namable {
protected:
  Thread(const std::string &name, const std::string &sync_name);
  Thread(const Thread &copy) = delete;

PUBLISHED:
  virtual ~Thread();

protected:
  Thread &operator = (const Thread &copy) = delete;

  virtual void thread_main()=0;

PUBLISHED:
  static PT(Thread) bind_thread(const std::string &name, const std::string &sync_name);

  INLINE const std::string &get_sync_name() const;

  INLINE int get_pstats_index() const;
  INLINE int get_python_index() const;
  INLINE std::string get_unique_id() const;

  INLINE int get_pipeline_stage() const;
  void set_pipeline_stage(int pipeline_stage);
  INLINE void set_min_pipeline_stage(int min_pipeline_stage);

  INLINE static Thread *get_main_thread();
  INLINE static Thread *get_external_thread();
  INLINE static Thread *get_current_thread();
  INLINE static int get_current_pipeline_stage();
  INLINE static bool is_threading_supported();
  INLINE static bool is_true_threads();
  INLINE static bool is_simple_threads();
  BLOCKING INLINE static void sleep(double seconds);

  BLOCKING INLINE static void force_yield();
  BLOCKING INLINE static void consider_yield();

  virtual void output(std::ostream &out) const;
  void output_blocker(std::ostream &out) const;
  static void write_status(std::ostream &out);

  INLINE bool is_started() const;
  INLINE bool is_joinable() const;

  bool start(ThreadPriority priority, bool joinable);
  BLOCKING INLINE void join();
  INLINE void preempt();

  INLINE TypedReferenceCount *get_current_task() const;

  INLINE void set_python_index(int index);

  INLINE static void prepare_for_exit();

  MAKE_PROPERTY(sync_name, get_sync_name);
  MAKE_PROPERTY(pstats_index, get_pstats_index);
  MAKE_PROPERTY(python_index, get_python_index);
  MAKE_PROPERTY(unique_id, get_unique_id);
  MAKE_PROPERTY(pipeline_stage, get_pipeline_stage, set_pipeline_stage);

  MAKE_PROPERTY(main_thread, get_main_thread);
  MAKE_PROPERTY(external_thread, get_external_thread);
  MAKE_PROPERTY(current_thread, get_current_thread);
  MAKE_PROPERTY(current_pipeline_stage, get_current_pipeline_stage);

  MAKE_PROPERTY(threading_supported, is_threading_supported);
  MAKE_PROPERTY(true_threads, is_true_threads);
  MAKE_PROPERTY(simple_threads, is_simple_threads);

  MAKE_PROPERTY(started, is_started);
  MAKE_PROPERTY(joinable, is_joinable);
  MAKE_PROPERTY(current_task, get_current_task);

public:
  // This class allows integration with PStats, particularly in the
  // SIMPLE_THREADS case.
  class EXPCL_PANDA_PIPELINE PStatsCallback {
  public:
    virtual ~PStatsCallback();
    virtual void deactivate_hook(Thread *thread);
    virtual void activate_hook(Thread *thread);
  };

  INLINE void set_pstats_index(int pstats_index);
  INLINE void set_pstats_callback(PStatsCallback *pstats_callback);
  INLINE PStatsCallback *get_pstats_callback() const;

#ifdef ANDROID
  INLINE JNIEnv *get_jni_env() const;
#endif

private:
  static void init_main_thread();
  static void init_external_thread();

protected:
  bool _started;

private:
  std::string _sync_name;
  ThreadImpl _impl;
  int _pstats_index;
  int _pipeline_stage;
  PStatsCallback *_pstats_callback;
  bool _joinable;
  AtomicAdjust::Pointer _current_task;

  int _python_index;

#ifdef DEBUG_THREADS
  MutexDebug *_blocked_on_mutex;
  ConditionVarDebug *_waiting_on_cvar;
#endif  // DEBUG_THREADS

private:
  static Thread *_main_thread;
  static Thread *_external_thread;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    Namable::init_type(),
    register_type(_type_handle, "Thread",
                  TypedReferenceCount::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class MutexDebug;
  friend class ConditionVarDebug;

  friend class ThreadDummyImpl;
  friend class ThreadWin32Impl;
  friend class ThreadPosixImpl;
  friend class ThreadSimpleImpl;
  friend class MainThread;
  friend class AsyncTask;
};

INLINE std::ostream &operator << (std::ostream &out, const Thread &thread);

#include "thread.I"

#endif
