// Filename: asyncTaskManager.I
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
//     Function: AsyncTaskManager::get_num_threads
//       Access: Published
//  Description: Returns the number of threads that have been created
//               to service the tasks within this task manager.  This
//               will return 0 before the threads have been started;
//               it will also return 0 if thread support is not
//               available.
////////////////////////////////////////////////////////////////////
INLINE int AsyncTaskManager::
get_num_threads() const {
  MutexHolder holder(_lock);
  return _threads.size();
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskManager::get_num_tasks
//       Access: Published
//  Description: Returns the number of tasks that are currently active
//               within the task manager.
////////////////////////////////////////////////////////////////////
INLINE int AsyncTaskManager::
get_num_tasks() const {
  return _num_tasks;
}
