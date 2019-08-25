/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskChain.cxx
 * @author drose
 * @date 2006-08-23
 */

#include "asyncTaskChain.h"
#include "asyncTaskManager.h"
#include "event.h"
#include "mutexHolder.h"
#include "indent.h"
#include "pStatClient.h"
#include "pStatTimer.h"
#include "clockObject.h"
#include "config_event.h"
#include <algorithm>
#include <stdio.h>  // For sprintf/snprintf

using std::max;
using std::ostream;
using std::ostringstream;
using std::string;

TypeHandle AsyncTaskChain::_type_handle;

PStatCollector AsyncTaskChain::_task_pcollector("Task");
PStatCollector AsyncTaskChain::_wait_pcollector("Wait");

/**
 *
 */
AsyncTaskChain::
AsyncTaskChain(AsyncTaskManager *manager, const string &name) :
  Namable(name),
  _manager(manager),
  _cvar(manager->_lock),
  _tick_clock(false),
  _timeslice_priority(false),
  _num_threads(0),
  _thread_priority(TP_normal),
  _frame_budget(-1.0),
  _frame_sync(false),
  _num_busy_threads(0),
  _num_tasks(0),
  _num_awaiting_tasks(0),
  _state(S_initial),
  _current_sort(-INT_MAX),
  _pickup_mode(false),
  _needs_cleanup(false),
  _current_frame(0),
  _time_in_frame(0.0),
  _block_till_next_frame(false),
  _next_implicit_sort(0)
{
}

/**
 *
 */
AsyncTaskChain::
~AsyncTaskChain() {
  // We only grab the lock if _needs_cleanup is true.  This way, the temporary
  // AsyncTaskChain objects created (and destructed) within the task manager
  // won't risk a double-lock.
  if (_needs_cleanup) {
    MutexHolder holder(_manager->_lock);
    do_cleanup();
  }
}

/**
 * Sets the tick_clock flag.  When this is true, get_clock()->tick() will be
 * called automatically at each task epoch.  This is false by default.
 */
void AsyncTaskChain::
set_tick_clock(bool tick_clock) {
  MutexHolder holder(_manager->_lock);
  _tick_clock = tick_clock;
}

/**
 * Returns the tick_clock flag.  See set_tick_clock().
 */
bool AsyncTaskChain::
get_tick_clock() const {
  MutexHolder holder(_manager->_lock);
  return _tick_clock;
}

/**
 * Changes the number of threads for this task chain.  This may require
 * stopping the threads if they are already running.
 */
void AsyncTaskChain::
set_num_threads(int num_threads) {
  nassertv(num_threads >= 0);

  if (task_cat.is_debug()) {
    do_output(task_cat.debug());
    task_cat.debug(false)
      << ": set_num_threads(" << num_threads << ")\n";
  }

  if (!Thread::is_threading_supported()) {
    num_threads = 0;
  }

  MutexHolder holder(_manager->_lock);
  if (_num_threads != num_threads) {
    do_stop_threads();
    _num_threads = num_threads;

    if (_num_tasks != 0) {
      do_start_threads();
    }
  }
}

/**
 * Returns the number of threads that will be servicing tasks for this chain.
 * Also see get_num_running_threads().
 */
int AsyncTaskChain::
get_num_threads() const {
  MutexHolder holder(_manager->_lock);
  return _num_threads;
}

/**
 * Returns the number of threads that have been created and are actively
 * running.  This will return 0 before the threads have been started; it will
 * also return 0 if thread support is not available.
 */
int AsyncTaskChain::
get_num_running_threads() const {
  MutexHolder holder(_manager->_lock);
  return _threads.size();
}

/**
 * Changes the priority associated with threads that serve this task chain.
 * This may require stopping the threads if they are already running.
 */
void AsyncTaskChain::
set_thread_priority(ThreadPriority priority) {
  MutexHolder holder(_manager->_lock);
  if (_thread_priority != priority) {
    do_stop_threads();
    _thread_priority = priority;

    if (_num_tasks != 0) {
      do_start_threads();
    }
  }
}

/**
 * Returns the priority associated with threads that serve this task chain.
 */
ThreadPriority AsyncTaskChain::
get_thread_priority() const {
  MutexHolder holder(_manager->_lock);
  return _thread_priority;
}

/**
 * Sets the maximum amount of time per frame the tasks on this chain are
 * granted for execution.  If this is less than zero, there is no limit; if it
 * is >= 0, it represents a maximum amount of time (in seconds) that will be
 * used to execute tasks.  If this time is exceeded in any one frame, the task
 * chain will stop executing tasks until the next frame, as defined by the
 * TaskManager's clock.
 */
void AsyncTaskChain::
set_frame_budget(double frame_budget) {
  MutexHolder holder(_manager->_lock);
  _frame_budget = frame_budget;
}

/**
 * Returns the maximum amount of time per frame the tasks on this chain are
 * granted for execution.  See set_frame_budget().
 */
double AsyncTaskChain::
get_frame_budget() const {
  MutexHolder holder(_manager->_lock);
  return _frame_budget;
}

/**
 * Sets the frame_sync flag.  When this flag is true, this task chain will be
 * forced to sync with the TaskManager's clock.  It will run no faster than
 * one epoch per clock frame.
 *
 * When this flag is false, the default, the task chain will finish all of its
 * tasks and then immediately start from the first task again, regardless of
 * the clock frame.  When it is true, the task chain will finish all of its
 * tasks and then wait for the clock to tick to the next frame before resuming
 * the first task.
 *
 * This only makes sense for threaded task chains.  Non-threaded task chains
 * are automatically synchronous.
 */
