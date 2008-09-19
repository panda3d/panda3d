// Filename: asyncTaskManager.cxx
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

TypeHandle AsyncTaskManager::_type_handle;

PStatCollector AsyncTaskManager::_task_pcollector("Task");
PStatCollector AsyncTaskManager::_wait_pcollector("Wait");

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskManager::
AsyncTaskManager(const string &name, int num_threads) :
  Namable(name),
  _num_threads(0),
  _cvar(_lock),
  _num_tasks(0),
  _num_busy_threads(0),
  _state(S_initial),
  _current_sort(INT_MAX),
  _clock(ClockObject::get_global_clock()),
  _tick_clock(false)
{
  set_num_threads(num_threads);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskManager::
~AsyncTaskManager() {
  stop_threads();

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
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::set_num_threads
//       Access: Published
//  Description: Changes the number of threads for this task manager.
//               This may require stopping the threads if they are
//               already running.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
set_num_threads(int num_threads) {
  nassertv(num_threads >= 0);

  if (!Thread::is_threading_supported()) {
    num_threads = 0;
  }

  MutexHolder holder(_lock);
  if (_num_threads != num_threads) {
    do_stop_threads();
    _num_threads = num_threads;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::stop_threads
//       Access: Published
//  Description: Stops any threads that are currently running.  If any
//               tasks are still pending and have not yet been picked
//               up by a thread, they will not be serviced unless
//               poll() or start_threads() is later called.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
stop_threads() {
  if (_state == S_started || _state == S_aborting) {
    // Clean up all of the threads.
    MutexHolder holder(_lock);
    do_stop_threads();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::start_threads
//       Access: Published
//  Description: Starts any requested threads to service the tasks on
//               the queue.  This is normally not necessary, since
//               adding a task will start the threads automatically.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
start_threads() {
  if (_state == S_initial || _state == S_aborting) {
    MutexHolder holder(_lock);
    do_start_threads();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::add
//       Access: Published
//  Description: Adds the indicated task to the active queue.  It is
//               an error if the task is already added to this or any
//               other active queue.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
add(AsyncTask *task) {
  MutexHolder holder(_lock);

  if (task->_state == AsyncTask::S_servicing_removed) {
    if (task->_manager == this) {
      // Re-adding a self-removed task; this just means clearing the
      // removed flag.
      task->_state = AsyncTask::S_servicing;
      return;
    }
  }

  nassertv(task->_manager == NULL &&
           task->_state == AsyncTask::S_inactive);
  nassertv(!do_has_task(task));

  do_start_threads();

  task->_manager = this;
  add_task_by_name(task);

  double now = _clock->get_frame_time();
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
  _cvar.signal_all();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::has_task
//       Access: Published
//  Description: Returns true if the indicated task has been added to
//               this AsyncTaskManager, false otherwise.
////////////////////////////////////////////////////////////////////
bool AsyncTaskManager::
has_task(AsyncTask *task) const {
  MutexHolder holder(_lock);

  if (task->_manager != this) {
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
//     Function: AsyncTaskManager::find_task
//       Access: Published
//  Description: Returns the first task found with the indicated name,
//               or NULL if there is no task with the indicated name.
//
//               If there are multiple tasks with the same name,
//               returns one of them arbitrarily.
////////////////////////////////////////////////////////////////////
AsyncTask *AsyncTaskManager::
find_task(const string &name) const {
  AsyncTask sample_task(name);
  sample_task.local_object();

  TasksByName::const_iterator tbni = _tasks_by_name.lower_bound(&sample_task);
  if (tbni != _tasks_by_name.end() && (*tbni)->get_name() == name) {
    return (*tbni);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::find_tasks
//       Access: Published
//  Description: Returns the list of tasks found with the indicated
//               name.
////////////////////////////////////////////////////////////////////
AsyncTaskCollection AsyncTaskManager::
find_tasks(const string &name) const {
  AsyncTask sample_task(name);
  sample_task.local_object();

  TasksByName::const_iterator tbni = _tasks_by_name.lower_bound(&sample_task);
  AsyncTaskCollection result;
  while (tbni != _tasks_by_name.end() && (*tbni)->get_name() == name) {
    result.add_task(*tbni);
    ++tbni;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::find_tasks_matching
//       Access: Published
//  Description: Returns the list of tasks found whose name matches
//               the indicated glob pattern, e.g. "my_task_*".
////////////////////////////////////////////////////////////////////
AsyncTaskCollection AsyncTaskManager::
find_tasks_matching(const GlobPattern &pattern) const {
  string prefix = pattern.get_const_prefix();
  AsyncTask sample_task(prefix);
  sample_task.local_object();

  TasksByName::const_iterator tbni = _tasks_by_name.lower_bound(&sample_task);
  AsyncTaskCollection result;
  while (tbni != _tasks_by_name.end() && (*tbni)->get_name().substr(0, prefix.size()) == prefix) {
    AsyncTask *task = (*tbni);
    if (pattern.matches(task->get_name())) {
      result.add_task(task);
    }
    ++tbni;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::remove
//       Access: Published
//  Description: Removes the indicated task from the active queue.
//               Returns true if the task is successfully removed, or
//               false if it wasn't there.
////////////////////////////////////////////////////////////////////
bool AsyncTaskManager::
remove(AsyncTask *task) {
  // We pass this up to the multi-task remove() flavor.  Do we care
  // about the tiny cost of creating an AsyncTaskCollection here?
  // Probably not.
  AsyncTaskCollection tasks;
  tasks.add_task(task);
  return remove(tasks) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::remove
//       Access: Published
//  Description: Removes all of the tasks in the AsyncTaskCollection.
//               Returns the number of tasks removed.
////////////////////////////////////////////////////////////////////
int AsyncTaskManager::
remove(const AsyncTaskCollection &tasks) {
  MutexHolder holder(_lock);
  int num_removed = 0;

  int num_tasks = tasks.get_num_tasks();
  int i;
  for (i = 0; i < num_tasks; ++i) {
    AsyncTask *task = tasks.get_task(i);
    
    if (task->_manager != this) {
      // Not a member of this manager, or already removed.
      nassertr(!do_has_task(task), num_removed);
    } else {
      switch (task->_state) {
      case AsyncTask::S_servicing:
        // This task is being serviced.
        task->_state = AsyncTask::S_servicing_removed;
        break;

      case AsyncTask::S_servicing_removed:
        // Being serviced, though it will be removed later.
        break;
      
      case AsyncTask::S_sleeping:
        // Sleeping, easy.
        {
          int index = find_task_on_heap(_sleeping, task);
          nassertr(index != -1, num_removed);
          _sleeping.erase(_sleeping.begin() + index);
          make_heap(_sleeping.begin(), _sleeping.end(), AsyncTaskSortWakeTime());
          ++num_removed;
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
            nassertr(index != -1, num_removed);
            _next_active.erase(_next_active.begin() + index);
          }
          ++num_removed;
          cleanup_task(task, false);
        }
      }
    }
  }

  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::wait_for_tasks
//       Access: Published
//  Description: Blocks until the task list is empty.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
wait_for_tasks() {
  MutexHolder holder(_lock);

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
//     Function: AsyncTaskManager::get_tasks
//       Access: Published
//  Description: Returns the set of tasks that are active or sleeping
//               on the task manager, at the time of the call.
////////////////////////////////////////////////////////////////////
AsyncTaskCollection AsyncTaskManager::
get_tasks() {
  AsyncTaskCollection result = get_active_tasks();
  result.add_tasks_from(get_sleeping_tasks());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::get_active_tasks
//       Access: Published
//  Description: Returns the set of tasks that are active (and not
//               sleeping) on the task manager, at the time of the
//               call.
////////////////////////////////////////////////////////////////////
AsyncTaskCollection AsyncTaskManager::
get_active_tasks() {
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
//     Function: AsyncTaskManager::get_sleeping_tasks
//       Access: Published
//  Description: Returns the set of tasks that are sleeping (and not
//               active) on the task manager, at the time of the
//               call.
////////////////////////////////////////////////////////////////////
AsyncTaskCollection AsyncTaskManager::
get_sleeping_tasks() {
  AsyncTaskCollection result;

  TaskHeap::const_iterator ti;
  for (ti = _sleeping.begin(); ti != _sleeping.end(); ++ti) {
    AsyncTask *task = (*ti);
    result.add_task(task);
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::poll
//       Access: Published
//  Description: Runs through all the tasks in the task list, once, if
//               the task manager is running in single-threaded mode
//               (no threads available).  This method does nothing in
//               threaded mode, so it may safely be called in either
//               case.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
poll() {
  MutexHolder holder(_lock);
  do_start_threads();

  if (!_threads.empty()) {
    return;
  }
  
  do_poll();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
output(ostream &out) const {
  MutexHolder holder(_lock);

  out << get_type() << " " << get_name()
      << "; " << _num_tasks << " tasks";
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
write(ostream &out, int indent_level) const {
  MutexHolder holder(_lock);
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
    double now = _clock->get_frame_time();

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
//     Function: AsyncTaskManager::do_has_task
//       Access: Protected
//  Description: Returns true if the task is on one of the task lists,
//               false if it is not (false may mean that the task is
//               currently being serviced).  Assumes the lock is
//               currently held.
////////////////////////////////////////////////////////////////////
bool AsyncTaskManager::
do_has_task(AsyncTask *task) const {
  return (find_task_on_heap(_active, task) != -1 ||
          find_task_on_heap(_next_active, task) != -1 ||
          find_task_on_heap(_sleeping, task) != -1);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::find_task_on_heap
//       Access: Protected
//  Description: Returns the index number of the indicated task within
//               the specified task list, or -1 if the task is not
//               found in the list (this may mean that it is currently
//               being serviced).  Assumes that the lock is currently
//               held.
////////////////////////////////////////////////////////////////////
int AsyncTaskManager::
find_task_on_heap(const TaskHeap &heap, AsyncTask *task) const {
  for (int i = 0; i < (int)heap.size(); ++i) {
    if (heap[i] == task) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::remove_task_by_name
//       Access: Protected
//  Description: Removes the task from the _tasks_by_name index, if it
//               has a nonempty name.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
remove_task_by_name(AsyncTask *task) {
  if (!task->get_name().empty()) {
    // We have to scan linearly through all of the tasks with the same
    // name.
    TasksByName::const_iterator tbni = _tasks_by_name.lower_bound(task);
    while (tbni != _tasks_by_name.end()) {
      if ((*tbni) == task) {
        _tasks_by_name.erase(tbni);
        return;
      }
      if ((*tbni)->get_name() != task->get_name()) {
        // Too far.
        break;
      }
      
      ++tbni;
    }

    // For some reason, the task wasn't on the index.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::service_one_task
//       Access: Protected
//  Description: Pops a single task off the active queue, services it,
//               and restores it to the end of the queue.  This is
//               called internally only within one of the task
//               threads.  Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
service_one_task(AsyncTaskManager::AsyncTaskManagerThread *thread) {
  if (!_active.empty()) {
    PT(AsyncTask) task = _active.front();
    pop_heap(_active.begin(), _active.end(), AsyncTaskSortPriority());
    _active.pop_back();

    if (thread != (AsyncTaskManager::AsyncTaskManagerThread *)NULL) {
      thread->_servicing = task;
    }

    nassertv(task->get_sort() == _current_sort);
    nassertv(task->_state == AsyncTask::S_active);
    task->_state = AsyncTask::S_servicing;
    task->_servicing_thread = thread;

    // Now release the manager lock while we actually service the
    // task.
    _lock.release();
    AsyncTask::DoneStatus ds = task->do_task();

    // Now we have to re-acquire the manager lock, so we can put the
    // task back on the queue (and so we can return with the lock
    // still held).
    _lock.lock();

    if (thread != (AsyncTaskManager::AsyncTaskManagerThread *)NULL) {
      thread->_servicing = NULL;
    }
    task->_servicing_thread = NULL;

    if (task->_manager == this) {
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
            double now = _clock->get_frame_time();
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
//     Function: AsyncTaskManager::cleanup_task
//       Access: Protected
//  Description: Called internally when a task has completed (or been
//               interrupted) and is about to be removed from the
//               active queue.  Assumes the lock is held.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
cleanup_task(AsyncTask *task, bool clean_exit) {
  nassertv(task->_manager == this);

  task->_state = AsyncTask::S_inactive;
  task->_manager = NULL;
  --_num_tasks;

  remove_task_by_name(task);

  if (clean_exit && !task->_done_event.empty()) {
    PT_Event event = new Event(task->_done_event);
    event->add_parameter(EventParameter(task));
    throw_event(event);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::finish_sort_group
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
bool AsyncTaskManager::
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
    _clock->tick();
  }
  if (!_threads.empty()) {
    PStatClient::thread_tick(get_name());
  }
    
  _active.swap(_next_active);

  // Check for any sleeping tasks that need to be woken.
  double now = _clock->get_frame_time();
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
//     Function: AsyncTaskManager::do_stop_threads
//       Access: Protected
//  Description: The private implementation of stop_threads; assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
do_stop_threads() {
  if (_state == S_started || _state == S_aborting) {
    _state = S_shutdown;
    _cvar.signal_all();
    
    Threads wait_threads;
    wait_threads.swap(_threads);
    
    // We have to release the lock while we join, so the threads can
    // wake up and see that we're shutting down.
    _lock.release();
    Threads::iterator ti;
    for (ti = wait_threads.begin(); ti != wait_threads.end(); ++ti) {
      (*ti)->join();
    }
    _lock.lock();
    
    _state = S_initial;
    nassertv(_num_busy_threads == 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::do_start_threads
//       Access: Protected
//  Description: The private implementation of start_threads; assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
do_start_threads() {
  if (_state == S_aborting) {
    do_stop_threads();
  }

  if (_state == S_initial) {
    _state = S_started;
    _num_busy_threads = 0;
    if (Thread::is_threading_supported()) {
      _threads.reserve(_num_threads);
      for (int i = 0; i < _num_threads; ++i) {
        ostringstream strm;
        strm << get_name() << "_" << i;
        PT(AsyncTaskManagerThread) thread = new AsyncTaskManagerThread(strm.str(), this);
        if (thread->start(TP_low, true)) {
          _threads.push_back(thread);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::do_poll
//       Access: Protected
//  Description: The private implementation of poll(), this assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
do_poll() {
  while (!_active.empty() && _state != S_shutdown && _state != S_aborting) {
    _current_sort = _active.front()->get_sort();
    service_one_task(NULL);
  }

  if (_state != S_shutdown && _state != S_aborting) {
    finish_sort_group();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::AsyncTaskManagerThread::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
AsyncTaskManager::AsyncTaskManagerThread::
AsyncTaskManagerThread(const string &name, AsyncTaskManager *manager) :
  Thread(name, manager->get_name()),
  _manager(manager),
  _servicing(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::AsyncTaskManagerThread::thread_main
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::AsyncTaskManagerThread::
thread_main() {
  MutexHolder holder(_manager->_lock);
  while (_manager->_state != S_shutdown && _manager->_state != S_aborting) {
    if (!_manager->_active.empty() &&
        _manager->_active.front()->get_sort() == _manager->_current_sort) {
      PStatTimer timer(_task_pcollector);
      _manager->_num_busy_threads++;
      _manager->service_one_task(this);
      _manager->_num_busy_threads--;
      _manager->_cvar.signal_all();

    } else {
      // We've finished all the available tasks of the current sort
      // value.  We can't pick up a new task until all of the threads
      // finish the tasks with the same sort value.
      if (_manager->_num_busy_threads == 0) {
        // We're the last thread to finish.  Update _current_sort.
        if (!_manager->finish_sort_group()) {
          // Nothing to do.  Wait for more tasks to be added.
          if (_manager->_sleeping.empty()) {
            PStatTimer timer(_wait_pcollector);
            _manager->_cvar.wait();
          } else {
            double wake_time = _manager->_sleeping.front()->get_wake_time();
            double now = _manager->_clock->get_frame_time();
            double timeout = max(wake_time - now, 0.0);
            PStatTimer timer(_wait_pcollector);
            _manager->_cvar.wait(timeout);
          }            
        }

      } else {
        // Wait for the other threads to finish their current task
        // before we continue.
        PStatTimer timer(_wait_pcollector);
        _manager->_cvar.wait();
      }
    }
  }
}

