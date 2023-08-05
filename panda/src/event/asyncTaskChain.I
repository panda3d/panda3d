/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskChain.I
 * @author drose
 * @date 2006-08-23
 */

/**
 * Returns true if the thread(s) have been started and are ready to service
 * requests, false otherwise.  If this is false, the next call to add() or
 * add_and_do() will automatically start the threads.
 */
INLINE bool AsyncTaskChain::
is_started() const {
  return (_state == S_started);
}

#ifndef CPPPARSER
/**
 * Adds a new task to the task chain which calls the indicated callable.
 * This method is defined as a more convenient alternative to subclassing
 * AsyncTask.
 *
 * This given callable allowed to be any object defining a call operator that
 * accepts an AsyncTask pointer and returns a DoneStatus.
 *
 * Returns the newly created AsyncTask object.
 *
 * @since 1.11.0
 */
template<class Callable>
INLINE AsyncTask *AsyncTaskChain::
add(Callable callable, const std::string &name, int sort, int priority) {
  class InlineTask final : public AsyncTask {
  public:
    InlineTask(Callable callable, const std::string &name, int sort, int priority) :
      AsyncTask(name),
      _callable(std::move(callable)) {
      _sort = sort;
      _priority = priority;
    }

    ALLOC_DELETED_CHAIN(InlineTask);

  private:
    virtual DoneStatus do_task() override final {
      return _callable(this);
    }

    Callable _callable;
  };
  AsyncTask *task = new InlineTask(std::move(callable), name, sort, priority);
  add(task);
  return task;
}
#endif

/**
 * Returns the time at which the next sleeping thread will awaken, or -1 if
 * there are no sleeping threads.  Assumes the lock is already held.
 */
INLINE double AsyncTaskChain::
do_get_next_wake_time() const {
  if (!_sleeping.empty()) {
    return _sleeping.front()->_wake_time;
  }
  return -1.0;
}

/**
 * Returns the time at which the indicated thread will awaken.  Assumes the
 * lock is already held.
 */
INLINE double AsyncTaskChain::
get_wake_time(AsyncTask *task) {
  return task->_wake_time;
}