void AsyncTaskChain::
set_frame_sync(bool frame_sync) {
  MutexHolder holder(_manager->_lock);
  _frame_sync = frame_sync;
}

/**
 * Returns the frame_sync flag.  See set_frame_sync().
 */
bool AsyncTaskChain::
get_frame_sync() const {
  MutexHolder holder(_manager->_lock);
  return _frame_sync;
}

/**
 * Sets the timeslice_priority flag.  This changes the interpretation of
 * priority, and the number of times per epoch each task will run.
 *
 * When this flag is true, some tasks might not run in any given epoch.
 * Instead, tasks with priority higher than 1 will be given precedence, in
 * proportion to the amount of time they have already used.  This gives
 * higher-priority tasks more runtime than lower-priority tasks.  Each task
 * gets the amount of time proportional to its priority value, so a task with
 * priority 100 will get five times as much processing time as a task with
 * priority 20.  For these purposes, priority values less than 1 are deemed to
 * be equal to 1.
 *
 * When this flag is false (the default), all tasks are run exactly once each
 * epoch, round-robin style.  Priority is only used to determine which task
 * runs first within tasks of the same sort value.
 */
void AsyncTaskChain::
set_timeslice_priority(bool timeslice_priority) {
  MutexHolder holder(_manager->_lock);
  _timeslice_priority = timeslice_priority;
}

/**
 * Returns the timeslice_priority flag.  This changes the interpretation of
 * priority, and the number of times per epoch each task will run.  See
 * set_timeslice_priority().
 */
bool AsyncTaskChain::
get_timeslice_priority() const {
  MutexHolder holder(_manager->_lock);
  return _timeslice_priority;
}

/**
 * Stops any threads that are currently running.  If any tasks are still
 * pending and have not yet been picked up by a thread, they will not be
 * serviced unless poll() or start_threads() is later called.
 */
void AsyncTaskChain::
stop_threads() {
  if (_state == S_started || _state == S_interrupted) {
    // Clean up all of the threads.
    MutexHolder holder(_manager->_lock);
    do_stop_threads();
  }
}

/**
 * Starts any requested threads to service the tasks on the queue.  This is
 * normally not necessary, since adding a task will start the threads
 * automatically.
 */
void AsyncTaskChain::
start_threads() {
  if (_state == S_initial || _state == S_interrupted) {
    MutexHolder holder(_manager->_lock);
    do_start_threads();
  }
}

/**
 * Returns true if the indicated task has been added to this AsyncTaskChain,
 * false otherwise.
 */
bool AsyncTaskChain::
has_task(AsyncTask *task) const {
  MutexHolder holder(_manager->_lock);

  if (task->_chain != this) {
    nassertr(!do_has_task(task), false);
    return false;
  }

  if (task->_state == AsyncTask::S_servicing_removed) {
    return false;
  }

  // The task might not actually be in the active queue, since it might be
  // being serviced right now.  That's OK.
  return true;
}

/**
 * Blocks until the task list is empty.
 */
void AsyncTaskChain::
wait_for_tasks() {
  MutexHolder holder(_manager->_lock);
  do_wait_for_tasks();
}

/**
 * Returns the number of tasks that are currently active or sleeping within
 * the task chain.
 */
int AsyncTaskChain::
get_num_tasks() const {
  MutexHolder holder(_manager->_lock);
  return _num_tasks;
}

/**
 * Returns the set of tasks that are active or sleeping on the task chain, at
 * the time of the call.
 */
AsyncTaskCollection AsyncTaskChain::
get_tasks() const {
  MutexHolder holder(_manager->_lock);
  AsyncTaskCollection result = do_get_active_tasks();
  result.add_tasks_from(do_get_sleeping_tasks());
  return result;
}

/**
 * Returns the set of tasks that are active (and not sleeping) on the task
 * chain, at the time of the call.
 */
AsyncTaskCollection AsyncTaskChain::
get_active_tasks() const {
  MutexHolder holder(_manager->_lock);
  return do_get_active_tasks();
}

/**
 * Returns the set of tasks that are sleeping (and not active) on the task
 * chain, at the time of the call.
 */
AsyncTaskCollection AsyncTaskChain::
get_sleeping_tasks() const {
  MutexHolder holder(_manager->_lock);
  return do_get_sleeping_tasks();
}

/**
 * Runs through all the tasks in the task list, once, if the task chain is
 * running in single-threaded mode (no threads available).  This method does
 * nothing in threaded mode, so it may safely be called in either case.
 *
 * Normally, you would not call this function directly; instead, call
 * AsyncTaskManager::poll(), which polls all of the task chains in sequence.
 */
void AsyncTaskChain::
poll() {
  MutexHolder holder(_manager->_lock);
  do_poll();
}

/**
 * Returns the scheduled time (on the manager's clock) of the next sleeping
 * task, on any task chain, to awaken.  Returns -1 if there are no sleeping
 * tasks.
 */
double AsyncTaskChain::
get_next_wake_time() const {
  MutexHolder holder(_manager->_lock);
  return do_get_next_wake_time();
}

/**
 *
 */
void AsyncTaskChain::
output(ostream &out) const {
  MutexHolder holder(_manager->_lock);
  do_output(out);
}

/**
 *
 */
