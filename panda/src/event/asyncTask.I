// Filename: asyncTask.I
// Created by:  drose (23Aug06)
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


////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AsyncTask::
AsyncTask(const string &name) : 
  Namable(name),
  _delay(0.0),
  _has_delay(false),
  _wake_time(0.0),
  _sort(0),
  _priority(0),
  _state(S_inactive),
  _servicing_thread(NULL),
  _manager(NULL),
  _chain(NULL)
{
#ifdef HAVE_PYTHON
  _python_object = NULL;
#endif  // HAVE_PYTHON
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_state
//       Access: Published
//  Description: Returns the current state of the task.
////////////////////////////////////////////////////////////////////
INLINE AsyncTask::State AsyncTask::
get_state() const {
  return _state;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_manager
//       Access: Published
//  Description: Returns the AsyncTaskManager that this task is active
//               on.  This will be NULL if the state is S_inactive.
////////////////////////////////////////////////////////////////////
INLINE AsyncTaskManager *AsyncTask::
get_manager() const {
  return _manager;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::set_delay
//       Access: Published
//  Description: Specifies the amount of time, in seconds, by which
//               this task will be delayed after it has been added to
//               the AsyncTaskManager.  At least the specified amount
//               of time (and possibly more) will elapse before the
//               task begins.
//
//               You may specify a delay of 0.0 to guarantee that the
//               task will run in the next epoch following the one in
//               which it is added.
//
//               Setting this value after the task has already been
//               added will not affect the task's wake time; it will
//               only affect the task if the it is re-added to the
//               queue in the future.
////////////////////////////////////////////////////////////////////
INLINE void AsyncTask::
set_delay(double delay) {
  _delay = delay;
  _has_delay = true;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::clear_delay
//       Access: Published
//  Description: Removes any delay specified for the task.  The next
//               time the task is added to the queue, it will run
//               immediately.  This does not affect the task's wake
//               time if it has already been added to the queue.
////////////////////////////////////////////////////////////////////
INLINE void AsyncTask::
clear_delay() {
  _delay = 0.0;
  _has_delay = false;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::has_delay
//       Access: Published
//  Description: Returns true if a delay has been set for this task
//               via set_delay(), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool AsyncTask::
has_delay() const {
  return _has_delay;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_delay
//       Access: Published
//  Description: Returns the delay value that has been set via
//               set_delay, if any.
////////////////////////////////////////////////////////////////////
INLINE double AsyncTask::
get_delay() const {
  return _delay;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_wake_time
//       Access: Published
//  Description: If this task has been added to an AsyncTaskManager
//               with a delay in effect, this returns the time at
//               which the task is expected to awaken.  It has no
//               meaning if the task has not yet been added to a
//               queue, or if there was no delay in effect at the time
//               the task was added.
//
//               It is only valid to call this if the task's status is
//               S_sleeping.
////////////////////////////////////////////////////////////////////
INLINE double AsyncTask::
get_wake_time() const {
  nassertr(_state == S_sleeping, 0.0);
  return _wake_time;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_start_time
//       Access: Published
//  Description: Returns the time at which the task was started,
//               according to the task manager's clock.
//
//               It is only valid to call this if the task's status is
//               not S_inactive.
////////////////////////////////////////////////////////////////////
INLINE double AsyncTask::
get_start_time() const {
  nassertr(_state != S_inactive, 0.0);
  return _start_time;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::clear_name
//       Access: Public
//  Description: Resets the task's name to empty.
////////////////////////////////////////////////////////////////////
INLINE void AsyncTask::
clear_name() {
  set_name(string());
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_task_chain
//       Access: Published
//  Description: Returns the AsyncTaskChain on which this task will
//               be running.  Each task chain runs tasks independently
//               of the others.
////////////////////////////////////////////////////////////////////
INLINE const string &AsyncTask::
get_task_chain() const {
  return _chain_name;
}


////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_sort
//       Access: Published
//  Description: Returns the task's current sort value.  See
//               set_sort().
////////////////////////////////////////////////////////////////////
INLINE int AsyncTask::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_priority
//       Access: Published
//  Description: Returns the task's current priority value.  See
//               set_priority().
////////////////////////////////////////////////////////////////////
INLINE int AsyncTask::
get_priority() const {
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::set_done_event
//       Access: Published
//  Description: Sets the event name that will be triggered
//               when the task finishes.  This should only be called
//               before the task has been started, or after it has
//               finished and before it is about to be restarted
//               (i.e. when get_state() returns S_inactive).
////////////////////////////////////////////////////////////////////
INLINE void AsyncTask::
set_done_event(const string &done_event) {
  nassertv(_state == S_inactive);
  _done_event = done_event;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_done_event
//       Access: Published
//  Description: Returns the event name that will be triggered
//               when the task finishes.  See set_done_event().
////////////////////////////////////////////////////////////////////
INLINE const string &AsyncTask::
get_done_event() const {
  return _done_event;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::set_python_object
//       Access: Published
//  Description: Specifies an arbitrary Python object that will be
//               piggybacked on the task object.
////////////////////////////////////////////////////////////////////
INLINE void AsyncTask::
set_python_object(PyObject *python_object) {
  Py_XINCREF(python_object);
  Py_XDECREF(_python_object);
  _python_object = python_object;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_python_object
//       Access: Published
//  Description: Returns the Python object that was specified to
//               set_python_object(), if any, or None if no object was
//               specified.
////////////////////////////////////////////////////////////////////
INLINE PyObject *AsyncTask::
get_python_object() const {
  if (_python_object != (PyObject *)NULL) {
    Py_XINCREF(_python_object);
    return _python_object;
  }
  Py_INCREF(Py_None);
  return Py_None;
}
#endif  // HAVE_PYTHON

