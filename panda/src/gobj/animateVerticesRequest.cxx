// Filename: animateVerticesRequest.cxx
// Created by:  pratt (20Nov07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
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

#include "animateVerticesRequest.h"
#include "geomVertexData.h"

TypeHandle AnimateVerticesRequest::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimateVerticesRequest::do_task
//       Access: Protected, Virtual
//  Description: Performs the task: that is, calls animate vertices
//               on _geom_vertex_data.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus AnimateVerticesRequest::
do_task() {
  Thread *current_thread = Thread::get_current_thread();

  // There is no need to store or return a result.  The GeomVertexData caches
  // the result and it will be used later in the rendering process.
  _geom_vertex_data->animate_vertices(true, current_thread);
  _is_ready = true;

  // Don't continue the task; we're done.
  return AsyncTask::DS_done;
}