void AsyncTaskChain::
write(ostream &out, int indent_level) const {
  MutexHolder holder(_manager->_lock);
  do_write(out, indent_level);
}

/**
 * Adds the indicated task to the active queue.  It is an error if the task is
 * already added to this or any other active queue.
 *
 * This is normally called only by the AsyncTaskManager.  Assumes the lock is
 * already held.
 */
void AsyncTaskChain::
do_add(AsyncTask *task) {
  nassertv(task->_chain == nullptr &&
           task->_manager == nullptr &&
           task->_chain_name == get_name() &&
           task->_state == AsyncTask::S_inactive);
  nassertv(!do_has_task(task));

  do_start_threads();

  task->_chain = this;
  task->_manager = _manager;

  double now = _manager->_clock->get_frame_time();
  task->_start_time = now;
  task->_start_frame = _manager->_clock->get_frame_count();

  // Remember the order in which tasks were added to the chain.
  task->_implicit_sort = _next_implicit_sort++;

  _manager->add_task_by_name(task);

  if (task->has_delay()) {
    // This is a deferred task.  Add it to the sleeping queue.
    task->_wake_time = now + task->get_delay();
    task->_start_time = task->_wake_time;
    task->_state = AsyncTask::S_sleeping;
    _sleeping.push_back(task);
    push_heap(_sleeping.begin(), _sleeping.end(), AsyncTaskSortWakeTime());

  } else {
    // This is an active task.  Add it to the active set.
    task->_state = AsyncTask::S_active;
    if (task_cat.is_spam()) {
      task_cat.spam()
        << "Adding " << *task << " with sort " << task->get_sort()
        << " to chain " << get_name() << " with current_sort "
        << _current_sort << "\n";
    }
    if (task->get_sort() >= _current_sort) {
      // It will run this frame.
      _active.push_back(task);
      push_heap(_active.begin(), _active.end(), AsyncTaskSortPriority());
    } else {
      // It will run next frame.
      _next_active.push_back(task);
    }
  }
  ++_num_tasks;
  ++(_manager->_num_tasks);
  _needs_cleanup = true;

  _cvar.notify_all();
}

/**
 * Removes the indicated task from this chain.  Returns true if removed, false
 * otherwise.  Assumes the lock is already held.  The task->upon_death()
 * method is called with clean_exit=false if upon_death is given.
 */
bool AsyncTaskChain::
do_remove(AsyncTask *task, bool upon_death) {
  nassertr(task->_chain == this, false);

  switch (task->_state) {
  case AsyncTask::S_servicing:
    // This task is being serviced.  upon_death will be called afterwards.
    task->_state = AsyncTask::S_servicing_removed;
    return true;

  case AsyncTask::S_servicing_removed:
    // Being serviced, though it is already marked to be removed afterwards.
    return false;

  case AsyncTask::S_sleeping:
    // Sleeping, easy.
    {
      int index = find_task_on_heap(_sleeping, task);
      nassertr(index != -1, false);
      PT(AsyncTask) hold_task = task;
      _sleeping.erase(_sleeping.begin() + index);
      make_heap(_sleeping.begin(), _sleeping.end(), AsyncTaskSortWakeTime());
      cleanup_task(task, upon_death, false);
    }
    return true;

  case AsyncTask::S_active:
    {
      // Active, but not being serviced, easy.
      PT(AsyncTask) hold_task = task;
      int index = find_task_on_heap(_active, task);
      if (index != -1) {
        _active.erase(_active.begin() + index);
        make_heap(_active.begin(), _active.end(), AsyncTaskSortPriority());
      } else {
        index = find_task_on_heap(_next_active, task);
        if (index != -1) {
          _next_active.erase(_next_active.begin() + index);
        } else {
          index = find_task_on_heap(_this_active, task);
          nassertr(index != -1, false);
        }
      }
      cleanup_task(task, upon_death, false);
      return true;
    }

  default:
    break;
  }

  return false;
}

/**
 * Blocks until the task list is empty.  Assumes the lock is held.
 */
void AsyncTaskChain::
do_wait_for_tasks() {
  do_start_threads();

  if (_threads.empty()) {
    // Non-threaded case.
    while (_num_tasks > 0) {
      if (_state == S_shutdown || _state == S_interrupted) {
        return;
      }
      do_poll();
    }

  } else {
    // Threaded case.
    while (_num_tasks > 0) {
      if (_state == S_shutdown || _state == S_interrupted) {
        return;
      }

      PStatTimer timer(_wait_pcollector);
      _cvar.wait();
    }
  }
}

/**
 * Stops all threads and messily empties the task list.  This is intended to
 * be called on destruction only.  Assumes the lock is already held.
 */
