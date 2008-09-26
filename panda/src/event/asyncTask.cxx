// Filename: asyncTask.cxx
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

#include "asyncTask.h"
#include "asyncTaskManager.h"
#include "config_event.h"

PStatCollector AsyncTask::_show_code_pcollector("App:Show code");
TypeHandle AsyncTask::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
AsyncTask::
AsyncTask(const string &name) : 
  _delay(0.0),
  _has_delay(false),
  _wake_time(0.0),
  _sort(0),
  _priority(0),
  _state(S_inactive),
  _servicing_thread(NULL),
  _manager(NULL),
  _chain(NULL),
  _dt(0.0),
  _max_dt(0.0),
  _total_dt(0.0),
  _num_frames(0)
{
#ifdef HAVE_PYTHON
  _python_object = NULL;
#endif  // HAVE_PYTHON
  set_name(name);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
AsyncTask::
~AsyncTask() {
  nassertv(_state == S_inactive && _manager == NULL && _chain == NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::remove
//       Access: Published
//  Description: Removes the task from its active manager, if any, and
//               makes the state S_inactive (or possible
//               S_servicing_removed).  This is a no-op if the state
//               is already S_inactive.
////////////////////////////////////////////////////////////////////
void AsyncTask::
remove() {
  if (_manager != (AsyncTaskManager *)NULL) {
    _manager->remove(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_elapsed_time
//       Access: Published
//  Description: Returns the amount of time that has elapsed since
//               the task was started, according to the task manager's
//               clock.
//
//               It is only valid to call this if the task's status is
//               not S_inactive.
////////////////////////////////////////////////////////////////////
double AsyncTask::
get_elapsed_time() const {
  nassertr(_state != S_inactive, 0.0);
  nassertr(_manager != (AsyncTaskManager *)NULL, 0.0);
  return _manager->_clock->get_frame_time() - _start_time;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::set_name
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void AsyncTask::
set_name(const string &name) {
  if (_manager != (AsyncTaskManager *)NULL) {
    MutexHolder holder(_manager->_lock);
    if (Namable::get_name() != name) {
      // Changing an active task's name requires moving it around on
      // its name index.

      _manager->remove_task_by_name(this);
      Namable::set_name(name);
      _manager->add_task_by_name(this);
    }
  } else {
    // If it hasn't been started anywhere, we can just change the
    // name.
    Namable::set_name(name);
  }

#ifdef DO_PSTATS
  // Update the PStatCollector with the new name.  If the name ends
  // with a hyphen followed by a string of digits, we strip all that
  // off, for the parent collector, to group related tasks together in
  // the pstats graph.  We still create a child collector that
  // contains the full name, however.
  size_t end = name.size();
  size_t p = end;
  while (true) {
    while (p > 0 && isdigit(name[p - 1])) {
      --p;
    }
    if (p > 0 && (name[p - 1] == '-' || name[p - 1] == '_')) {
      --p;
      end = p;
    } else {
      p = end;
      break;
    }
  }
  PStatCollector parent(_show_code_pcollector, name.substr(0, end));
  _task_pcollector = PStatCollector(parent, name);
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::set_task_chain
//       Access: Published
//  Description: Specifies the AsyncTaskChain on which this task will
//               be running.  Each task chain runs tasks independently
//               of the others.
////////////////////////////////////////////////////////////////////
void AsyncTask::
set_task_chain(const string &chain_name) {
  if (chain_name != _chain_name) {
    if (_manager != (AsyncTaskManager *)NULL) {
      MutexHolder holder(_manager->_lock);
      if (_state == S_active) {
        // Changing chains on an "active" (i.e. enqueued) task means
        // removing it and re-inserting it into the queue.
        PT(AsyncTask) hold_task = this;
        PT(AsyncTaskManager) manager = _manager;

        AsyncTaskChain *chain_a = manager->do_find_task_chain(_chain_name);
        nassertv(chain_a != (AsyncTaskChain *)NULL);
        chain_a->do_remove(this);
        _chain_name = chain_name;

        AsyncTaskChain *chain_b = manager->do_find_task_chain(_chain_name);
        if (chain_b == (AsyncTaskChain *)NULL) {
          event_cat.warning()
            << "Creating implicit AsyncTaskChain " << _chain_name
            << " for " << manager->get_type() << " "
            << manager->get_name() << "\n";
          chain_b = manager->do_make_task_chain(_chain_name);
        }
        chain_b->do_add(this);

      } else {
        // If it's sleeping, currently being serviced, or something
        // else, we can just change the chain_name value directly.
        _chain_name = chain_name;
      }
    } else {
      // If it hasn't been started anywhere, we can just change the
      // chain_name value.
      _chain_name = chain_name;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::set_sort
//       Access: Published
//  Description: Specifies a sort value for this task.  Within a given
//               AsyncTaskManager, all of the tasks with a given sort
//               value are guaranteed to be completed before any tasks
//               with a higher sort value are begun.
//
//               To put it another way, two tasks might execute in
//               parallel with each other only if they both have the
//               same sort value.  Tasks with a lower sort value are
//               executed first.
//
//               This is different from the priority, which makes no
//               such exclusion guarantees.
////////////////////////////////////////////////////////////////////
void AsyncTask::
set_sort(int sort) {
  if (sort != _sort) {
    if (_manager != (AsyncTaskManager *)NULL) {
      MutexHolder holder(_manager->_lock);
      if (_state == S_active && _sort >= _chain->_current_sort) {
        // Changing sort on an "active" (i.e. enqueued) task means
        // removing it and re-inserting it into the queue.
        PT(AsyncTask) hold_task = this;
        AsyncTaskChain *chain = _manager->do_find_task_chain(_chain_name);
        nassertv(chain != (AsyncTaskChain *)NULL);
        chain->do_remove(this);
        _sort = sort;
        chain->do_add(this);

      } else {
        // If it's sleeping, currently being serviced, or something
        // else, we can just change the sort value directly.
        _sort = sort;
      }
    } else {
      // If it hasn't been started anywhere, we can just change the
      // sort value.
      _sort = sort;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::set_priority
//       Access: Published
//  Description: Specifies a priority value for this task.  In
//               general, tasks with a higher priority value are
//               executed before tasks with a lower priority value
//               (but only for tasks with the same sort value).
//
//               Unlike the sort value, tasks with different
//               priorities may execute at the same time, if the
//               AsyncTaskManager has more than one thread servicing
//               tasks.
////////////////////////////////////////////////////////////////////
void AsyncTask::
set_priority(int priority) {
  if (priority != _priority) {
    if (_manager != (AsyncTaskManager *)NULL) {
      MutexHolder holder(_manager->_lock);
      if (_state == S_active && _sort >= _chain->_current_sort) {
        // Changing priority on an "active" (i.e. enqueued) task means
        // removing it and re-inserting it into the queue.
        PT(AsyncTask) hold_task = this;
        AsyncTaskChain *chain = _manager->do_find_task_chain(_chain_name);
        nassertv(chain != (AsyncTaskChain *)NULL);
        chain->do_remove(this);
        _priority = priority;
        chain->do_add(this);

      } else {
        // If it's sleeping, currently being serviced, or something
        // else, we can just change the priority value directly.
        _priority = priority;
      }
    } else {
      // If it hasn't been started anywhere, we can just change the
      // priority value.
      _priority = priority;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AsyncTask::
output(ostream &out) const {
  out << get_type();
  if (has_name()) {
    out << " " << get_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::unlock_and_do_task
//       Access: Protected
//  Description: Called by the AsyncTaskManager to actually run the
//               task.  Assumes the lock is held.  See do_task().
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus AsyncTask::
unlock_and_do_task() {
  nassertr(_manager != (AsyncTaskManager *)NULL, DS_done);
  PT(ClockObject) clock = _manager->get_clock();

  // It's important to release the lock while the task is being
  // serviced.
  _manager->_lock.release();
  
  double start = clock->get_real_time();
  _task_pcollector.start();
  DoneStatus status = do_task();
  _task_pcollector.stop();
  double end = clock->get_real_time();

  // Now reacquire the lock (so we can return with the lock held).
  _manager->_lock.lock();

  _dt = end - start;
  _max_dt = max(_dt, _max_dt);
  _total_dt += _dt;
  ++_num_frames;

  return status;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::do_task
//       Access: Protected, Virtual
//  Description: Override this function to do something useful for the
//               task.  The return value should be one of:
//
//               DS_done: the task is finished, remove from active and
//               throw the done event.
//
//               DS_cont: the task has more work to do, keep it active
//               and call this function again in the next epoch.
//
//               DS_again: put the task to sleep for get_delay()
//               seconds, then put it back on the active queue.
//
//               DS_abort: abort the task, and interrupt the whole
//               AsyncTaskManager.
//
//               This function is called with the lock *not* held.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus AsyncTask::
do_task() {
  return DS_done;
}
