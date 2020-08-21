/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskSequence.cxx
 * @author drose
 * @date 2008-10-04
 */

#include "asyncTaskSequence.h"
#include "asyncTaskManager.h"

TypeHandle AsyncTaskSequence::_type_handle;

/**
 *
 */
AsyncTaskSequence::
AsyncTaskSequence(const std::string &name) :
  AsyncTask(name),
  _repeat_count(0),
  _task_index(0)
{
}

/**
 *
 */
AsyncTaskSequence::
~AsyncTaskSequence() {
  set_current_task(nullptr, true);
}

/**
 * Override this function to return true if the task can be successfully
 * executed, false if it cannot.  Mainly intended as a sanity check when
 * attempting to add the task to a task manager.
 *
 * This function is called with the lock held.
 */
bool AsyncTaskSequence::
is_runnable() {
  return (get_num_tasks() > 0);
}

/**
 * Override this function to do something useful for the task.
 *
 * This function is called with the lock *not* held.
 */
AsyncTask::DoneStatus AsyncTaskSequence::
do_task() {
  // Clear the delay that might have been set from a previous wait.
  _delay = 0.0;
  _has_delay = false;

  if (_task_index >= get_num_tasks()) {
    // Ran off the end of the task list.
    set_current_task(nullptr, true);
    _task_index = 0;
    if (_task_index >= get_num_tasks()) {
      return DS_done;
    }
    if (_repeat_count > 0) {
      --_repeat_count;
    }
    if (_repeat_count != 0) {
      return DS_cont;
    }
    return DS_done;
  }

  AsyncTask *task = get_task(_task_index);
  set_current_task(task, true);
  nassertr(_current_task != nullptr, DS_exit);

  DoneStatus result = _current_task->do_task();
  switch (result) {
  case DS_again:
  case DS_pause:
    // The task wants to sleep for a period of time.
    {
      double now = _manager->_clock->get_frame_time();
      _current_task->_start_time = now + _current_task->_delay;

      _delay = _current_task->_delay;
      _has_delay = _current_task->_has_delay;

      if (result == DS_pause) {
        // When it wakes up, move on to the next task.
        ++_task_index;
      }
    }
    return DS_again;

  case DS_done:
    // Go on to the next task.
    ++_task_index;
    return DS_cont;

  case DS_cont:
  case DS_pickup:
  case DS_exit:
  case DS_interrupt:
  case DS_await:
    // Just return these results through.
    return result;
  }

  // Shouldn't get here.
  nassertr(false, DS_exit);
  return DS_exit;
}

/**
 * Override this function to do something useful when the task has been added
 * to the active queue.
 *
 * This function is called with the lock held.  You may temporarily release if
 * it necessary, but be sure to return with it held.
 */
void AsyncTaskSequence::
upon_birth(AsyncTaskManager *manager) {
  AsyncTask::upon_birth(manager);
  _task_index = 0;
  set_current_task(nullptr, true);
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
 * This function is called with the lock held.  You may temporarily release if
 * it necessary, but be sure to return with it held.
 */
void AsyncTaskSequence::
upon_death(AsyncTaskManager *manager, bool clean_exit) {
  AsyncTask::upon_death(manager, clean_exit);
  set_current_task(nullptr, clean_exit);
}

/**
 * Sets the current executing task.  If NULL, sets the current task to NULL.
 */
void AsyncTaskSequence::
set_current_task(AsyncTask *task, bool clean_exit) {
  if (_current_task == task) {
    return;
  }

  if (_current_task != nullptr) {
    nassertv(_current_task->_state == S_active_nested);
    nassertv(_current_task->_manager == _manager || _manager == nullptr);
    _current_task->_state = S_inactive;
    _current_task->_manager = nullptr;
    _current_task->upon_death(_manager, clean_exit);
  }

  _current_task = task;

  if (_current_task != nullptr) {
    nassertv(_current_task->_state == S_inactive);
    nassertv(_current_task->_manager == nullptr);
    _current_task->upon_birth(_manager);
    nassertv(_current_task->_state == S_inactive);
    nassertv(_current_task->_manager == nullptr);
    _current_task->_manager = _manager;
    _current_task->_state = S_active_nested;

    double now = _manager->_clock->get_frame_time();
    task->_start_time = now;
    task->_start_frame = _manager->_clock->get_frame_count();
  }
}