void AsyncTaskChain::
do_cleanup() {
  if (task_cat.is_spam()) {
    do_output(task_cat.spam());
    task_cat.spam(false)
      << ": do_cleanup()\n";
  }
  do_stop_threads();
  _num_threads = 0;

  // Don't call the upon_death functions while we clean up the tasks.
  // Instead, store all the tasks in a list as we clean them up, and then call
  // the upon_death functions all at once.  We do this because calling
  // upon_death wil release the lock, allowing the iterators to become
  // invalid.

  TaskHeap dead;
  dead.reserve(_num_tasks);

  _needs_cleanup = false;

  TaskHeap::const_iterator ti;
  for (ti = _active.begin(); ti != _active.end(); ++ti) {
    AsyncTask *task = (*ti);
    dead.push_back(task);
    cleanup_task(task, false, false);
  }
  for (ti = _this_active.begin(); ti != _this_active.end(); ++ti) {
    AsyncTask *task = (*ti);
    dead.push_back(task);
    cleanup_task(task, false, false);
  }
  for (ti = _next_active.begin(); ti != _next_active.end(); ++ti) {
    AsyncTask *task = (*ti);
    dead.push_back(task);
    cleanup_task(task, false, false);
  }
  for (ti = _sleeping.begin(); ti != _sleeping.end(); ++ti) {
    AsyncTask *task = (*ti);
    dead.push_back(task);
    cleanup_task(task, false, false);
  }

  // There might still be one task remaining: the currently-executing task.
  nassertv(_num_tasks == 0 || _num_tasks == 1);

  // Now go back and call the upon_death functions.
  _manager->_lock.unlock();
  for (ti = dead.begin(); ti != dead.end(); ++ti) {
    (*ti)->upon_death(_manager, false);
  }
  _manager->_lock.lock();

  if (task_cat.is_spam()) {
    do_output(task_cat.spam());
    task_cat.spam(false)
      << ": done do_cleanup()\n";
  }
}

/**
 * Returns true if the task is on one of the task lists, false if it is not
 * (false may mean that the task is currently being serviced).  Assumes the
 * lock is currently held.
 */
bool AsyncTaskChain::
do_has_task(AsyncTask *task) const {
  return (find_task_on_heap(_active, task) != -1 ||
          find_task_on_heap(_next_active, task) != -1 ||
          find_task_on_heap(_sleeping, task) != -1 ||
          find_task_on_heap(_this_active, task) != -1);
}

/**
 * Returns the index number of the indicated task within the specified task
 * list, or -1 if the task is not found in the list (this may mean that it is
 * currently being serviced).  Assumes that the lock is currently held.
 */
int AsyncTaskChain::
find_task_on_heap(const TaskHeap &heap, AsyncTask *task) const {
  for (int i = 0; i < (int)heap.size(); ++i) {
    if (heap[i] == task) {
      return i;
    }
  }

  return -1;
}

/**
 * Pops a single task off the active queue, services it, and restores it to
 * the end of the queue.  This is called internally only within one of the
 * task threads.  Assumes the lock is already held.
 *
 * Note that the lock may be temporarily released by this method.
 */
void AsyncTaskChain::
service_one_task(AsyncTaskChain::AsyncTaskChainThread *thread) {
  if (!_active.empty()) {
    PT(AsyncTask) task = _active.front();
    pop_heap(_active.begin(), _active.end(), AsyncTaskSortPriority());
    _active.pop_back();

    if (thread != nullptr) {
      thread->_servicing = task;
    }

    if (task_cat.is_spam()) {
      task_cat.spam()
        << "Servicing " << *task << " in "
        << *Thread::get_current_thread() << "\n";
    }

    nassertv(task->get_sort() == _current_sort);
    nassertv(task->_state == AsyncTask::S_active);
    task->_state = AsyncTask::S_servicing;
    task->_servicing_thread = thread;

    AsyncTask::DoneStatus ds = task->unlock_and_do_task();

    if (thread != nullptr) {
      thread->_servicing = nullptr;
    }
    task->_servicing_thread = nullptr;

    if (task->_chain == this) {
      if (task->_state == AsyncTask::S_servicing_removed) {
        // This task wants to kill itself.
        cleanup_task(task, true, false);

      } else if (task->_chain_name != get_name()) {
        // The task wants to jump to a different chain.
        PT(AsyncTask) hold_task = task;
        cleanup_task(task, false, false);
        task->jump_to_task_chain(_manager);

      } else {
        switch (ds) {
        case AsyncTask::DS_cont:
          // The task is still alive; put it on the next frame's active queue.
          task->_state = AsyncTask::S_active;
          _next_active.push_back(task);
          _cvar.notify_all();
          break;

        case AsyncTask::DS_again:
          // The task wants to sleep again.
          {
            double now = _manager->_clock->get_frame_time();
            task->_wake_time = now + task->get_delay();
            task->_start_time = task->_wake_time;
            task->_state = AsyncTask::S_sleeping;
            _sleeping.push_back(task);
            push_heap(_sleeping.begin(), _sleeping.end(), AsyncTaskSortWakeTime());
            if (task_cat.is_spam()) {
              task_cat.spam()
                << "Sleeping " << *task << ", wake time at "
                << task->_wake_time - now << "\n";
            }
            _cvar.notify_all();
          }
          break;

        case AsyncTask::DS_pickup:
          // The task wants to run again this frame if possible.
          task->_state = AsyncTask::S_active;
          _this_active.push_back(task);
          _cvar.notify_all();
          break;

        case AsyncTask::DS_interrupt:
          // The task had an exception and wants to raise a big flag.
          task->_state = AsyncTask::S_active;
          _next_active.push_back(task);
          if (_state == S_started) {
            _state = S_interrupted;
            _cvar.notify_all();
          }
          break;

        case AsyncTask::DS_await:
          // The task wants to wait for another one to finish.
          task->_state = AsyncTask::S_awaiting;
          _cvar.notify_all();
          ++_num_awaiting_tasks;
          break;

        default:
          // The task has finished.
          cleanup_task(task, true, true);
        }
      }
    } else {
      task_cat.error()
        << "Task is no longer on chain " << get_name()
        << ": " << *task << "\n";
    }

    if (task_cat.is_spam()) {
      task_cat.spam()
        << "Done servicing " << *task << " in "
        << *Thread::get_current_thread() << "\n";
    }
  }
  thread_consider_yield();
}

