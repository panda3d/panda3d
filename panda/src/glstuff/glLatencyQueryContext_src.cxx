/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glLatencyQueryContext_src.cxx
 * @author rdb
 * @date 2014-09-24
 */

#ifndef OPENGLES  // Timer queries not supported by OpenGL ES.

TypeHandle CLP(LatencyQueryContext)::_type_handle;

/**
 *
 */
CLP(LatencyQueryContext)::
CLP(LatencyQueryContext)(CLP(GraphicsStateGuardian) *glgsg,
                         int pstats_index) :
  CLP(TimerQueryContext)(glgsg, pstats_index),
  _timestamp(0)
{
  glgsg->_glGetInteger64v(GL_TIMESTAMP, &_timestamp);
}

/**
 * Returns the timestamp that is the result of this timer query.  There's no
 * guarantee about which clock this uses, the only guarantee is that
 * subtracting a start time from an end time should yield a time in seconds.
 * If is_answer_ready() did not return true, this function may block before it
 * returns.
 *
 * It is only valid to call this from the draw thread.
 */
double CLP(LatencyQueryContext)::
get_timestamp() const {
  GLint64 time_ns;
  _glgsg->_glGetQueryObjecti64v(_index, GL_QUERY_RESULT, &time_ns);

  return (time_ns - _timestamp) * 0.000000001;
}

#endif  // OPENGLES
