/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTask.cxx
 * @author drose
 * @date 2006-08-23
 */

#include "asyncTask.h"
#include "asyncTaskManager.h"
#include "config_event.h"
#include "pt_Event.h"
#include "throw_event.h"
#include "eventParameter.h"

using std::string;

AtomicAdjust::Integer AsyncTask::_next_task_id;
PStatCollector AsyncTask::_show_code_pcollector("App:Show code");
TypeHandle AsyncTask::_type_handle;

/**
 *
 */
AsyncTask::
AsyncTask(const string &name) :
  _chain_name("default"),
  _delay(0.0),
  _has_delay(false),
  _wake_time(0.0),
  _sort(0),
  _priority(0),
  _state(S_inactive),
  _servicing_thread(nullptr),
  _chain(nullptr),
  _start_time(0.0),
  _start_frame(0),
  _dt(0.0),
  _max_dt(0.0),
  _total_dt(0.0),
  _num_frames(0)
{
  set_name(name);

  // Carefully copy _next_task_id and increment it so that we get a unique ID.
  AtomicAdjust::Integer current_id = _next_task_id;
  while (AtomicAdjust::compare_and_exchange(_next_task_id, current_id, current_id + 1) != current_id) {
    current_id = _next_task_id;
  }

  _task_id = current_id;
}

/**
 *
 */
AsyncTask::
~AsyncTask() {
  nassertv(_state == S_inactive && _manager == nullptr && _chain == nullptr);
}

/**
 * Removes the task from its active manager, if any, and makes the state
 * S_inactive (or possible S_servicing_removed).  This is a no-op if the state
 * is already S_inactive.
 */
bool AsyncTask::
remove() {
  AsyncTaskManager *manager = _manager;
  if (manager != nullptr) {
    nassertr(_chain->_manager == manager, false);
    if (task_cat.is_debug()) {
      task_cat.debug()
        << "Removing " << *this << "\n";
    }
    MutexHolder holder(manager->_lock);
    if (_chain->do_remove(this, true)) {
      return true;
    } else {
      if (task_cat.is_debug()) {
        task_cat.debug()
          << "  (unable to remove " << *this << ")\n";
      }
      return false;
    }
  }
  return false;
}

/**
 * If this task has been added to an AsyncTaskManager with a delay in effect,
 * this returns the time at which the task is expected to awaken.  It has no
 * meaning if the task has not yet been added to a queue, or if there was no
 * delay in effect at the time the task was added.
 *
 * If the task's status is not S_sleeping, this returns 0.0.
 */
double AsyncTask::
get_wake_time() const {
  if (_manager != nullptr) {
    MutexHolder holder(_manager->_lock);
    if (_state == S_sleeping) {
      return _wake_time;
    }
  }

  // If it's not on any manager, or it's not sleeping, the wake time is 0.0.
  return 0.0;
}

/**
 * If the task is currently sleeping on a task chain, this resets its wake
 * time to the current time + get_delay().  It is as if the task had suddenly
 * returned DS_again.  The task will sleep for its current delay seconds
 * before running again.  This method may therefore be used to make the task
 * wake up sooner or later than it would have otherwise.
 *
 * If the task is not already sleeping, this method has no effect.
 */
void AsyncTask::
recalc_wake_time() {
  if (_manager != nullptr) {
    MutexHolder holder(_manager->_lock);
    if (_state == S_sleeping) {
      double now = _manager->_clock->get_frame_time();
      _wake_time = now + _delay;
      _start_time = _wake_time;

      make_heap(_chain->_sleeping.begin(), _chain->_sleeping.end(),
                AsyncTaskChain::AsyncTaskSortWakeTime());
    }
  }
}

/**
 * Returns the amount of time that has elapsed since the task was started,
 * according to the task manager's clock.
 *
 * It is only valid to call this if the task's status is not S_inactive.
 */
double AsyncTask::
get_elapsed_time() const {
  nassertr(_state != S_inactive, 0.0);
  nassertr(_manager != nullptr, 0.0);
  return _manager->_clock->get_frame_time() - _start_time;
}

/**
 * Returns the number of frames that have elapsed since the task was started,
 * according to the task manager's clock.
 *
 * It is only valid to call this if the task's status is not S_inactive.
 */
