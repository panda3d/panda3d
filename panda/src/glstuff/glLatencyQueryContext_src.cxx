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
  CLP(TimerQueryContext)(glgsg, pstats_index)
{
  glgsg->_glGetInteger64v(GL_TIMESTAMP, &_epoch);
}

#endif  // OPENGLES
