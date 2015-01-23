// Filename: glTimerQueryContext_src.cxx
// Created by:  rdb (22Aug14)
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

#include "pnotify.h"
#include "dcast.h"
#include "lightMutexHolder.h"
#include "pStatTimer.h"

#ifndef OPENGLES  // Timer queries not supported by OpenGL ES.

TypeHandle CLP(TimerQueryContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GLTimerQueryContext::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CLP(TimerQueryContext)::
~CLP(TimerQueryContext)() {
  if (_index != 0) {
    // Tell the GSG to recycle this index when it gets around to it.
    // If it has already shut down, though, too bad.  This means we
    // never get to free this index, but presumably the app is
    // already shutting down anyway.
    if (!_glgsg.was_deleted()) {
      LightMutexHolder holder(_glgsg->_lock);
      _glgsg->_deleted_queries.push_back(_index);
      _index = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLTimerQueryContext::is_answer_ready
//       Access: Public, Virtual
//  Description: Returns true if the query's answer is ready, false
//               otherwise.  If this returns false, the application
//               must continue to poll until it returns true.
//
//               It is only valid to call this from the draw thread.
////////////////////////////////////////////////////////////////////
bool CLP(TimerQueryContext)::
is_answer_ready() const {
  GLuint result;
  _glgsg->_glGetQueryObjectuiv(_index, GL_QUERY_RESULT_AVAILABLE, &result);

  return (result != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GLTimerQueryContext::waiting_for_answer
//       Access: Public, Virtual
//  Description: Requests the graphics engine to expedite the pending
//               answer--the application is now waiting until the
//               answer is ready.
//
//               It is only valid to call this from the draw thread.
////////////////////////////////////////////////////////////////////
void CLP(TimerQueryContext)::
waiting_for_answer() {
  PStatTimer timer(GraphicsStateGuardian::_wait_timer_pcollector);
  glFlush();
}

////////////////////////////////////////////////////////////////////
//     Function: TimerQueryContext::get_timestamp
//       Access: Public, Virtual
//  Description: Returns the timestamp that is the result of this
//               timer query.  There's no guarantee about which
//               clock this uses, the only guarantee is that
//               subtracting a start time from an end time should
//               yield a time in seconds.
//               If is_answer_ready() did not return true, this
//               function may block before it returns.
//
//               It is only valid to call this from the draw thread.
////////////////////////////////////////////////////////////////////
double CLP(TimerQueryContext)::
get_timestamp() const {
  GLuint64 time_ns;

  /*GLuint available;
  _glgsg->_glGetQueryObjectuiv(_index[1], GL_QUERY_RESULT_AVAILABLE, &available);
  if (available) {
    // The answer is ready now.
    do_get_timestamps(begin_ns, end_ns);
  } else {
    // The answer is not ready; this call will block.
    PStatTimer timer(GraphicsStateGuardian::_wait_timer_pcollector);
    do_get_timestamps(begin_ns, end_ns);
  }*/

  _glgsg->_glGetQueryObjectui64v(_index, GL_QUERY_RESULT, &time_ns);

  return time_ns * 0.000000001;
}

#endif  // OPENGLES
