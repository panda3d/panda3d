// Filename: asyncTaskManager.I
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


////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::is_started
//       Access: Published
//  Description: Returns true if the thread(s) have been started and
//               are ready to service requests, false otherwise.  If
//               this is false, the next call to add() or add_and_do()
//               will automatically start the threads.
////////////////////////////////////////////////////////////////////
INLINE bool AsyncTaskManager::
is_started() const {
  return (_state == S_started);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::set_clock
//       Access: Published
//  Description: Replaces the clock pointer used within the
//               AsyncTaskManager.  This is used to control when tasks
//               with a set_delay() specified will be scheduled.  It
//               can also be ticked automatically each epoch, if
//               set_tick_clock() is true.
//
//               The default is the global clock pointer.
////////////////////////////////////////////////////////////////////
INLINE void AsyncTaskManager::
set_clock(ClockObject *clock) {
  _clock = clock;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::get_clock
//       Access: Published
//  Description: Returns the clock pointer used within the
//               AsyncTaskManager.  See set_clock().
////////////////////////////////////////////////////////////////////
INLINE ClockObject *AsyncTaskManager::
get_clock() {
  return _clock;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::set_tick_clock
//       Access: Published
//  Description: Sets the tick_clock flag.  When this is true,
//               get_clock()->tick() will be called automatically at
//               each task epoch.  This is false by default.
////////////////////////////////////////////////////////////////////
INLINE void AsyncTaskManager::
set_tick_clock(bool clock) {
  _tick_clock = clock;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::get_tick_clock
//       Access: Published
//  Description: Returns the tick_clock flag..  See set_tick_clock().
////////////////////////////////////////////////////////////////////
INLINE bool AsyncTaskManager::
get_tick_clock() const {
  return _tick_clock;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::get_num_threads
//       Access: Published
//  Description: Returns the number of threads that will be servicing
//               tasks for this manager.  Also see
//               get_num_running_threads().
////////////////////////////////////////////////////////////////////
INLINE int AsyncTaskManager::
get_num_threads() const {
  MutexHolder holder(_lock);
  return _num_threads;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::get_num_running_threads
//       Access: Published
//  Description: Returns the number of threads that have been created
//               and are actively running.  This will return 0 before
//               the threads have been started; it will also return 0
//               if thread support is not available.
////////////////////////////////////////////////////////////////////
INLINE int AsyncTaskManager::
get_num_running_threads() const {
  MutexHolder holder(_lock);
  return _threads.size();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::get_num_tasks
//       Access: Published
//  Description: Returns the number of tasks that are currently active
//               or sleeping within the task manager.
////////////////////////////////////////////////////////////////////
INLINE int AsyncTaskManager::
get_num_tasks() const {
  MutexHolder holder(_lock);
  return _num_tasks;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::add_task_by_name
//       Access: Protected
//  Description: Adds the task to the _tasks_by_name index, if it has
//               a nonempty name.
////////////////////////////////////////////////////////////////////
INLINE void AsyncTaskManager::
add_task_by_name(AsyncTask *task) {
  if (!task->get_name().empty()) {
    _tasks_by_name.insert(task);
  }
}
