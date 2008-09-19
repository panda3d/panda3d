// Filename: modelLoadRequest.cxx
// Created by:  drose (29Aug06)
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

#include "modelLoadRequest.h"
#include "loader.h"
#include "config_pgraph.h"

TypeHandle ModelLoadRequest::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ModelLoadRequest::do_task
//       Access: Protected, Virtual
//  Description: Performs the task: that is, loads the one model.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus ModelLoadRequest::
do_task() {
  double delay = async_load_delay;
  if (delay != 0.0) {
    Thread::sleep(delay);
  }

  Loader *loader;
  DCAST_INTO_R(loader, _manager, DS_done);

  _model = loader->load_sync(_filename, _options);
  _is_ready = true;

  // Don't continue the task; we're done.
  return DS_done;
}