int AsyncTask::
get_elapsed_frames() const {
  nassertr(_state != S_inactive, 0);
  nassertr(_manager != nullptr, 0);
  return _manager->_clock->get_frame_count() - _start_frame;
}

/**
 *
 */
void AsyncTask::
set_name(const string &name) {
  if (_manager != nullptr) {
    MutexHolder holder(_manager->_lock);
    if (Namable::get_name() != name) {
      // Changing an active task's name requires moving it around on its name
      // index.

      _manager->remove_task_by_name(this);
      Namable::set_name(name);
      _manager->add_task_by_name(this);
    }
  } else {
    // If it hasn't been started anywhere, we can just change the name.
    Namable::set_name(name);
  }

#ifdef DO_PSTATS
  // Update the PStatCollector with the new name.  If the name includes a
  // colon, we stop the collector name there, and don't go further.
  size_t end = name.size();
  size_t colon = name.find(':');
  if (colon != string::npos) {
    end = std::min(end, colon);
  }

  // If the name ends with a hyphen followed by a string of digits, we strip
  // all that off, for the parent collector, to group related tasks together
  // in the pstats graph.  We still create a child collector that contains the
  // full name, however.
  size_t trimmed = end;
  size_t p = trimmed;
  while (true) {
    while (p > 0 && isdigit(name[p - 1])) {
      --p;
    }
    if (p > 0 && (name[p - 1] == '-' || name[p - 1] == '_')) {
      --p;
      trimmed = p;
    } else {
      p = trimmed;
      break;
    }
  }
  PStatCollector parent(_show_code_pcollector, name.substr(0, trimmed));
  // prevent memory leak _task_pcollector = PStatCollector(parent,
  // name.substr(0, end));
  _task_pcollector = parent;
#endif  // DO_PSTATS
}

/**
 * Returns the initial part of the name, up to but not including any trailing
 * digits following a hyphen or underscore.
 */
string AsyncTask::
get_name_prefix() const {
  string name = get_name();
  size_t trimmed = name.size();
  size_t p = trimmed;
  while (true) {
    while (p > 0 && isdigit(name[p - 1])) {
      --p;
    }
    if (p > 0 && (name[p - 1] == '-' || name[p - 1] == '_')) {
      --p;
      trimmed = p;
    } else {
      p = trimmed;
      break;
    }
  }

  return name.substr(0, trimmed);
}

/**
 * Specifies the AsyncTaskChain on which this task will be running.  Each task
 * chain runs tasks independently of the others.
 */
void AsyncTask::
set_task_chain(const string &chain_name) {
  if (chain_name != _chain_name) {
    if (_manager != nullptr) {
      MutexHolder holder(_manager->_lock);
      if (_state == S_active) {
        // Changing chains on an "active" (i.e.  enqueued) task means removing
        // it and re-inserting it into the queue.
        PT(AsyncTask) hold_task = this;
        PT(AsyncTaskManager) manager = _manager;

        AsyncTaskChain *chain_a = manager->do_find_task_chain(_chain_name);
        nassertv(chain_a != nullptr);
        chain_a->do_remove(this);
        _chain_name = chain_name;

        jump_to_task_chain(manager);

      } else {
        // If it's sleeping, currently being serviced, or something else, we
        // can just change the chain_name value directly.
        _chain_name = chain_name;
      }
    } else {
      // If it hasn't been started anywhere, we can just change the chain_name
      // value.
      _chain_name = chain_name;
    }
  }
}

/**
 * Specifies a sort value for this task.  Within a given AsyncTaskManager, all
 * of the tasks with a given sort value are guaranteed to be completed before
 * any tasks with a higher sort value are begun.
 *
 * To put it another way, two tasks might execute in parallel with each other
 * only if they both have the same sort value.  Tasks with a lower sort value
 * are executed first.
 *
 * This is different from the priority, which makes no such exclusion
 * guarantees.
 */
