/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelLoadRequest.cxx
 * @author drose
 * @date 2006-08-29
 */

#include "modelLoadRequest.h"
#include "loader.h"
#include "config_pgraph.h"

TypeHandle ModelLoadRequest::_type_handle;

/**
 * Create a new ModelLoadRequest, and add it to the loader via load_async(),
 * to begin an asynchronous load.
 */
ModelLoadRequest::
ModelLoadRequest(const std::string &name,
                 const Filename &filename, const LoaderOptions &options,
                 Loader *loader) :
  AsyncTask(name),
  _filename(filename),
  _options(options),
  _loader(loader)
{
}

/**
 * Performs the task: that is, loads the one model.
 */
AsyncTask::DoneStatus ModelLoadRequest::
do_task() {
  double delay = async_load_delay;
  if (delay != 0.0) {
    Thread::sleep(delay);
  }

  PT(PandaNode) model = _loader->load_sync(_filename, _options);
  set_result(model);

  // Don't continue the task; we're done.
  return DS_done;
}
