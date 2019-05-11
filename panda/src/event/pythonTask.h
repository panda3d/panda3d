/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonTask.h
 * @author drose
 * @date 2008-09-16
 */

#ifndef PYTHONTASK_H
#define PYTHONTASK_H

#include "pandabase.h"

#include "asyncTask.h"

#ifdef HAVE_PYTHON
#include "py_panda.h"
#include "extension.h"

/**
 * This class exists to allow association of a Python function or coroutine
 * with the AsyncTaskManager.
 */
class PythonTask final : public AsyncTask {
PUBLISHED:
  PythonTask(PyObject *function = Py_None, const std::string &name = std::string());
  virtual ~PythonTask();
  ALLOC_DELETED_CHAIN(PythonTask);

  void set_function(PyObject *function);
  INLINE PyObject *get_function();

  void set_args(PyObject *args, bool append_task);
  PyObject *get_args();

  void set_upon_death(PyObject *upon_death);
  INLINE PyObject *get_upon_death();

  void set_owner(PyObject *owner);
  INLINE PyObject *get_owner() const;

  INLINE void set_result(PyObject *result);

public:
  // This is exposed only for the result() function in asyncFuture_ext.cxx
  // to use, which is why it is not published.
  PyObject *get_result() const;
  //PyObject *exception() const;

PUBLISHED:
  int __setattr__(PyObject *self, PyObject *attr, PyObject *v);
  int __delattr__(PyObject *self, PyObject *attr);
  PyObject *__getattribute__(PyObject *self, PyObject *attr) const;

  int __traverse__(visitproc visit, void *arg);
  int __clear__();

PUBLISHED:
  // The amount of seconds that have elapsed since the task was started,
  // according to the task manager's clock.
  MAKE_PROPERTY(time, get_elapsed_time);

  // If this task has been added to an AsyncTaskManager with a delay in
  // effect, this contains the time at which the task is expected to awaken.
  // It has no meaning of the task has not yet been added to a queue, or if
  // there was no delay in effect at the time the task was added.  If the
  // task's status is not S_sleeping, this contains 0.0.
  MAKE_PROPERTY(wake_time, get_wake_time);

  // Alias of wake_time.
  MAKE_PROPERTY(wakeTime, get_wake_time);

  // The delay value that has been set on this task, if any, or None.
  MAKE_PROPERTY2(delay_time, has_delay, get_delay, set_delay, clear_delay);

  // Alias of delay_time.
  MAKE_PROPERTY2(delayTime, has_delay, get_delay, set_delay, clear_delay);

  // The number of frames that have elapsed since the task was started,
  // according to the task manager's clock.
  MAKE_PROPERTY(frame, get_elapsed_frames);

  // This is a special variable to hold the instance dictionary in which
  // custom variables may be stored.
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
  PyObject *_upon_death;
  PyObject *_owner;

  PyObject *_exception;
  PyObject *_exc_value;
  PyObject *_exc_traceback;

  PyObject *_generator;
  PyObject *_future_done;

  bool _append_task;
  bool _ignore_return;
  bool _registered_to_owner;
  mutable bool _retrieved_exception;

  friend class Extension<AsyncFuture>;

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