void AsyncTask::
set_sort(int sort) {
  if (sort != _sort) {
    if (_manager != nullptr) {
      MutexHolder holder(_manager->_lock);
      if (_state == S_active && _sort >= _chain->_current_sort) {
        // Changing sort on an "active" (i.e.  enqueued) task means removing
        // it and re-inserting it into the queue.
        PT(AsyncTask) hold_task = this;
        AsyncTaskChain *chain = _manager->do_find_task_chain(_chain_name);
        nassertv(chain != nullptr);
        chain->do_remove(this);
        _sort = sort;
        chain->do_add(this);

      } else {
        // If it's sleeping, currently being serviced, or something else, we
        // can just change the sort value directly.
        _sort = sort;
      }
    } else {
      // If it hasn't been started anywhere, we can just change the sort
      // value.
      _sort = sort;
    }
  }
}

/**
 * Specifies a priority value for this task.  In general, tasks with a higher
 * priority value are executed before tasks with a lower priority value (but
 * only for tasks with the same sort value).
 *
 * Unlike the sort value, tasks with different priorities may execute at the
 * same time, if the AsyncTaskManager has more than one thread servicing
 * tasks.
 *
 * Also see AsyncTaskChain::set_timeslice_priority(), which changes the
 * meaning of this value.  In the default mode, when the timeslice_priority
 * flag is false, all tasks always run once per epoch, regardless of their
 * priority values (that is, the priority controls the order of the task
 * execution only, not the number of times it runs).  On the other hand, if
 * you set the timeslice_priority flag to true, then changing a task's
 * priority has an effect on the number of times it runs.
 */
void AsyncTask::
set_priority(int priority) {
  if (priority != _priority) {
    if (_manager != nullptr) {
      MutexHolder holder(_manager->_lock);
      if (_state == S_active && _sort >= _chain->_current_sort) {
        // Changing priority on an "active" (i.e.  enqueued) task means
        // removing it and re-inserting it into the queue.
        PT(AsyncTask) hold_task = this;
        AsyncTaskChain *chain = _manager->do_find_task_chain(_chain_name);
        nassertv(chain != nullptr);
        chain->do_remove(this);
        _priority = priority;
        chain->do_add(this);

      } else {
        // If it's sleeping, currently being serviced, or something else, we
        // can just change the priority value directly.
        _priority = priority;
      }
    } else {
      // If it hasn't been started anywhere, we can just change the priority
      // value.
      _priority = priority;
    }
  }
}

/**
 *
 */
void AsyncTask::
output(std::ostream &out) const {
  out << get_type();
  if (has_name()) {
    out << " " << get_name();
  }
}

/**
 * Switches the AsyncTask to its new task chain, named by _chain_name.  Called
 * internally only.
 */
void AsyncTask::
jump_to_task_chain(AsyncTaskManager *manager) {
  AsyncTaskChain *chain_b = manager->do_find_task_chain(_chain_name);
  if (chain_b == nullptr) {
    task_cat.warning()
      << "Creating implicit AsyncTaskChain " << _chain_name
      << " for " << manager->get_type() << " "
      << manager->get_name() << "\n";
    chain_b = manager->do_make_task_chain(_chain_name);
  }
  chain_b->do_add(this);
}

/**
 * Called by the AsyncTaskManager to actually run the task.  Assumes the lock
 * is held.  See do_task().
 */
