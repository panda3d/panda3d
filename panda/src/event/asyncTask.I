/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTask.I
 * @author drose
 * @date 2006-08-23
 */

/**
 * Returns the current state of the task.
 */
INLINE AsyncTask::State AsyncTask::
get_state() const {
  return _state;
}

/**
 * Returns true if the task is currently active or sleeping on some task
 * chain, meaning that it will be executed in its turn, or false if it is not
 * active.  If the task has recently been removed while it is in the middle of
 * execution, this will return false, because the task will not run again once
 * it finishes.
 */
INLINE bool AsyncTask::
is_alive() const {
  switch (_state) {
  case S_active:
  case S_servicing:
  case S_sleeping:
  case S_active_nested:
  case S_awaiting:
    return true;

  case S_inactive:
  case S_servicing_removed:
    return false;
  }

  // Shouldn't get here.
  return false;
}

/**
 * Returns the AsyncTaskManager that this task is active on.  This will be
 * NULL if the state is S_inactive.
 */
INLINE AsyncTaskManager *AsyncTask::
get_manager() const {
  return _manager;
}

/**
 * Specifies the amount of time, in seconds, by which this task will be
 * delayed after it has been added to the AsyncTaskManager.  At least the
 * specified amount of time (and possibly more) will elapse before the task
 * begins.
 *
 * You may specify a delay of 0.0 to guarantee that the task will run in the
 * next epoch following the one in which it is added.
 *
 * Setting this value after the task has already been added will not affect
 * the task's wake time; it will only affect the task if it is re-added to the
 * queue in the future, for instance if the task returns DS_again.  However,
 * see recalc_wake_time() if you wish to apply the delay effect immediately.
 */
INLINE void AsyncTask::
set_delay(double delay) {
  _delay = delay;
  _has_delay = true;
}

/**
 * Removes any delay specified for the task.  The next time the task is added
 * to the queue, it will run immediately.  This does not affect the task's
 * wake time if it has already been added to the queue.
 */
INLINE void AsyncTask::
clear_delay() {
  _delay = 0.0;
  _has_delay = false;
}

/**
 * Returns true if a delay has been set for this task via set_delay(), or
 * false otherwise.
 */
INLINE bool AsyncTask::
has_delay() const {
  return _has_delay;
}

/**
 * Returns the delay value that has been set via set_delay, if any.
 */
INLINE double AsyncTask::
get_delay() const {
  return _delay;
}

/**
 * Returns the time at which the task was started, according to the task
 * manager's clock.
 *
 * It is only valid to call this if the task's status is not S_inactive.
 */
INLINE double AsyncTask::
get_start_time() const {
  nassertr(_state != S_inactive, 0.0);
  return _start_time;
}

/**
 * Returns the frame number at which the task was started, according to the
 * task manager's clock.
 *
 * It is only valid to call this if the task's status is not S_inactive.
 */
INLINE int AsyncTask::
get_start_frame() const {
  nassertr(_state != S_inactive, 0);
  return _start_frame;
}

/**
 * Resets the task's name to empty.
 */
INLINE void AsyncTask::
clear_name() {
  set_name(std::string());
}

/**
 * Returns a number guaranteed to be unique for each different AsyncTask
 * object in the universe.
 */
INLINE AtomicAdjust::Integer AsyncTask::
get_task_id() const {
  return _task_id;
}

/**
 * Returns the AsyncTaskChain on which this task will be running.  Each task
 * chain runs tasks independently of the others.
 */
INLINE const std::string &AsyncTask::
get_task_chain() const {
  return _chain_name;
}


/**
 * Returns the task's current sort value.  See set_sort().
 */
INLINE int AsyncTask::
get_sort() const {
  return _sort;
}

/**
 * Returns the task's current priority value.  See set_priority().
 */
INLINE int AsyncTask::
get_priority() const {
  return _priority;
}

/**
 * Sets the event name that will be triggered when the task finishes.  This
 * should only be called before the task has been started, or after it has
 * finished and before it is about to be restarted (i.e.  when get_state()
 * returns S_inactive).
 */
INLINE void AsyncTask::
set_done_event(const std::string &done_event) {
  nassertv(_state == S_inactive);
  _done_event = done_event;
}

/**
 * Returns the amount of time elapsed during the task's previous run cycle, in
 * seconds.
 */
INLINE double AsyncTask::
get_dt() const {
  return _dt;
}

/**
 * Returns the maximum amount of time elapsed during any one of the task's
 * previous run cycles, in seconds.
 */
INLINE double AsyncTask::
get_max_dt() const {
  return _max_dt;
}

/**
 * Returns the average amount of time elapsed during each of the task's
 * previous run cycles, in seconds.
 */
INLINE double AsyncTask::
get_average_dt() const {
  if (_num_frames == 0) {
    return 0.0;
  } else {
    return _total_dt / _num_frames;
  }
}
