/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskPause.cxx
 * @author drose
 * @date 2008-10-04
 */

#include "asyncTaskPause.h"

TypeHandle AsyncTaskPause::_type_handle;

/**
 *
 */
AsyncTaskPause::
AsyncTaskPause(double delay) :
  AsyncTask("pause")
{
  set_delay(delay);
}

/**
 * Override this function to do something useful for the task.
 *
 * This function is called with the lock *not* held.
 */
AsyncTask::DoneStatus AsyncTaskPause::
do_task() {
  return DS_pause;
}
