// Filename: thread.h
// Created by:  cary (16Sep98)
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
class EXPCL_PANDA Thread : public TypedReferenceCount {
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
  INLINE void set_pstats_index(int pstats_index);

  INLINE int get_pipeline_stage() const;
  void set_pipeline_stage(int pipeline_stage);
  INLINE void set_min_pipeline_stage(int min_pipeline_stage);

  INLINE static Thread *get_main_thread();
  INLINE static Thread *get_external_thread();
  INLINE static Thread *get_current_thread();
  INLINE static int get_current_pipeline_stage();
  INLINE static bool is_threading_supported();
  BLOCKING INLINE static void sleep(double seconds);

  virtual void output(ostream &out) const;

  INLINE bool is_started() const;

  bool start(ThreadPriority priority, bool joinable);
  INLINE void interrupt();
  BLOCKING INLINE void join();

public:
  INLINE static void prepare_for_exit();

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
  friend class ThreadNsprImpl;
};

INLINE ostream &operator << (ostream &out, const Thread &thread);

#include "thread.I"

#endif
