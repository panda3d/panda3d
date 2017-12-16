/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animateVerticesRequest.cxx
 * @author pratt
 * @date 2007-11-20
 */

#include "animateVerticesRequest.h"
#include "geomVertexData.h"

TypeHandle AnimateVerticesRequest::_type_handle;

/**
 * Performs the task: that is, calls animate vertices on _geom_vertex_data.
 */
AsyncTask::DoneStatus AnimateVerticesRequest::
do_task() {
  Thread *current_thread = Thread::get_current_thread();

  // There is no need to store or return a result.  The GeomVertexData caches
  // the result and it will be used later in the rendering process.
  _geom_vertex_data->animate_vertices(true, current_thread);

  // Don't continue the task; we're done.
  return AsyncTask::DS_done;
}
