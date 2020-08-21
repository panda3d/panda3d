/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskManager.I
 * @author drose
 * @date 2006-08-23
 */

/**
 * Replaces the clock pointer used within the AsyncTaskManager.  This is used
 * to control when tasks with a set_delay() specified will be scheduled.  It
 * can also be ticked automatically each epoch, if set_tick_clock() is true.
 *
 * The default is the global clock pointer.
 */
INLINE void AsyncTaskManager::
set_clock(ClockObject *clock) {
  _clock = clock;
}

/**
 * Returns the clock pointer used within the AsyncTaskManager.  See
 * set_clock().
 */
INLINE ClockObject *AsyncTaskManager::
get_clock() {
  return _clock;
}

/**
 * Returns the number of tasks that are currently active or sleeping within
 * the task manager.
 */
INLINE size_t AsyncTaskManager::
get_num_tasks() const {
  MutexHolder holder(_lock);
  return _num_tasks;
}

/**
 * Returns a pointer to the global AsyncTaskManager.  This is the
 * AsyncTaskManager that most code should use for queueing tasks and suchlike.
 */
INLINE AsyncTaskManager *AsyncTaskManager::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    make_global_ptr();
  }
  return _global_ptr;
}

/**
 * Adds the task to the _tasks_by_name index, if it has a nonempty name.
 */
INLINE void AsyncTaskManager::
add_task_by_name(AsyncTask *task) {
  if (!task->get_name().empty()) {
    _tasks_by_name.insert(task);
  }
}