/**
 * Called internally when a task has completed (or been interrupted) and is
 * about to be removed from the active queue.  Assumes the lock is held.
 *
 * If upon_death is true, then task->upon_death() will also be called, with
 * the indicated clean_exit parameter.
 *
 * Note that the lock may be temporarily released by this method.
 */
void AsyncTaskChain::
cleanup_task(AsyncTask *task, bool upon_death, bool clean_exit) {
  if (task_cat.is_spam()) {
    do_output(task_cat.spam());
    task_cat.spam(false)
      << ": cleanup_task(" << *task << ", " << upon_death << ", " << clean_exit
      << ")\n";
  }

  nassertv(task->_chain == this);

  task->_state = AsyncTask::S_inactive;
  task->_chain = nullptr;
  --_num_tasks;
  --(_manager->_num_tasks);

  _manager->remove_task_by_name(task);

  if (upon_death) {
    _manager->_lock.unlock();
    if (task->set_future_state(clean_exit ? AsyncFuture::FS_finished
                                          : AsyncFuture::FS_cancelled)) {
      task->notify_done(clean_exit);
    }
    task->upon_death(_manager, clean_exit);
    _manager->_lock.lock();
  }

  task->_manager = nullptr;
}

/**
 * Called internally when all tasks of a given sort value have been completed,
 * and it is time to increment to the next sort value, or begin the next
 * epoch.  Assumes the lock is held.
 *
 * Returns true if there are more tasks on the queue after this operation, or
 * false if the task list is empty and we need to wait.
 */
bool AsyncTaskChain::
finish_sort_group() {
  nassertr(_num_busy_threads == 0, true);

  if (!_threads.empty()) {
    PStatClient::thread_tick(get_name());
  }

  if (!_active.empty()) {
    // There are more tasks; just set the next sort value.
    nassertr(_current_sort < _active.front()->get_sort(), true);
    _current_sort = _active.front()->get_sort();
    _cvar.notify_all();
    return true;
  }

  // There are no more tasks in this epoch; advance to the next epoch.

  if (!_this_active.empty() && _frame_budget >= 0.0) {
    // Enter pickup mode.  This is a special mode at the end of the epoch in
    // which we are just re-running the tasks that think they can still run
    // within the frame, in an attempt to use up our frame budget.

    if (task_cat.is_spam()) {
      do_output(task_cat.spam());
      task_cat.spam(false)
        << ": next epoch (pickup mode)\n";
    }

    _pickup_mode = true;
    _active.swap(_this_active);

  } else {
    // Not in pickup mode.

    if (task_cat.is_spam()) {
      do_output(task_cat.spam());
      task_cat.spam(false)
        << ": next epoch\n";
    }

    _pickup_mode = false;

    // Here, there's no difference between _this_active and _next_active.
    // Combine them.
    _next_active.insert(_next_active.end(), _this_active.begin(), _this_active.end());
    _this_active.clear();

    _active.swap(_next_active);

    // We only tick the clock and wake sleepers in normal mode, the first time
    // through the task list; not in pickup mode when we are re-running the
    // stragglers just to use up our frame budget.

    if (_tick_clock) {
      if (task_cat.is_spam()) {
        do_output(task_cat.spam());
        task_cat.spam(false)
          << ": tick clock\n";
      }
      _manager->_clock->tick();
      _manager->_frame_cvar.notify_all();

    } else if (_frame_sync) {
      // If we're a synced chain, we have to wait at the end of the epoch for
      // someone else to tick the clock.
      _block_till_next_frame = true;
    }

    // Check for any sleeping tasks that need to be woken.
    double now = _manager->_clock->get_frame_time();
    while (!_sleeping.empty() && _sleeping.front()->_wake_time <= now) {
      PT(AsyncTask) task = _sleeping.front();
      if (task_cat.is_spam()) {
        task_cat.spam()
          << "Waking " << *task << ", wake time at "
          << task->_wake_time - now << "\n";
      }
      pop_heap(_sleeping.begin(), _sleeping.end(), AsyncTaskSortWakeTime());
      _sleeping.pop_back();
      task->_state = AsyncTask::S_active;
      task->_start_frame = _manager->_clock->get_frame_count();
      _active.push_back(task);
    }

    if (task_cat.is_spam()) {
      if (_sleeping.empty()) {
        task_cat.spam()
          << "No more tasks on sleeping queue.\n";
      } else {
        task_cat.spam()
          << "Next sleeper: " << *_sleeping.front() << ", wake time at "
          << _sleeping.front()->_wake_time - now << "\n";
      }
    }

    // Any tasks that are on the active queue at the beginning of the epoch
    // are deemed to have run one frame (or to be about to).
    TaskHeap::const_iterator ti;
    for (ti = _active.begin(); ti != _active.end(); ++ti) {
      AsyncTask *task = (*ti);
      ++task->_num_frames;
    }
  }

  if (_timeslice_priority) {
    filter_timeslice_priority();
  }

  nassertr((size_t)_num_tasks == _active.size() + _this_active.size() + _next_active.size() + _sleeping.size() + (size_t)_num_awaiting_tasks, true);
  make_heap(_active.begin(), _active.end(), AsyncTaskSortPriority());

  _current_sort = -INT_MAX;

  if (!_active.empty()) {
    // Signal the threads to start executing the first task again.
    _cvar.notify_all();
    return true;
  }

  // There are no tasks to be had anywhere.  Chill.
  _pickup_mode = false;
  nassertr(_this_active.empty(), false);
  return false;
}

