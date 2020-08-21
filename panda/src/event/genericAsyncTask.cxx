/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file genericAsyncTask.cxx
 * @author drose
 * @date 2008-10-03
 */

#include "genericAsyncTask.h"
#include "pnotify.h"

TypeHandle GenericAsyncTask::_type_handle;

/**
 *
 */
GenericAsyncTask::
GenericAsyncTask(const std::string &name) :
  AsyncTask(name)
{
  _function = nullptr;
  _upon_birth = nullptr;
  _upon_death = nullptr;
  _user_data = nullptr;
}

/**
 *
 */
GenericAsyncTask::
GenericAsyncTask(const std::string &name, GenericAsyncTask::TaskFunc *function, void *user_data) :
  AsyncTask(name),
  _function(function),
  _user_data(user_data)
{
  _upon_birth = nullptr;
  _upon_death = nullptr;
}

/**
 * Override this function to return true if the task can be successfully
 * executed, false if it cannot.  Mainly intended as a sanity check when
 * attempting to add the task to a task manager.
 *
 * This function is called with the lock held.
 */
bool GenericAsyncTask::
is_runnable() {
  return _function != nullptr;
}

/**
 * Override this function to do something useful for the task.
 *
 * This function is called with the lock *not* held.
 */
AsyncTask::DoneStatus GenericAsyncTask::
do_task() {
  nassertr(_function != nullptr, DS_interrupt);
  return (*_function)(this, _user_data);
}

/**
 * Override this function to do something useful when the task has been added
 * to the active queue.
 *
 * This function is called with the lock *not* held.
 */
void GenericAsyncTask::
upon_birth(AsyncTaskManager *manager) {
  AsyncTask::upon_birth(manager);

  if (_upon_birth != nullptr) {
    (*_upon_birth)(this, _user_data);
  }
}

/**
 * Override this function to do something useful when the task has been
 * removed from the active queue.  The parameter clean_exit is true if the
 * task has been removed because it exited normally (returning DS_done), or
 * false if it was removed for some other reason (e.g.
 * AsyncTaskManager::remove()).  By the time this method is called, _manager
 * has been cleared, so the parameter manager indicates the original
 * AsyncTaskManager that owned this task.
 *
 * The normal behavior is to throw the done_event only if clean_exit is true.
 *
 * This function is called with the lock *not* held.
 */
void GenericAsyncTask::
upon_death(AsyncTaskManager *manager, bool clean_exit) {
  AsyncTask::upon_death(manager, clean_exit);

  if (_upon_death != nullptr) {
    (*_upon_death)(this, clean_exit, _user_data);
  }
}
