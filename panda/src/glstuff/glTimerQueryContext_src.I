// Filename: glTimerQueryContext_src.I
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


////////////////////////////////////////////////////////////////////
//     Function: CLP(TimerQueryContext)::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLP(TimerQueryContext)::
CLP(TimerQueryContext)(CLP(GraphicsStateGuardian) *glgsg,
                       int pstats_index) :
  TimerQueryContext(pstats_index),
  _glgsg(glgsg),
  _index(0)
{
}
