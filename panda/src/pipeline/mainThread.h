// Filename: mainThread.h
// Created by:  drose (15Jan06)
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

#ifndef MAINTHREAD_H
#define MAINTHREAD_H

#include "pandabase.h"
#include "thread.h"

////////////////////////////////////////////////////////////////////
//       Class : MainThread
// Description : The special "main thread" class.  There is one
//               instance of these in the world, and it is returned by
//               Thread::get_main_thread().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE MainThread : public Thread {
private:
  MainThread();
  virtual void thread_main();

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Thread::init_type();
    register_type(_type_handle, "MainThread",
                  Thread::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class Thread;
};

#endif
