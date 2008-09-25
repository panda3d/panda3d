// Filename: asyncTaskChain.cxx
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

#include "asyncTaskChain.h"
#include "asyncTaskManager.h"
#include "event.h"
#include "pt_Event.h"
#include "throw_event.h"
#include "eventParameter.h"
#include "mutexHolder.h"
#include "indent.h"
#include "pStatClient.h"
#include "pStatTimer.h"
#include "clockObject.h"
#include <algorithm>

TypeHandle AsyncTaskChain::_type_handle;

PStatCollector AsyncTaskChain::_task_pcollector("Task");
PStatCollector AsyncTaskChain::_wait_pcollector("Wait");

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskChain::
AsyncTaskChain(AsyncTaskManager *manager, const string &name) :
  Namable(name),
  _manager(manager),
  _tick_clock(false),
  _num_threads(0),
  _cvar(manager->_lock),
  _num_tasks(0),
  _num_busy_threads(0),
  _state(S_initial),
  _current_sort(INT_MAX),
  _needs_cleanup(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskChain::
~AsyncTaskChain() {
  // We only grab the lock if _needs_cleanup is true.  This way, the
  // temporary AsyncTaskChain objects created (and destructed) within
  // the task manager won't risk a double-lock.
  if (_needs_cleanup) {
    MutexHolder holder(_manager->_lock);
    do_cleanup();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::set_num_threads
//       Access: Published
//  Description: Changes the number of threads for this task chain.
//               This may require stopping the threads if they are
//               already running.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
set_num_threads(int num_threads) {
  nassertv(num_threads >= 0);

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

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::get_num_threads
//       Access: Published
//  Description: Returns the number of threads that will be servicing
//               tasks for this chain.  Also see
//               get_num_running_threads().
////////////////////////////////////////////////////////////////////
int AsyncTaskChain::
get_num_threads() const {
  MutexHolder holder(_manager->_lock);
  return _num_threads;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::get_num_running_threads
//       Access: Published
//  Description: Returns the number of threads that have been created
//               and are actively running.  This will return 0 before
//               the threads have been started; it will also return 0
//               if thread support is not available.
////////////////////////////////////////////////////////////////////
int AsyncTaskChain::
get_num_running_threads() const {
  MutexHolder holder(_manager->_lock);
  return _threads.size();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::stop_threads
//       Access: Published
//  Description: Stops any threads that are currently running.  If any
//               tasks are still pending and have not yet been picked
//               up by a thread, they will not be serviced unless
//               poll() or start_threads() is later called.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
stop_threads() {
  if (_state == S_started || _state == S_aborting) {
    // Clean up all of the threads.
    MutexHolder holder(_manager->_lock);
    do_stop_threads();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::start_threads
//       Access: Published
//  Description: Starts any requested threads to service the tasks on
//               the queue.  This is normally not necessary, since
//               adding a task will start the threads automatically.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
start_threads() {
  if (_state == S_initial || _state == S_aborting) {
    MutexHolder holder(_manager->_lock);
    do_start_threads();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::has_task
//       Access: Published
//  Description: Returns true if the indicated task has been added to
//               this AsyncTaskChain, false otherwise.
////////////////////////////////////////////////////////////////////
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

  // The task might not actually be in the active queue, since it
  // might be being serviced right now.  That's OK.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::wait_for_tasks
//       Access: Published
//  Description: Blocks until the task list is empty.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
wait_for_tasks() {
  MutexHolder holder(_manager->_lock);
  do_wait_for_tasks();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::get_num_tasks
//       Access: Published
//  Description: Returns the number of tasks that are currently active
//               or sleeping within the task chain.
////////////////////////////////////////////////////////////////////
int AsyncTaskChain::
get_num_tasks() const {
  MutexHolder holder(_manager->_lock);
  return _num_tasks;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::get_tasks
//       Access: Published
//  Description: Returns the set of tasks that are active or sleeping
//               on the task chain, at the time of the call.
////////////////////////////////////////////////////////////////////
AsyncTaskCollection AsyncTaskChain::
get_tasks() const {
  MutexHolder holder(_manager->_lock);
  AsyncTaskCollection result = do_get_active_tasks();
  result.add_tasks_from(do_get_sleeping_tasks());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::get_active_tasks
//       Access: Published
//  Description: Returns the set of tasks that are active (and not
//               sleeping) on the task chain, at the time of the
//               call.
////////////////////////////////////////////////////////////////////
AsyncTaskCollection AsyncTaskChain::
get_active_tasks() const {
  MutexHolder holder(_manager->_lock);
  return do_get_active_tasks();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::get_sleeping_tasks
//       Access: Published
//  Description: Returns the set of tasks that are sleeping (and not
//               active) on the task chain, at the time of the
//               call.
////////////////////////////////////////////////////////////////////
AsyncTaskCollection AsyncTaskChain::
get_sleeping_tasks() const {
  MutexHolder holder(_manager->_lock);
  return do_get_sleeping_tasks();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::poll
//       Access: Published
//  Description: Runs through all the tasks in the task list, once, if
//               the task chain is running in single-threaded mode
//               (no threads available).  This method does nothing in
//               threaded mode, so it may safely be called in either
//               case.
//
//               Normally, you would not call this function directly;
//               instead, call AsyncTaskManager::poll(), which polls
//               all of the task chains in sequence.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
poll() {
  MutexHolder holder(_manager->_lock);
  do_poll();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
output(ostream &out) const {
  MutexHolder holder(_manager->_lock);

  out << get_type() << " " << get_name()
      << "; " << _num_tasks << " tasks";
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
write(ostream &out, int indent_level) const {
  MutexHolder holder(_manager->_lock);
  do_write(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_add
//       Access: Protected
//  Description: Adds the indicated task to the active queue.  It is
//               an error if the task is already added to this or any
//               other active queue.
//
//               This is normally called only by the AsyncTaskManager.
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
do_add(AsyncTask *task) {
  nassertv(task->_chain == NULL &&
           task->_manager == NULL &&
           task->_chain_name == get_name() &&
           task->_state == AsyncTask::S_inactive);
  nassertv(!do_has_task(task));

  do_start_threads();

  task->_chain = this;
  task->_manager = _manager;

  double now = _manager->_clock->get_frame_time();
  task->_start_time = now;

  if (task->has_delay()) {
    // This is a deferred task.  Add it to the sleeping queue.
    task->_wake_time = now + task->get_delay();
    task->_state = AsyncTask::S_sleeping;
    _sleeping.push_back(task);
    push_heap(_sleeping.begin(), _sleeping.end(), AsyncTaskSortWakeTime());

  } else {
    // This is an active task.  Add it to the active set.
    task->_state = AsyncTask::S_active;
    if (task->get_sort() > _current_sort) {
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
  _cvar.signal_all();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_remove
//       Access: Protected
//  Description: Removes the indicated task from this chain.  Returns
//               true if removed, false otherwise.  Assumes the lock
//               is already held.
////////////////////////////////////////////////////////////////////
bool AsyncTaskChain::
do_remove(AsyncTask *task) {
  bool removed = false;

  nassertr(task->_chain == this, false);

  switch (task->_state) {
  case AsyncTask::S_servicing:
    // This task is being serviced.
    task->_state = AsyncTask::S_servicing_removed;
    removed = true;
    break;
    
  case AsyncTask::S_servicing_removed:
    // Being serviced, though it will be removed later.
    break;
    
  case AsyncTask::S_sleeping:
    // Sleeping, easy.
    {
      int index = find_task_on_heap(_sleeping, task);
      nassertr(index != -1, false);
      _sleeping.erase(_sleeping.begin() + index);
      make_heap(_sleeping.begin(), _sleeping.end(), AsyncTaskSortWakeTime());
      removed = true;
      cleanup_task(task, false);
    }
    break;
    
  case AsyncTask::S_active:
    {
      // Active, but not being serviced, easy.
      int index = find_task_on_heap(_active, task);
      if (index != -1) {
        _active.erase(_active.begin() + index);
        make_heap(_active.begin(), _active.end(), AsyncTaskSortPriority());
      } else {
        index = find_task_on_heap(_next_active, task);
        nassertr(index != -1, false);
        _next_active.erase(_next_active.begin() + index);
      }
      removed = true;
      cleanup_task(task, false);
    }
  }

  return removed;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_wait_for_tasks
//       Access: Protected
//  Description: Blocks until the task list is empty.  Assumes the
//               lock is held.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
do_wait_for_tasks() {
  do_start_threads();

  if (_threads.empty()) {
    // Non-threaded case.
    while (_num_tasks > 0) {
      if (_state == S_shutdown || _state == S_aborting) {
        return;
      }
      do_poll();
    }

  } else {
    // Threaded case.
    while (_num_tasks > 0) {
      if (_state == S_shutdown || _state == S_aborting) {
        return;
      }
      
      PStatTimer timer(_wait_pcollector);
      _cvar.wait();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_cleanup
//       Access: Protected
//  Description: Stops all threads and messily empties the task list.
//               This is intended to be called on destruction only.
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
do_cleanup() {
  do_stop_threads();

  TaskHeap::const_iterator ti;
  for (ti = _active.begin(); ti != _active.end(); ++ti) {
    AsyncTask *task = (*ti);
    cleanup_task(task, false);
  }
  for (ti = _next_active.begin(); ti != _next_active.end(); ++ti) {
    AsyncTask *task = (*ti);
    cleanup_task(task, false);
  }
  for (ti = _sleeping.begin(); ti != _sleeping.end(); ++ti) {
    AsyncTask *task = (*ti);
    cleanup_task(task, false);
  }

  _needs_cleanup = false;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_has_task
//       Access: Protected
//  Description: Returns true if the task is on one of the task lists,
//               false if it is not (false may mean that the task is
//               currently being serviced).  Assumes the lock is
//               currently held.
////////////////////////////////////////////////////////////////////
bool AsyncTaskChain::
do_has_task(AsyncTask *task) const {
  return (find_task_on_heap(_active, task) != -1 ||
          find_task_on_heap(_next_active, task) != -1 ||
          find_task_on_heap(_sleeping, task) != -1);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::find_task_on_heap
//       Access: Protected
//  Description: Returns the index number of the indicated task within
//               the specified task list, or -1 if the task is not
//               found in the list (this may mean that it is currently
//               being serviced).  Assumes that the lock is currently
//               held.
////////////////////////////////////////////////////////////////////
int AsyncTaskChain::
find_task_on_heap(const TaskHeap &heap, AsyncTask *task) const {
  for (int i = 0; i < (int)heap.size(); ++i) {
    if (heap[i] == task) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::service_one_task
//       Access: Protected
//  Description: Pops a single task off the active queue, services it,
//               and restores it to the end of the queue.  This is
//               called internally only within one of the task
//               threads.  Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
service_one_task(AsyncTaskChain::AsyncTaskChainThread *thread) {
  if (!_active.empty()) {
    PT(AsyncTask) task = _active.front();
    pop_heap(_active.begin(), _active.end(), AsyncTaskSortPriority());
    _active.pop_back();

    if (thread != (AsyncTaskChain::AsyncTaskChainThread *)NULL) {
      thread->_servicing = task;
    }

    nassertv(task->get_sort() == _current_sort);
    nassertv(task->_state == AsyncTask::S_active);
    task->_state = AsyncTask::S_servicing;
    task->_servicing_thread = thread;

    // Now release the chain lock while we actually service the
    // task.
    _manager->_lock.release();
    AsyncTask::DoneStatus ds = task->do_task();

    // Now we have to re-acquire the chain lock, so we can put the
    // task back on the queue (and so we can return with the lock
    // still held).
    _manager->_lock.lock();

    if (thread != (AsyncTaskChain::AsyncTaskChainThread *)NULL) {
      thread->_servicing = NULL;
    }
    task->_servicing_thread = NULL;

    if (task->_chain == this) {
      // TODO: check task->_chain_name to see if the task wants to
      // jump chains.

      if (task->_state == AsyncTask::S_servicing_removed) {
        // This task wants to kill itself.
        cleanup_task(task, false);

      } else {
        switch (ds) {
        case AsyncTask::DS_cont:
          // The task is still alive; put it on the next frame's active
          // queue.
          task->_state = AsyncTask::S_active;
          _next_active.push_back(task);
          _cvar.signal_all();
          break;
          
        case AsyncTask::DS_again:
          // The task wants to sleep again.
          {
            double now = _manager->_clock->get_frame_time();
            task->_wake_time = now + task->get_delay();
            task->_state = AsyncTask::S_sleeping;
            _sleeping.push_back(task);
            push_heap(_sleeping.begin(), _sleeping.end(), AsyncTaskSortWakeTime());
            _cvar.signal_all();
          }
          break;

        case AsyncTask::DS_abort:
          // The task had an exception and wants to raise a big flag.
          cleanup_task(task, false);
          if (_state == S_started) {
            _state = S_aborting;
            _cvar.signal_all();
          }
          break;
          
        default:
          // The task has finished.
          cleanup_task(task, true);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::cleanup_task
//       Access: Protected
//  Description: Called internally when a task has completed (or been
//               interrupted) and is about to be removed from the
//               active queue.  Assumes the lock is held.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
cleanup_task(AsyncTask *task, bool clean_exit) {
  nassertv(task->_chain == this);

  task->_state = AsyncTask::S_inactive;
  task->_chain = NULL;
  task->_manager = NULL;
  --_num_tasks;
  --(_manager->_num_tasks);

  _manager->remove_task_by_name(task);

  if (clean_exit && !task->_done_event.empty()) {
    PT_Event event = new Event(task->_done_event);
    event->add_parameter(EventParameter(task));
    throw_event(event);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::finish_sort_group
//       Access: Protected
//  Description: Called internally when all tasks of a given sort
//               value have been completed, and it is time to
//               increment to the next sort value, or begin the next
//               epoch.  Assumes the lock is held.
//
//               Returns true if there are more tasks on the queue
//               after this operation, or false if the task list is
//               empty and we need to wait.
////////////////////////////////////////////////////////////////////
bool AsyncTaskChain::
finish_sort_group() {
  nassertr(_num_busy_threads == 0, true);

  if (!_active.empty()) {
    // There are more tasks; just set the next sort value.
    nassertr(_current_sort < _active.front()->get_sort(), true);
    _current_sort = _active.front()->get_sort();
    _cvar.signal_all();
    return true;
  }

  // There are no more tasks in this epoch; advance to the next epoch.
  if (_tick_clock) {
    _manager->_clock->tick();
  }
  if (!_threads.empty()) {
    PStatClient::thread_tick(get_name());
  }
    
  _active.swap(_next_active);

  // Check for any sleeping tasks that need to be woken.
  double now = _manager->_clock->get_frame_time();
  while (!_sleeping.empty() && _sleeping.front()->get_wake_time() <= now) {
    PT(AsyncTask) task = _sleeping.front();
    pop_heap(_sleeping.begin(), _sleeping.end(), AsyncTaskSortWakeTime());
    _sleeping.pop_back();
    task->_state = AsyncTask::S_active;
    _active.push_back(task);
  }

  make_heap(_active.begin(), _active.end(), AsyncTaskSortPriority());
  nassertr(_num_tasks == _active.size() + _sleeping.size(), true);

  if (!_active.empty()) {
    // Get the first task on the queue.
    _current_sort = _active.front()->get_sort();
    _cvar.signal_all();
    return true;
  }

  // There are no tasks to be had anywhere.  Chill.
  _current_sort = INT_MAX;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_stop_threads
//       Access: Protected
//  Description: The private implementation of stop_threads; assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
do_stop_threads() {
  if (_state == S_started || _state == S_aborting) {
    _state = S_shutdown;
    _cvar.signal_all();
    
    Threads wait_threads;
    wait_threads.swap(_threads);
    
    // We have to release the lock while we join, so the threads can
    // wake up and see that we're shutting down.
    _manager->_lock.release();
    Threads::iterator ti;
    for (ti = wait_threads.begin(); ti != wait_threads.end(); ++ti) {
      (*ti)->join();
    }
    _manager->_lock.lock();
    
    _state = S_initial;
    nassertv(_num_busy_threads == 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_start_threads
//       Access: Protected
//  Description: The private implementation of start_threads; assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
do_start_threads() {
  if (_state == S_aborting) {
    do_stop_threads();
  }

  if (_state == S_initial) {
    _state = S_started;
    _num_busy_threads = 0;
    if (Thread::is_threading_supported()) {
      _needs_cleanup = true;
      _threads.reserve(_num_threads);
      for (int i = 0; i < _num_threads; ++i) {
        ostringstream strm;
        strm << _manager->get_name();
        if (has_name()) {
          strm << "_" << get_name();
        }
        strm << "_" << i;
        PT(AsyncTaskChainThread) thread = new AsyncTaskChainThread(strm.str(), this);
        if (thread->start(TP_low, true)) {
          _threads.push_back(thread);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_get_active_tasks
//       Access: Protected
//  Description: Returns the set of tasks that are active (and not
//               sleeping) on the task chain, at the time of the
//               call.  Assumes the lock is held.
////////////////////////////////////////////////////////////////////
AsyncTaskCollection AsyncTaskChain::
do_get_active_tasks() const {
  AsyncTaskCollection result;

  Threads::const_iterator thi;
  for (thi = _threads.begin(); thi != _threads.end(); ++thi) {
    AsyncTask *task = (*thi)->_servicing;
    if (task != (AsyncTask *)NULL) {
      result.add_task(task);
    }
  }
  TaskHeap::const_iterator ti;
  for (ti = _active.begin(); ti != _active.end(); ++ti) {
    AsyncTask *task = (*ti);
    result.add_task(task);
  }
  for (ti = _next_active.begin(); ti != _next_active.end(); ++ti) {
    AsyncTask *task = (*ti);
    result.add_task(task);
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_get_sleeping_tasks
//       Access: Published
//  Description: Returns the set of tasks that are sleeping (and not
//               active) on the task chain, at the time of the
//               call.  Assumes the lock is held.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_poll
//       Access: Protected
//  Description: The private implementation of poll(), this assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
do_poll() {
  do_start_threads();

  if (!_threads.empty()) {
    return;
  }
  
  while (!_active.empty() && _state != S_shutdown && _state != S_aborting) {
    _current_sort = _active.front()->get_sort();
    service_one_task(NULL);
  }

  if (_state != S_shutdown && _state != S_aborting) {
    finish_sort_group();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::do_write
//       Access: Protected
//  Description: The private implementation of write(), this assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::
do_write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << "\n";

  // Collect a list of all active tasks, then sort them into order for
  // output.
  TaskHeap tasks = _active;
  tasks.insert(tasks.end(), _next_active.begin(), _next_active.end());

  Threads::const_iterator thi;
  for (thi = _threads.begin(); thi != _threads.end(); ++thi) {
    AsyncTask *task = (*thi)->_servicing;
    if (task != (AsyncTask *)NULL && 
        task->_state != AsyncTask::S_servicing_removed) {
      tasks.push_back(task);
    }
  }

  if (!tasks.empty()) {
    indent(out, indent_level + 2)
      << "Active tasks:\n";

    sort(tasks.begin(), tasks.end(), AsyncTaskSortPriority());

    // Since AsyncTaskSortPriority() sorts backwards (because of STL's
    // push_heap semantics), we go through the task list in reverse
    // order to print them forwards.
    TaskHeap::reverse_iterator ti;
    int current_sort = tasks.back()->get_sort() - 1;
    for (ti = tasks.rbegin(); ti != tasks.rend(); ++ti) {
      AsyncTask *task = (*ti);
      if (task->get_sort() != current_sort) {
        current_sort = task->get_sort();
        indent(out, indent_level + 2)
          << "sort = " << current_sort << "\n";
      }
      if (task->_state == AsyncTask::S_servicing) {
        indent(out, indent_level + 3)
          << "*" << *task << "\n";
      } else {
        indent(out, indent_level + 4)
          << *task << "\n";
      }
    }
  }

  if (!_sleeping.empty()) {
    indent(out, indent_level + 2)
      << "Sleeping tasks:\n";
    double now = _manager->_clock->get_frame_time();

    // Instead of iterating through the _sleeping list in heap order,
    // copy it and then use repeated pops to get it out in sorted
    // order, for the user's satisfaction.
    TaskHeap sleeping = _sleeping;
    while (!sleeping.empty()) {
      PT(AsyncTask) task = sleeping.front();
      pop_heap(sleeping.begin(), sleeping.end(), AsyncTaskSortWakeTime());
      sleeping.pop_back();

      indent(out, indent_level + 4)
        << task->get_wake_time() - now << "s: "
        << *task << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::AsyncTaskChainThread::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
AsyncTaskChain::AsyncTaskChainThread::
AsyncTaskChainThread(const string &name, AsyncTaskChain *chain) :
  Thread(name, chain->get_name()),
  _chain(chain),
  _servicing(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::AsyncTaskChainThread::thread_main
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AsyncTaskChain::AsyncTaskChainThread::
thread_main() {
  MutexHolder holder(_chain->_manager->_lock);
  while (_chain->_state != S_shutdown && _chain->_state != S_aborting) {
    if (!_chain->_active.empty() &&
        _chain->_active.front()->get_sort() == _chain->_current_sort) {
      PStatTimer timer(_task_pcollector);
      _chain->_num_busy_threads++;
      _chain->service_one_task(this);
      _chain->_num_busy_threads--;
      _chain->_cvar.signal_all();

    } else {
      // We've finished all the available tasks of the current sort
      // value.  We can't pick up a new task until all of the threads
      // finish the tasks with the same sort value.
      if (_chain->_num_busy_threads == 0) {
        // We're the last thread to finish.  Update _current_sort.
        if (!_chain->finish_sort_group()) {
          // Nothing to do.  Wait for more tasks to be added.
          if (_chain->_sleeping.empty()) {
            PStatTimer timer(_wait_pcollector);
            _chain->_cvar.wait();
          } else {
            double wake_time = _chain->_sleeping.front()->get_wake_time();
            double now = _chain->_manager->_clock->get_frame_time();
            double timeout = max(wake_time - now, 0.0);
            PStatTimer timer(_wait_pcollector);
            _chain->_cvar.wait(timeout);
          }            
        }

      } else {
        // Wait for the other threads to finish their current task
        // before we continue.
        PStatTimer timer(_wait_pcollector);
        _chain->_cvar.wait();
      }
    }
  }
}

