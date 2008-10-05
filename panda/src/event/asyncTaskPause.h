// Filename: asyncTaskPause.h
// Created by:  drose (04Oct08)
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

#ifndef ASYNCTASKPAUSE_H
#define ASYNCTASKPAUSE_H

#include "pandabase.h"

#include "asyncTask.h"

class AsyncTaskManager;

////////////////////////////////////////////////////////////////////
//       Class : AsyncTaskPause
// Description : A special kind of task that simple returns DS_pause,
//               to pause for a specified number of seconds and then
//               finish.  It's intended to be used within an
//               AsyncTaskSequence.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_EVENT AsyncTaskPause : public AsyncTask {
PUBLISHED:
  AsyncTaskPause(double delay);
  ALLOC_DELETED_CHAIN(AsyncTaskPause);

protected:
  virtual DoneStatus do_task();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "AsyncTaskPause",
                  AsyncTask::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "asyncTaskPause.I"

#endif