AsyncTask::DoneStatus AsyncTask::
unlock_and_do_task() {
  nassertr(_manager != nullptr, DS_done);
  PT(ClockObject) clock = _manager->get_clock();

  // Indicate that this task is now the current task running on the thread.
  Thread *current_thread = Thread::get_current_thread();
  nassertr(current_thread->_current_task == nullptr, DS_interrupt);

#ifdef __GNUC__
  __attribute__((unused))
#endif
  void *ptr = AtomicAdjust::compare_and_exchange_ptr
    (current_thread->_current_task, nullptr, (TypedReferenceCount *)this);

  // If the return value is other than nullptr, someone else must have
  // assigned the task first, in another thread.  That shouldn't be possible.

  // But different versions of gcc appear to have problems compiling these
  // assertions correctly.
#ifndef __GNUC__
  nassertr(ptr == nullptr, DS_interrupt);
  nassertr(current_thread->_current_task == this, DS_interrupt);
#endif  // __GNUC__

  // It's important to release the lock while the task is being serviced.
  _manager->_lock.unlock();

  double start = clock->get_real_time();
  _task_pcollector.start();
  DoneStatus status = do_task();
  _task_pcollector.stop();
  double end = clock->get_real_time();

  // Now reacquire the lock (so we can return with the lock held).
  _manager->_lock.lock();

  _dt = end - start;
  _max_dt = std::max(_dt, _max_dt);
  _total_dt += _dt;

  _chain->_time_in_frame += _dt;

  // Now indicate that this is no longer the current task.
  nassertr(current_thread->_current_task == this, status);

  ptr = AtomicAdjust::compare_and_exchange_ptr
    (current_thread->_current_task, (TypedReferenceCount *)this, nullptr);

  // If the return value is other than this, someone else must have assigned
  // the task first, in another thread.  That shouldn't be possible.

  // But different versions of gcc appear to have problems compiling these
  // assertions correctly.
#ifndef __GNUC__
  nassertr(ptr == this, DS_interrupt);
  nassertr(current_thread->_current_task == nullptr, DS_interrupt);
#endif  // __GNUC__

  return status;
}

/**
 * Cancels this task.  This is equivalent to remove().
 */
bool AsyncTask::
cancel() {
  bool result = remove();
  nassertr(done(), false);
  return result;
}

/**
 * Override this function to return true if the task can be successfully
 * executed, false if it cannot.  Mainly intended as a sanity check when
 * attempting to add the task to a task manager.
 *
 * This function is called with the lock held.
 */
bool AsyncTask::
is_runnable() {
  return true;
}

/**
 * Override this function to do something useful for the task.  The return
 * value should be one of:
 *
 * DS_done: the task is finished, remove from active and throw the done event.
 *
 * DS_cont: the task has more work to do, keep it active and call this
 * function again in the next epoch.
 *
 * DS_again: like DS_cont, but next time call the function from the beginning,
 * almost as if it were freshly added to the task manager.  The task's
 * get_start_time() will be reset to now, and its get_elapsed_time() will be
 * reset to 0.  If the task has a set_delay(), it will wait again for that
 * amount of time to elapse before restarting.  Timing accounting, however, is
 * not reset.
 *
 * DS_pickup: like DS_cont, but if the task chain has a frame budget and that
 * budget has not yet been met, re-run the task again without waiting for the
 * next frame.  Otherwise, run it next epoch as usual.
 *
 * DS_exit: stop the task, and stop the enclosing sequence too.  Outside of a
 * sequence, this is the same as DS_done.
 *
 * DS_pause: delay the task for set_delay() seconds, then stop it.  This is
 * only useful within a sequence.
 *
 * DS_interrupt: Interrupt the whole AsyncTaskManager.  The task will continue
 * again next epoch, as if it had returned DS_cont.
 *
 * This function is called with the lock *not* held.
 */
AsyncTask::DoneStatus AsyncTask::
do_task() {
  return DS_done;
}

/**
 * Override this function to do something useful when the task has been added
 * to the active queue.
 *
 * This function is called with the lock *not* held.
 */
void AsyncTask::
upon_birth(AsyncTaskManager *manager) {
  // Throw a generic add event for the manager.
  string add_name = manager->get_name() + "-addTask";
  PT_Event event = new Event(add_name);
  event->add_parameter(EventParameter(this));
  throw_event(event);
}

/**
 * Override this function to do something useful when the task has been
 * removed from the active queue.  The parameter clean_exit is true if the
 * task has been removed because it exited normally (returning DS_done), or
 * false if it was removed for some other reason (e.g.
 * AsyncTaskManager::remove()).  By the time this method is called, _manager
 * may have been cleared, so the parameter manager indicates the original
 * AsyncTaskManager that owned this task.
 *
 * This function is called with the lock *not* held.
 */
void AsyncTask::
upon_death(AsyncTaskManager *manager, bool clean_exit) {
  //NB. done_event is now being thrown in AsyncFuture::notify_done().

  // Throw a generic remove event for the manager.
  if (manager != nullptr) {
    string remove_name = manager->get_name() + "-removeTask";
    PT_Event event = new Event(remove_name);
    event->add_parameter(EventParameter(this));
    throw_event(event);
  }
}
