/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelSaveRequest.cxx
 * @author drose
 * @date 2012-12-19
 */

#include "modelSaveRequest.h"
#include "loader.h"
#include "config_pgraph.h"

TypeHandle ModelSaveRequest::_type_handle;

/**
 * Create a new ModelSaveRequest, and add it to the loader via save_async(),
 * to begin an asynchronous save.
 */
ModelSaveRequest::
ModelSaveRequest(const std::string &name,
                 const Filename &filename, const LoaderOptions &options,
                 PandaNode *node, Loader *loader) :
  AsyncTask(name),
  _filename(filename),
  _options(options),
  _node(node),
  _loader(loader),
  _success(false)
{
}

/**
 * Performs the task: that is, saves the one model.
 */
AsyncTask::DoneStatus ModelSaveRequest::
do_task() {
  double delay = async_load_delay;
  if (delay != 0.0) {
    Thread::sleep(delay);
  }

  _success = _loader->save_sync(_filename, _options, _node);

  // Don't continue the task; we're done.
  return DS_done;
}
