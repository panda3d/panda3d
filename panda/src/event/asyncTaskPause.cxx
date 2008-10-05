// Filename: asyncTaskPause.cxx
// Created by:  drose (04Oct08)
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

#include "asyncTaskPause.h"

TypeHandle AsyncTaskPause::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskPause::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
AsyncTaskPause::
AsyncTaskPause(double delay) :
  AsyncTask("pause")
{
  set_delay(delay);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskPause::do_task
//       Access: Protected, Virtual
//  Description: Override this function to do something useful for the
//               task.
//
//               This function is called with the lock *not* held.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus AsyncTaskPause::
do_task() {
  return DS_pause;
}
