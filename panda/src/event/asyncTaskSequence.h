// Filename: asyncTaskSequence.h
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

#ifndef ASYNCTASKSEQUENCE_H
#define ASYNCTASKSEQUENCE_H

#include "pandabase.h"

#include "asyncTask.h"
#include "asyncTaskCollection.h"

class AsyncTaskManager;

////////////////////////////////////////////////////////////////////
//       Class : AsyncTaskSequence
// Description : A special kind of task that serves as a list of tasks
//               internally.  Each task on the list is executed in
//               sequence, one per epoch.
//
//               This is similar to a Sequence interval, though it has
//               some slightly different abilities.  For instance,
//               although you can't start at any arbitrary point in
//               the sequence, you can construct a task sequence whose
//               duration changes during playback.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_EVENT AsyncTaskSequence : public AsyncTask, public AsyncTaskCollection {
PUBLISHED:
  AsyncTaskSequence(const string &name);
  virtual ~AsyncTaskSequence();
  ALLOC_DELETED_CHAIN(AsyncTaskSequence);

  INLINE void set_repeat_count(int repeat_count);
  INLINE int get_repeat_count() const;

  INLINE int get_current_task_index() const;

protected:
  virtual bool is_runnable();
  virtual DoneStatus do_task();
  virtual void upon_birth(AsyncTaskManager *manager);
  virtual void upon_death(AsyncTaskManager *manager, bool clean_exit);

private:
  void set_current_task(AsyncTask *task, bool clean_exit);

  int _repeat_count;
  int _task_index;
  PT(AsyncTask) _current_task;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "AsyncTaskSequence",
                  AsyncTask::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "asyncTaskSequence.I"

#endif

