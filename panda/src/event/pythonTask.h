// Filename: pythonTask.h
// Created by:  drose (16Sep08)
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

#ifndef PYTHONTASK_H
#define PYTHONTASK_H

#include "pandabase.h"

#include "asyncTask.h"

#ifdef HAVE_PYTHON
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : PythonTask
// Description : This class exists to allow association of a Python
//               function with the AsyncTaskManager.
////////////////////////////////////////////////////////////////////
class PythonTask : public AsyncTask {
PUBLISHED:
  PythonTask(PyObject *function = Py_None, const string &name = string());
  virtual ~PythonTask();
  ALLOC_DELETED_CHAIN(PythonTask);

  void set_function(PyObject *function);
  PyObject *get_function();

  void set_args(PyObject *args, bool append_task);
  PyObject *get_args();

  void set_upon_death(PyObject *upon_death);
  PyObject *get_upon_death();

  void set_owner(PyObject *owner);
  PyObject *get_owner();

  int __setattr__(PyObject *self, PyObject *attr, PyObject *v);
  int __delattr__(PyObject *self, PyObject *attr);
  PyObject *__getattr__(PyObject *attr) const;

  int __traverse__(visitproc visit, void *arg);
  int __clear__();

  INLINE void set_delay(PyObject *delay);
  INLINE PyObject *get_delay() const;

PUBLISHED:
  // The name of this task.
  MAKE_PROPERTY(name, get_name, set_name);

  // The amount of seconds that have elapsed since the task was
  // started, according to the task manager's clock.
  MAKE_PROPERTY(time, get_elapsed_time);

  // If this task has been added to an AsyncTaskManager with a delay
  // in effect, this contains the time at which the task is expected
  // to awaken.  It has no meaning of the task has not yet been added
  // to a queue, or if there was no delay in effect at the time the
  // task was added.
  //
  // If the task's status is not S_sleeping, this contains 0.0.
  MAKE_PROPERTY(wake_time, get_wake_time);

  // The delay value that has been set on this task, if any, or None.
  MAKE_PROPERTY(delay_time, get_delay, set_delay);

  // The number of frames that have elapsed since the task was
  // started, according to the task manager's clock.
  MAKE_PROPERTY(frame, get_elapsed_frames);

  // This is a number guaranteed to be unique for each different
  // AsyncTask object in the universe.
  MAKE_PROPERTY(id, get_task_id);

  // This is a special variable to hold the instance dictionary in
  // which custom variables may be stored.
  PyObject *__dict__;

protected:
  virtual bool is_runnable();
  virtual DoneStatus do_task();
  DoneStatus do_python_task();
  virtual void upon_birth(AsyncTaskManager *manager);
  virtual void upon_death(AsyncTaskManager *manager, bool clean_exit);

private:
  void register_to_owner();
  void unregister_from_owner();
  void call_owner_method(const char *method_name);
  void call_function(PyObject *function);

private:
  PyObject *_function;
  PyObject *_args;
  bool _append_task;
  PyObject *_upon_death;
  PyObject *_owner;
  bool _registered_to_owner;

  PyObject *_generator;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "PythonTask",
                  AsyncTask::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pythonTask.I"

#endif  // HAVE_PYTHON

#endif