/**
 * Called to filter the _active tasks list when we are in the special
 * timeslice_priority mode.  In this mode, go through and postpone any tasks
 * that have already exceeded their priority budget for this epoch.
 *
 * Assumes the lock is already held.
 */
void AsyncTaskChain::
filter_timeslice_priority() {
  if (_active.empty()) {
    return;
  }
  nassertv(_timeslice_priority);

  // We must first sum up the average per-epoch runtime of each task.
  double net_runtime = 0.0;
  int net_priority = 0;

  TaskHeap::iterator ti;
  for (ti = _active.begin(); ti != _active.end(); ++ti) {
    AsyncTask *task = (*ti);
    double runtime = max(task->get_average_dt(), 0.0);
    int priority = max(task->_priority, 1);
    net_runtime += runtime;
    net_priority += priority;
  }

  // That gives us a timeslice budget per priority value.
  double average_budget = net_runtime / (double)net_priority;

  TaskHeap keep, postpone;
  for (ti = _active.begin(); ti != _active.end(); ++ti) {
    AsyncTask *task = (*ti);
    double runtime = max(task->get_average_dt(), 0.0);
    int priority = max(task->_priority, 1);
    double consumed = runtime / (double)priority;
    // cerr << *task << " consumed " << consumed << " vs.  " << average_budget
    // << "\n";
    if (consumed > average_budget) {
      // Postpone.  Run this task next epoch.
      postpone.push_back(task);
    } else {
      // Keep, and run this task this epoch.
      keep.push_back(task);
    }
  }

  if (keep.empty()) {
    // Hmm, nothing to keep.  Grab the postponed task with the highest
    // priority and keep that instead.
    nassertv(!postpone.empty());
    ti = postpone.begin();
    TaskHeap::iterator max_ti = ti;
    ++ti;
    while (ti != postpone.end()) {
      if ((*ti)->_priority > (*max_ti)->_priority) {
        max_ti = ti;
      }
    }

    // cerr << "Nothing to keep, keeping " << *(*max_ti) << " instead\n";

    keep.push_back(*max_ti);
    postpone.erase(max_ti);
  }

  _active.swap(keep);
  if (_pickup_mode) {
    _this_active.insert(_this_active.end(), postpone.begin(), postpone.end());
  } else {
    _next_active.insert(_next_active.end(), postpone.begin(), postpone.end());
  }

  nassertv(!_active.empty());
}

/**
 * The private implementation of stop_threads; assumes the lock is already
 * held.
 */
void AsyncTaskChain::
do_stop_threads() {
  if (_state == S_started || _state == S_interrupted) {
    if (task_cat.is_debug() && !_threads.empty()) {
      task_cat.debug()
        << "Stopping " << _threads.size()
        << " threads for " << _manager->get_name()
        << " chain " << get_name()
        << " in " << *Thread::get_current_thread() << "\n";
    }

    _state = S_shutdown;
    _cvar.notify_all();
    _manager->_frame_cvar.notify_all();

    Threads wait_threads;
    wait_threads.swap(_threads);

    // We have to release the lock while we join, so the threads can wake up
    // and see that we're shutting down.
    _manager->_lock.unlock();
    Threads::iterator ti;
    for (ti = wait_threads.begin(); ti != wait_threads.end(); ++ti) {
      if (task_cat.is_debug()) {
        task_cat.debug()
          << "Waiting for " << *(*ti) << " in "
          << *Thread::get_current_thread() << "\n";
      }
      (*ti)->join();
      if (task_cat.is_spam()) {
        task_cat.spam()
          << "Done waiting for " << *(*ti) << " in "
          << *Thread::get_current_thread() << "\n";
      }
    }
    _manager->_lock.lock();

    _state = S_initial;

    // There might be one busy "thread" still: the main thread.
    nassertv(_num_busy_threads == 0 || _num_busy_threads == 1);
    cleanup_pickup_mode();
  }
}

/**
 * The private implementation of start_threads; assumes the lock is already
 * held.
 */
void AsyncTaskChain::
do_start_threads() {
  if (_state == S_interrupted) {
    do_stop_threads();
  }

  if (_state == S_initial) {
    _state = S_started;
    if (Thread::is_threading_supported() && _num_threads > 0) {
      if (task_cat.is_debug()) {
        task_cat.debug()
          << "Starting " << _num_threads << " threads for "
          << _manager->get_name() << " chain " << get_name() << "\n";
      }
      _needs_cleanup = true;
      _threads.reserve(_num_threads);
      for (int i = 0; i < _num_threads; ++i) {
        ostringstream strm;
        strm << _manager->get_name() << "_" << get_name() << "_" << i;
        PT(AsyncTaskChainThread) thread = new AsyncTaskChainThread(strm.str(), this);
        if (thread->start(_thread_priority, true)) {
          _threads.push_back(thread);
        }
      }
    }
  }
}

/**
 * Returns the set of tasks that are active (and not sleeping) on the task
 * chain, at the time of the call.  Assumes the lock is held.
 */
