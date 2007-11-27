// Filename: asyncTaskManager.cxx
// Created by:  drose (23Aug06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
  _num_threads(num_threads),
  _cvar(_lock),
  _num_tasks(0),
  _state(S_initial)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskManager::
~AsyncTaskManager() {
  stop_threads();
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
  if (_state == S_started) {
    // Clean up all of the threads.
    MutexHolder holder(_lock);
    if (_state == S_started) {
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
    }
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
  if (_state == S_initial) {
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

  nassertv(task->_manager == NULL &&
           task->_state == AsyncTask::S_inactive);
  nassertv(find_task(task) == -1);

  do_start_threads();

  task->_manager = this;
  task->_state = AsyncTask::S_active;

  _active.push_back(task);
  ++_num_tasks;
  _cvar.signal_all();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::add_and_do
//       Access: Published
//  Description: Adds the indicated task to the active queue, and
//               executes it immediately if this is a non-threaded
//               task manager.
//
//               The only difference between this method and add() is
//               in the case of a non-threaded task manager: in this
//               case, the method will execute the task inline, at
//               least one frame, before returning.  If the task
//               completes in one frame, this means it will completely
//               execute the task before returning in the non-threaded
//               case.  In the threaded case, this method behaves
//               exactly the same as add().
//
//               The return value is true if the task has been added
//               and is still pending, false if it has completed.
////////////////////////////////////////////////////////////////////
bool AsyncTaskManager::
add_and_do(AsyncTask *task) {
  MutexHolder holder(_lock);

  nassertr(task->_manager == NULL &&
           task->_state == AsyncTask::S_inactive, false);
  nassertr(find_task(task) == -1, false);

  do_start_threads();

  task->_manager = this;
  task->_state = AsyncTask::S_active;

  if (_threads.empty()) {
    // Got no threads.  Try to execute the task immediately.
    if (!task->do_task()) {
      // The task has finished in one frame.  Don't add it to the
      // queue.
      task_done(task);
      return false;
    }
  }

  _active.push_back(task);
  ++_num_tasks;
  _cvar.signal_all();

  return true;
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
  MutexHolder holder(_lock);

  // It's just possible this particular task is currently being
  // serviced.  Wait for it to finish.
  while (task->_manager == this && 
         task->_state == AsyncTask::S_servicing) {
    PStatTimer timer(_wait_pcollector);
    _cvar.wait();
  }

  if (task->_manager != this) {
    nassertr(find_task(task) == -1, false);
    return false;
  }

  nassertr(task->_state == AsyncTask::S_active, false);

  int index = find_task(task);
  nassertr(index != -1, false);
  _active.erase(_active.begin() + index);
  --_num_tasks;

  task->_state = AsyncTask::S_inactive;
  task->_manager = NULL;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::has_task
//       Access: Published
//  Description: Returns true if the indicated task is currently in
//               this manager's active queue, or false otherwise.
////////////////////////////////////////////////////////////////////
bool AsyncTaskManager::
has_task(AsyncTask *task) const {
  MutexHolder holder(_lock);

  if (task->_manager != this) {
    nassertr(find_task(task) == -1, false);
    return false;
  }

  // The task might not actually be in the active queue, since it
  // might be being serviced right now.  That's OK.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::wait_for_tasks
//       Access: Published
//  Description: Blocks until the task list is empty.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
wait_for_tasks() {
  MutexHolder holder(_lock);

  if (_threads.empty()) {
    // Non-threaded case.
    while (!_active.empty()) {
      if (_state == AsyncTaskManager::S_shutdown) {
        return;
      }
      do_poll();
    }

  } else {
    // Threaded case.
    while (_num_tasks > 0) {
      if (_state == AsyncTaskManager::S_shutdown) {
        return;
      }
      
      PStatTimer timer(_wait_pcollector);
      _cvar.wait();
    }
  }
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
      << "; " << get_num_tasks() << " tasks";
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

  Threads::const_iterator thi;
  for (thi = _threads.begin(); thi != _threads.end(); ++thi) {
    AsyncTask *task = (*thi)->_servicing;
    if (task != (AsyncTask *)NULL) {
      indent(out, indent_level + 1)
        << "*" << *task << "\n";
    }
  }

  Tasks::const_iterator ti;
  for (ti = _active.begin(); ti != _active.end(); ++ti) {
    AsyncTask *task = (*ti);
    indent(out, indent_level + 2)
      << *task << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::find_task
//       Access: Protected
//  Description: Returns the index number within the given request
//               queue of the indicated task, or -1 if the task is not
//               found in the active queue (this may mean that it is
//               currently being serviced).  Assumes that the lock is
//               currently held.
////////////////////////////////////////////////////////////////////
int AsyncTaskManager::
find_task(AsyncTask *task) const {
  for (int i = 0; i < (int)_active.size(); ++i) {
    if (_active[i] == task) {
      return i;
    }
  }

  return -1;
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
    _active.pop_front();
    thread->_servicing = task;

    nassertv(task->_state == AsyncTask::S_active);
    task->_state = AsyncTask::S_servicing;

    // Now release the manager lock while we actually service the
    // task.
    _lock.release();
    bool keep = task->do_task();

    // Now we have to re-acquire the manager lock, so we can put the
    // task back on the queue (and so we can return with the lock
    // still held).
    _lock.lock();

    thread->_servicing = NULL;
    if (task->_manager == this) {
      if (keep) {
        // The task is still alive; put it on the end of the active
        // queue.
        _active.push_back(task);
        task->_state = AsyncTask::S_active;
        
      } else {
        // The task has finished.
        task_done(task);
      }
      _cvar.signal_all();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::task_done
//       Access: Protected
//  Description: Called internally when a task has completed and is
//               about to be removed from the active queue.  Assumes
//               the lock is held.
////////////////////////////////////////////////////////////////////
void AsyncTaskManager::
task_done(AsyncTask *task) {
  task->_state = AsyncTask::S_inactive;
  task->_manager = NULL;
  --_num_tasks;

  if (!task->_done_event.empty()) {
    PT_Event event = new Event(task->_done_event);
    event->add_parameter(EventParameter(task));
    throw_event(event);
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
  if (_state == S_initial) {
    _state = S_started;
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
  Tasks new_active;
  int new_num_tasks = 0;
  Tasks::iterator ti;
  for (ti = _active.begin(); ti != _active.end(); ++ti) {
    AsyncTask *task = (*ti);
    nassertv(task->_state == AsyncTask::S_active);
    task->_state = AsyncTask::S_servicing;

    // Here we keep the manager lock held while we are servicing each
    // task.  This is the single-threaded implementation, after all,
    // so what difference should it make?
    if (task->do_task()) {
      new_active.push_back(task);
      ++new_num_tasks;
      task->_state = AsyncTask::S_active;

    } else {
      // The task has finished.
      task_done(task);
    }
  }

  _active.swap(new_active);
  _num_tasks = new_num_tasks;
  _cvar.signal_all();
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
  while (_manager->_state != AsyncTaskManager::S_shutdown) {
    PStatClient::thread_tick(_manager->get_name());

    {
      PStatTimer timer(_task_pcollector);
      _manager->service_one_task(this);
    }

    if (_manager->_active.empty() &&
        _manager->_state != AsyncTaskManager::S_shutdown) {
      PStatClient::thread_tick(_manager->get_name());
      PStatTimer timer(_wait_pcollector);
      _manager->_cvar.wait();
    }
  }
}

