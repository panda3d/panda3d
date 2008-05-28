// Filename: thread.h
// Created by:  cary (16Sep98)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef THREAD_H
#define THREAD_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "threadPriority.h"
#include "threadImpl.h"
#include "pnotify.h"
#include "config_pipeline.h"

class Mutex;
class ReMutex;
class MutexDebug;

////////////////////////////////////////////////////////////////////
//       Class : Thread
// Description : A thread; that is, a lightweight process.  This is an
//               abstract base class; to use it, you must subclass
//               from it and redefine thread_main().
//
//               The thread itself will keep a reference count on the
//               Thread object while it is running; when the thread
//               returns from its root function, the Thread object
//               will automatically be destructed if no other pointers
//               are referencing it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE Thread : public TypedReferenceCount {
protected:
  INLINE Thread(const string &name, const string &sync_name);

PUBLISHED:
  virtual ~Thread();

private:
  INLINE Thread(const Thread &copy);
  INLINE void operator = (const Thread &copy);

protected:
  virtual void thread_main()=0;

PUBLISHED:
  static PT(Thread) bind_thread(const string &name, const string &sync_name);

  INLINE const string &get_name() const;
  INLINE const string &get_sync_name() const;

  INLINE int get_pstats_index() const;

  INLINE int get_pipeline_stage() const;
  void set_pipeline_stage(int pipeline_stage);
  INLINE void set_min_pipeline_stage(int min_pipeline_stage);

  INLINE static Thread *get_main_thread();
  INLINE static Thread *get_external_thread();
  INLINE static Thread *get_current_thread();
  INLINE static int get_current_pipeline_stage();
  INLINE static bool is_threading_supported();
  INLINE static bool is_true_threads();
  BLOCKING INLINE static void sleep(double seconds);

  BLOCKING INLINE static void force_yield();
  BLOCKING INLINE static void consider_yield();

  virtual void output(ostream &out) const;
  static void write_status(ostream &out);

  INLINE bool is_started() const;

  bool start(ThreadPriority priority, bool joinable);
  BLOCKING INLINE void join();
  INLINE void preempt();

  INLINE static void prepare_for_exit();

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

private:
  static void init_main_thread();
  static void init_external_thread();

protected:
  bool _started;

private:
  string _name;
  string _sync_name;
  ThreadImpl _impl;
  int _pstats_index;
  int _pipeline_stage;
  PStatsCallback *_pstats_callback;

#ifdef DEBUG_THREADS
  MutexDebug *_blocked_on_mutex;
#endif

private:
  static Thread *_main_thread;
  static Thread *_external_thread;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "Thread",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class MutexDebug;

  friend class ThreadDummyImpl;
  friend class ThreadWin32Impl;
  friend class ThreadPosixImpl;
  friend class ThreadSimpleImpl;
  friend class MainThread;
};

INLINE ostream &operator << (ostream &out, const Thread &thread);

#include "thread.I"

#endif