AsyncTaskCollection AsyncTaskChain::
do_get_active_tasks() const {
  AsyncTaskCollection result;

  Threads::const_iterator thi;
  for (thi = _threads.begin(); thi != _threads.end(); ++thi) {
    AsyncTask *task = (*thi)->_servicing;
    if (task != nullptr) {
      result.add_task(task);
    }
  }
  TaskHeap::const_iterator ti;
  for (ti = _active.begin(); ti != _active.end(); ++ti) {
    AsyncTask *task = (*ti);
    result.add_task(task);
  }
  for (ti = _this_active.begin(); ti != _this_active.end(); ++ti) {
    AsyncTask *task = (*ti);
    result.add_task(task);
  }
  for (ti = _next_active.begin(); ti != _next_active.end(); ++ti) {
    AsyncTask *task = (*ti);
    result.add_task(task);
  }

  return result;
}

/**
 * Returns the set of tasks that are sleeping (and not active) on the task
 * chain, at the time of the call.  Assumes the lock is held.
 */
AsyncTaskCollection AsyncTaskChain::
do_get_sleeping_tasks() const {
  AsyncTaskCollection result;

  TaskHeap::const_iterator ti;
  for (ti = _sleeping.begin(); ti != _sleeping.end(); ++ti) {
    AsyncTask *task = (*ti);
    result.add_task(task);
  }

  return result;
}

/**
 * The private implementation of poll(), this assumes the lock is already
 * held.
 */
void AsyncTaskChain::
do_poll() {
  thread_consider_yield();
  if (_num_tasks == 0) {
    return;
  }

  do_start_threads();

  if (!_threads.empty()) {
    return;
  }

  if (_num_busy_threads != 0) {
    // We are recursively nested within another task.  Return, with a warning.
    task_cat.warning()
      << "Ignoring recursive poll() within another task.\n";
    return;
  }

  nassertv(!_pickup_mode);

  do {
    while (!_active.empty()) {
      if (_state == S_shutdown || _state == S_interrupted) {
        return;
      }
      int frame = _manager->_clock->get_frame_count();
      if (_current_frame != frame) {
        _current_frame = frame;
        _time_in_frame = 0.0;
        _block_till_next_frame = false;
      }
      if (_block_till_next_frame ||
          (_frame_budget >= 0.0 && _time_in_frame >= _frame_budget)) {
        // If we've exceeded our budget, stop here.  We'll resume from this
        // point at the next call to poll().
        cleanup_pickup_mode();
        return;
      }

      _current_sort = _active.front()->get_sort();

      // Normally, there won't be any threads running at the same time we're
      // in poll().  But it's possible, if someone calls set_num_threads()
      // while we're processing.
      _num_busy_threads++;
      service_one_task(nullptr);
      _num_busy_threads--;
      _cvar.notify_all();

      if (!_threads.empty()) {
        return;
      }
    }

    finish_sort_group();
  } while (_pickup_mode);
}

/**
 * Clean up the damage from setting pickup mode.  This means we restore the
 * _active and _next_active lists as they should have been without pickup
 * mode, for next frame.  Assumes the lock is held.
 */
void AsyncTaskChain::
cleanup_pickup_mode() {
  if (_pickup_mode) {
    _pickup_mode = false;

    // Move everything to the _next_active queue.
    _next_active.insert(_next_active.end(), _this_active.begin(), _this_active.end());
    _this_active.clear();
    _next_active.insert(_next_active.end(), _active.begin(), _active.end());
    _active.clear();

    // Now finish the epoch properly.
    finish_sort_group();
  }
}

/**
 * The private implementation of output(), this assumes the lock is already
 * held.
 */
void AsyncTaskChain::
do_output(ostream &out) const {
  if (_manager != nullptr) {
    out << _manager->get_type() << " " << _manager->get_name();
  } else {
    out << "(no manager)";
  }
  out << " task chain " << get_name()
      << "; " << _num_tasks << " tasks";
}

/**
 * The private implementation of write(), this assumes the lock is already
 * held.
 */
void AsyncTaskChain::
do_write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "Task chain \"" << get_name() << "\"\n";
  if (_num_threads > 0) {
    indent(out, indent_level + 2)
      << _num_threads << " threads, priority " << _thread_priority << "\n";
  }
  if (_frame_budget >= 0.0) {
    indent(out, indent_level + 2)
      << "frame budget " << _frame_budget << " s\n";
  }
  if (_timeslice_priority) {
    indent(out, indent_level + 2)
      << "timeslice priority\n";
  }
  if (_tick_clock) {
    indent(out, indent_level + 2)
      << "tick clock\n";
  }

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];
  sprintf(buffer, " %-32s %8s %8s %8s %8s %6s",
          "Task",
          "sleep(s)",
          "dt(ms)", "avg", "max",
          "sort");
  nassertv(strlen(buffer) < buffer_size);

  indent(out, indent_level)
    << buffer << "\n";

  indent(out, indent_level);
  for (int i = 0; i < 32+8+8+8+8+6+7; ++i) {
    out << '-';
  }
  out << "\n";

  // Collect a list of all active tasks, then sort them into order for output.
  TaskHeap tasks = _active;
  tasks.insert(tasks.end(), _this_active.begin(), _this_active.end());
  tasks.insert(tasks.end(), _next_active.begin(), _next_active.end());

  Threads::const_iterator thi;
  for (thi = _threads.begin(); thi != _threads.end(); ++thi) {
    AsyncTask *task = (*thi)->_servicing;
    if (task != nullptr) {
      tasks.push_back(task);
    }
  }

  double now = _manager->_clock->get_frame_time();

  if (!tasks.empty()) {
    sort(tasks.begin(), tasks.end(), AsyncTaskSortPriority());

    // Since AsyncTaskSortPriority() sorts backwards (because of STL's
    // push_heap semantics), we go through the task list in reverse order to
    // print them forwards.
    TaskHeap::reverse_iterator ti;
    for (ti = tasks.rbegin(); ti != tasks.rend(); ++ti) {
      AsyncTask *task = (*ti);
      write_task_line(out, indent_level, task, now);
    }
  }

  // Instead of iterating through the _sleeping list in heap order, copy it
  // and then use repeated pops to get it out in sorted order, for the user's
  // satisfaction.
  TaskHeap sleeping = _sleeping;
  while (!sleeping.empty()) {
    PT(AsyncTask) task = sleeping.front();
    pop_heap(sleeping.begin(), sleeping.end(), AsyncTaskSortWakeTime());
    sleeping.pop_back();

    write_task_line(out, indent_level, task, now);
  }
}

