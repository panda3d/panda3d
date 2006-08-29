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
//     Function: AsyncTaskManager::get_num_threads
//       Access: Published
//  Description: Returns the number of threads that have been created
//               to service the tasks within this task manager.
////////////////////////////////////////////////////////////////////
INLINE int AsyncTaskManager::
get_num_threads() const {
#ifdef HAVE_THREADS
  return _num_threads;
#else
  // Without threading support, this is always 0 threads.
  return 0;
#endif
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
