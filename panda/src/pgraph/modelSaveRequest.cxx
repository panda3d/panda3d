// Filename: modelSaveRequest.cxx
// Created by:  drose (19Dec12)
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

#include "modelSaveRequest.h"
#include "loader.h"
#include "config_pgraph.h"

TypeHandle ModelSaveRequest::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ModelSaveRequest::Constructor
//       Access: Published
//  Description: Create a new ModelSaveRequest, and add it to the loader
//               via save_async(), to begin an asynchronous save.
////////////////////////////////////////////////////////////////////
ModelSaveRequest::
ModelSaveRequest(const string &name, 
                 const Filename &filename, const LoaderOptions &options,
                 PandaNode *node, Loader *loader) :
  AsyncTask(name),
  _filename(filename),
  _options(options),
  _node(node),
  _loader(loader),
  _is_ready(false),
  _success(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ModelSaveRequest::do_task
//       Access: Protected, Virtual
//  Description: Performs the task: that is, saves the one model.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus ModelSaveRequest::
do_task() {
  double delay = async_load_delay;
  if (delay != 0.0) {
    Thread::sleep(delay);
  }

  _success = _loader->save_sync(_filename, _options, _node);
  _is_ready = true;

  // Don't continue the task; we're done.
  return DS_done;
}