/**
 * Writes a single line for a task in the report generated by do_write().
 * Assumes the lock is already held.
 */
void AsyncTaskChain::
write_task_line(ostream &out, int indent_level, AsyncTask *task, double now) const {
  char servicing_flag = ' ';
  if (task->_state == AsyncTask::S_servicing) {
    servicing_flag = '*';
  } else if (task->_state == AsyncTask::S_servicing_removed) {
    servicing_flag = '-';
  }

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

  if (task->_state == AsyncTask::S_sleeping) {
    // For sleeping tasks, include the wake time, as an elapsed time in
    // seconds.
    string name = task->get_name().substr(0, 32);
    sprintf(buffer, "%c%-32s %8.1f",
            servicing_flag, name.c_str(),
            task->_wake_time - now);
  } else {
    // For active tasks, don't include a wake time.  This means we have more
    // space for the name.
    string name = task->get_name().substr(0, 41);
    sprintf(buffer, "%c%-41s",
            servicing_flag, name.c_str());
  }
  nassertv(strlen(buffer) < buffer_size);

  indent(out, indent_level)
    << buffer;

  if (task->_num_frames > 0) {
    sprintf(buffer, " %8.1f %8.1f %8.1f %6d",
            task->_dt * 1000.0, task->get_average_dt() * 1000.0,
            task->_max_dt * 1000.0,
            task->_sort);
  } else {
    // No statistics for a task that hasn't run yet.
    sprintf(buffer, " %8s %8s %8s %6d",
            "", "", "",
            task->_sort);
  }

  nassertv(strlen(buffer) < buffer_size);
  out << buffer << "\n";
}

/**
 *
 */
AsyncTaskChain::AsyncTaskChainThread::
AsyncTaskChainThread(const string &name, AsyncTaskChain *chain) :
  Thread(name, chain->get_name()),
  _chain(chain),
  _servicing(nullptr)
{
}

/**
 *
 */
void AsyncTaskChain::AsyncTaskChainThread::
thread_main() {
  MutexHolder holder(_chain->_manager->_lock);
  while (_chain->_state != S_shutdown && _chain->_state != S_interrupted) {
    thread_consider_yield();
    if (!_chain->_active.empty() &&
        _chain->_active.front()->get_sort() == _chain->_current_sort) {

      int frame = _chain->_manager->_clock->get_frame_count();
      if (_chain->_current_frame != frame) {
        _chain->_current_frame = frame;
        _chain->_time_in_frame = 0.0;
        _chain->_block_till_next_frame = false;
      }

      // If we've exceeded our frame budget, sleep until the next frame.
      if (_chain->_block_till_next_frame ||
          (_chain->_frame_budget >= 0.0 && _chain->_time_in_frame >= _chain->_frame_budget)) {
        while ((_chain->_block_till_next_frame ||
                (_chain->_frame_budget >= 0.0 && _chain->_time_in_frame >= _chain->_frame_budget)) &&
               _chain->_state != S_shutdown && _chain->_state != S_interrupted) {
          _chain->cleanup_pickup_mode();
          _chain->_manager->_frame_cvar.wait();
          frame = _chain->_manager->_clock->get_frame_count();
          if (_chain->_current_frame != frame) {
            _chain->_current_frame = frame;
            _chain->_time_in_frame = 0.0;
            _chain->_block_till_next_frame = false;
          }
        }
        // Now that it's the next frame, go back to the top of the loop.
        continue;
      }

      PStatTimer timer(_task_pcollector);
      _chain->_num_busy_threads++;
      _chain->service_one_task(this);
      _chain->_num_busy_threads--;
      _chain->_cvar.notify_all();

    } else {
      // We've finished all the available tasks of the current sort value.  We
      // can't pick up a new task until all of the threads finish the tasks
      // with the same sort value.
      if (_chain->_num_busy_threads == 0) {
        // We're the last thread to finish.  Update _current_sort.
        if (!_chain->finish_sort_group()) {
          // Nothing to do.  Wait for more tasks to be added.
          if (_chain->_sleeping.empty()) {
            PStatTimer timer(_wait_pcollector);
            _chain->_cvar.wait();
          } else {
            double wake_time = _chain->do_get_next_wake_time();
            double now = _chain->_manager->_clock->get_frame_time();
            double timeout = max(wake_time - now, 0.0);
            PStatTimer timer(_wait_pcollector);
            _chain->_cvar.wait(timeout);
          }
        }

      } else {
        // Wait for the other threads to finish their current task before we
        // continue.
        PStatTimer timer(_wait_pcollector);
        _chain->_cvar.wait();
      }
    }
  }
}
