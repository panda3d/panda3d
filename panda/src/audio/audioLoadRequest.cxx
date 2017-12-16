/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioLoadRequest.cxx
 * @author drose
 * @date 2006-08-29
 */

#include "audioLoadRequest.h"
#include "audioManager.h"

TypeHandle AudioLoadRequest::_type_handle;

/**
 * Performs the task: that is, loads the one sound file.
 */
AsyncTask::DoneStatus AudioLoadRequest::
do_task() {
  set_result(_audio_manager->get_sound(_filename, _positional));

  // Don't continue the task; we're done.
  return DS_done;
}
