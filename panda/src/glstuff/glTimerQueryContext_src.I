/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glTimerQueryContext_src.I
 * @author rdb
 * @date 2014-08-22
 */

/**
 *
 */
INLINE CLP(TimerQueryContext)::
CLP(TimerQueryContext)(CLP(GraphicsStateGuardian) *glgsg,
                       int pstats_index) :
  TimerQueryContext(pstats_index),
  _glgsg(glgsg),
  _index(0)
{
}
