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
#include "threadPriority.h"
#include "threadImpl.h"
#include "notify.h"

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
class EXPCL_PANDAEXPRESS Thread : public TypedReferenceCount {
public:
  INLINE Thread(const string &name);
  virtual ~Thread();

private:
  INLINE Thread(const Thread &copy);
  INLINE void operator = (const Thread &copy);

protected:
  virtual void thread_main()=0;

public:
  INLINE const string &get_name() const;

  INLINE bool start(ThreadPriority priority, bool global, bool joinable);
  INLINE void interrupt();
  INLINE void join();

  INLINE static void prepare_for_exit();

  INLINE static Thread *get_current_thread();
  INLINE static bool is_threading_supported();
  INLINE static void sleep(double seconds);

  virtual void output(ostream &out) const;

private:
  bool _started;
  string _name;
  ThreadImpl _impl;
  friend class ThreadDummyImpl;
  friend class ThreadNsprImpl;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "Thread",
                  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const Thread &thread);

#include "thread.I"

#endif
